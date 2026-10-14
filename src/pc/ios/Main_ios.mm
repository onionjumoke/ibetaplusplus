// ─────────────────────────────────────────────────────────────────────────────
// Main_ios.mm — iOS entry point and the game-thread bootstrap.
//
// main() hands control to UIApplicationMain (never returns until the app
// dies); the app delegate spawns a detached thread that runs the same
// bootstrap as pc/Main.cpp: GL context → username → Minecraft::start.
// ─────────────────────────────────────────────────────────────────────────────

#import <UIKit/UIKit.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <unistd.h>

#include <mach/mach.h>
#include <pthread.h>
#include <execinfo.h>
#include <signal.h>

#include <atomic>
#include <algorithm>
#include <map>
#include <thread>
#include <vector>

#include "IosBridge.h"
#include "client/Minecraft.h"
#include "net/minecraft/src/GameResources.h"
#include "java/String.h"

#include "lwjgl/GLContext.h"

// ── Crash black box: dump raw backtrace to stderr (bp.log) on fatal signals ─
static void bpCrashHandler(int sig, siginfo_t *, void *ctx)
{
	// Async-signal-safety: write(2) + backtrace(3) only. backtrace_symbols_fd
	// mallocs (technically unsafe) but usually succeeds and gives readable
	// frames; we already wrote the raw addresses first either way.
	static const char msg[] = "\nBeta++: FATAL signal ";
	write(STDERR_FILENO, msg, sizeof(msg) - 1);
	char n[2] = {static_cast<char>('0' + (sig % 10)), '\n'};
	if (sig >= 10)
		n[0] = '0' + static_cast<char>(sig / 10);
	write(STDERR_FILENO, n, 2);

	// Registers first: sp/pc/lr + r0-r3 tell us null-deref vs wild-index vs
	// bad-stack without any symbolication. iOS 5 armv7 layout:
	// ucontext_t.uc_mcontext is _STRUCT_MCONTEXT BY VALUE; thread state lives
	// in .__ss.
	ucontext_t *uc = static_cast<ucontext_t *>(ctx);
	if (uc != nullptr)
	{
		const _STRUCT_MCONTEXT *mcp = uc->uc_mcontext;
		const _STRUCT_MCONTEXT &mc = *mcp;
		static const char rhdr[] = "Beta++: regs: ";
		write(STDERR_FILENO, rhdr, sizeof(rhdr) - 1);
		for (int ri = 0; ri < 13; ++ri)
		{
			char l[24];
			int len = snprintf(l, sizeof(l), "r%d=%08x ", ri,
			                   (unsigned)mc.__ss.__r[ri]);
			write(STDERR_FILENO, l, len);
		}
		char l2[96];
		int len2 = snprintf(l2, sizeof(l2),
		                    "sp=%08x lr=%08x pc=%08x\n",
		                    (unsigned)mc.__ss.__sp,
		                    (unsigned)mc.__ss.__lr,
		                    (unsigned)mc.__ss.__pc);
		write(STDERR_FILENO, l2, len2);
	}

	void *bt[48];
	int frames = backtrace(bt, 48);
	static const char hdr[] = "Beta++: crash backtrace (raw addrs):\n";
	write(STDERR_FILENO, hdr, sizeof(hdr) - 1);
	for (int i = 0; i < frames; ++i)
	{
		char line[32];
		const char pfx[] = "  #";
		write(STDERR_FILENO, pfx, 3);
		int v = i, len = 0;
		char tmp[8];
		do { tmp[len++] = static_cast<char>('0' + v % 10); v /= 10; } while (v);
		while (len) write(STDERR_FILENO, &tmp[--len], 1);
		write(STDERR_FILENO, " 0x", 3);
		uintptr_t a = reinterpret_cast<uintptr_t>(bt[i]);
		for (int sh = 28; sh >= 0; sh -= 4)
		{
			char h = "0123456789abcdef"[(a >> sh) & 0xF];
			write(STDERR_FILENO, &h, 1);
		}
		(void)line;
		write(STDERR_FILENO, "\n", 1);
	}
	backtrace_symbols_fd(bt, frames, STDERR_FILENO);
	signal(sig, SIG_DFL);
	raise(sig);
}

static void bpInstallCrashHandler()
{
	struct sigaction sa{};
	sa.sa_sigaction = bpCrashHandler;
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &sa, nullptr);
	sigaction(SIGBUS, &sa, nullptr);
	sigaction(SIGABRT, &sa, nullptr);
	sigaction(SIGILL, &sa, nullptr);
}

// ── System profiler: per-thread CPU deltas + RSS, printed every 5 s ─────────
// Gated on the bp.debug marker (same as stderr mirroring). Mach gives us
// per-thread user+sys time; deltas over the sample window = CPU%.

static std::atomic<bool> g_profRun(false);
static mach_port_t g_gameThread = MACH_PORT_NULL;
static mach_port_t g_mainThread = MACH_PORT_NULL;

// Defined in GLES11Shim.cpp at global scope.
extern std::atomic<uint32_t> g_playDraws;
extern std::atomic<uint32_t> g_playLists;

namespace
{

double threadCpuMs(thread_act_t t)
{
	struct thread_basic_info info;
	mach_msg_type_number_t count = THREAD_BASIC_INFO_COUNT;
	if (thread_info(t, THREAD_BASIC_INFO, (thread_info_t)&info, &count) != KERN_SUCCESS)
		return -1.0;
	return (info.user_time.seconds + info.system_time.seconds) * 1000.0 +
	       (info.user_time.microseconds + info.system_time.microseconds) / 1000.0;
}

void sysProfilerLoop()
{
	std::map<uint32_t, double> lastCpu;
	const useconds_t intervalUs = 5000000; // 5 s

	while (g_profRun.load())
	{
		usleep(intervalUs);

		thread_act_array_t list;
		mach_msg_type_number_t cnt = 0;
		if (task_threads(mach_task_self(), &list, &cnt) != KERN_SUCCESS)
			continue;

		int ncpu = (int)sysconf(_SC_NPROCESSORS_ONLN);
		if (ncpu <= 0) ncpu = 1;

		double totalDelta = 0.0;
		std::vector<std::pair<double, std::string>> rows;

		for (mach_msg_type_number_t i = 0; i < cnt; ++i)
		{
			double now = threadCpuMs(list[i]);
			if (now < 0.0)
				continue;
			uint32_t key = (uint32_t)list[i];
			double delta = now - lastCpu[key];
			lastCpu[key] = now;
			if (delta < 0.0)
				delta = 0.0;
			totalDelta += delta;

			const char *label = "t";
			char buf[24];
			if (list[i] == g_gameThread)
				label = "game";
			else if (list[i] == g_mainThread)
				label = "main";
			else
			{
				snprintf(buf, sizeof(buf), "t%u", key & 0xfff);
				label = buf;
			}
			rows.push_back({delta, label});
		}

		for (mach_msg_type_number_t i = 0; i < cnt; ++i)
			mach_port_deallocate(mach_task_self(), list[i]);
		vm_deallocate(mach_task_self(), (vm_address_t)list, cnt * sizeof(thread_act_t));

		std::sort(rows.begin(), rows.end(),
		          [](const std::pair<double, std::string> &a,
		             const std::pair<double, std::string> &b) {
			          return a.first > b.first;
		          });

		struct task_basic_info tbi;
		mach_msg_type_number_t icount = TASK_BASIC_INFO_COUNT;
		double rssMB = 0.0;
		if (task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&tbi, &icount) == KERN_SUCCESS)
			rssMB = tbi.resident_size / 1048576.0;

		uint32_t pd = g_playDraws.exchange(0);
		uint32_t pl = g_playLists.exchange(0);
		fprintf(stderr, "Beta++: sys ram=%.0fMB thr=%zu cpu:", rssMB, (size_t)cnt);
		double denom = ((double)intervalUs / 1000.0) * ncpu;
		size_t shown = 0;
		for (const auto &r : rows)
		{
			if (shown >= 4)
				break;
			fprintf(stderr, " %s %.0f%%", r.second.c_str(),
			        totalDelta > 0 ? r.first / denom * 100.0 : 0.0);
			shown++;
		}
		fprintf(stderr, " | draws=%u lists=%u\n", pd, pl);
	}
}

std::string trimUsername(std::string value)
{
	if (value.size() >= 3 &&
	    static_cast<unsigned char>(value[0]) == 0xef &&
	    static_cast<unsigned char>(value[1]) == 0xbb &&
	    static_cast<unsigned char>(value[2]) == 0xbf)
	{
		value.erase(0, 3);
	}

	const auto first = value.find_first_not_of(" \t\r\n");
	if (first == std::string::npos)
		return "";
	const auto last = value.find_last_not_of(" \t\r\n");
	return value.substr(first, last - first + 1);
}

std::string loadUsername()
{
	const std::filesystem::path path =
	    std::filesystem::path(GameResources::getExeDir()) / "username.txt";
	std::ifstream input(path);
	std::string username;
	if (input && std::getline(input, username))
		username = trimUsername(username);

	if (!username.empty())
		return username;

	username = "Player";
	std::ofstream output(path, std::ios::trunc);
	if (output)
		output << username << '\n';
	return username;
}

} // namespace

namespace iosapp
{

int runGameOnThisThread()
{
	g_gameThread = mach_thread_self();
	g_profRun.store(true);
	std::thread([] { sysProfilerLoop(); }).detach();

	// Wait for the window/view pair created by the app delegate.
	if (!waitForView(30.0))
	{
		fprintf(stderr, "Beta++: view never became ready\n");
		return 1;
	}
	fprintf(stderr, "Beta++: game thread starting, view ready\n");

	try
	{
		lwjgl::GLContext::instantiate();
	}
	catch (const std::exception &e)
	{
		fprintf(stderr, "Beta++: failed to create EAGL context: %s\n", e.what());
		return 1;
	}
	fprintf(stderr, "Beta++: EAGL context ready\n");

	jstring username = String::fromUTF8(loadUsername());
	jstring auth = u"-";

	Minecraft::start(&username, &auth);
	return 0;
}

} // namespace iosapp

int main(int argc, char *argv[])
{
	bpInstallCrashHandler();
	// Marker-file debug switch: if <TMPDIR>/bp.debug exists, mirror stderr
	// (assert/terminate messages, diagnostics) into <TMPDIR>/bp.log so
	// SpringBoard launches are debuggable over USB. The iOS sandbox only
	// permits writes inside Documents/Library/tmp of the container — never
	// the container root — hence TMPDIR (set by SpringBoard) rather than HOME.
	const char *tmpDir = ::getenv("TMPDIR");
	if (tmpDir == nullptr || tmpDir[0] == '\0')
		tmpDir = ::getenv("HOME");
	if (tmpDir != nullptr && tmpDir[0] == '\0')
		tmpDir = nullptr;
	if (tmpDir != nullptr)
	{
		std::string marker = std::string(tmpDir) + "/bp.debug";
		struct stat st;
		if (::stat(marker.c_str(), &st) == 0)
		{
			std::string logPath = std::string(tmpDir) + "/bp.log";
			if (::freopen(logPath.c_str(), "w", stderr) != nullptr)
				::setvbuf(stderr, nullptr, _IONBF, 0);
		}
	}

	g_mainThread = mach_thread_self();
	@autoreleasepool
	{
		UIApplicationMain(argc, argv, nil, @"IOSAppDelegate");
	}
	return 0; // unreachable: UIApplicationMain only returns at termination
}
