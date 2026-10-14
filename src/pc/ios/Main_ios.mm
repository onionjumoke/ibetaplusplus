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
#include <fcntl.h>
#include <climits>

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
// bp.log is truncated every launch (freopen "w"), so a crash backtrace is lost
// as soon as the user relaunches to retry. bp.crash.log is opened O_APPEND and
// the handler redirects stderr to it, so the *last* crash always survives.
static int g_crashFd = -1;

static void bpOpenCrashLog()
{
	const char *dir = ::getenv("TMPDIR");
	if (dir == nullptr || dir[0] == '\0')
		dir = ::getenv("HOME");
	if (dir == nullptr || dir[0] == '\0')
		return;
	char path[PATH_MAX];
	snprintf(path, sizeof(path), "%s/bp.crash.log", dir);
	g_crashFd = ::open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
}
static void bpCrashHandler(int sig, siginfo_t *, void *ctx)
{
	// Send the whole dump to the append-only crash log as well as stderr.
	if (g_crashFd >= 0)
		::dup2(g_crashFd, STDERR_FILENO);

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

namespace
{

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
	// Wait for the window/view pair created by the app delegate.
	if (!waitForView(30.0))
	{
		(void)0;
		return 1;
	}
	(void)0;

	try
	{
		lwjgl::GLContext::instantiate();
	}
	catch (const std::exception &e)
	{
		(void)0;
		return 1;
	}
	(void)0;

	jstring username = String::fromUTF8(loadUsername());
	jstring auth = u"-";

	Minecraft::start(&username, &auth);
	return 0;
}

} // namespace iosapp

int main(int argc, char *argv[])
{
	bpOpenCrashLog();
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

	@autoreleasepool
	{
		UIApplicationMain(argc, argv, nil, @"IOSAppDelegate");
	}
	return 0; // unreachable: UIApplicationMain only returns at termination
}
