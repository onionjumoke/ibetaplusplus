// ─────────────────────────────────────────────────────────────────────────────
// fs_backport.cpp — runtime implementation of std::filesystem's library side
// for iOS 5, where libSystem predates the openat()-based upstream sources.
//
// The vendored libc++ 19 headers declare these as
// `_LIBCPP_EXPORTED_FROM_ABI` inside std::__1::__fs::filesystem; we define
// them here against plain POSIX (stat/mkdir/opendir/rename), all of which the
// iPhoneOS5.0.sdk provides. The file clock matches filesystem_clock.cpp:
// nanoseconds since the Unix epoch.
// ─────────────────────────────────────────────────────────────────────────────

#include <__config>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <system_error>

#include <dirent.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

_LIBCPP_BEGIN_NAMESPACE_FILESYSTEM

namespace
{

using chrono::duration_cast;
using chrono::nanoseconds;
using chrono::seconds;

error_code ecFromErrno()
{
	return error_code(errno, generic_category());
}

file_status statusFromMode(mode_t m)
{
	if (S_ISDIR(m))
		return file_status(file_type::directory, static_cast<perms>(m & 07777));
	if (S_ISREG(m))
		return file_status(file_type::regular, static_cast<perms>(m & 07777));
	if (S_ISLNK(m))
		return file_status(file_type::symlink, static_cast<perms>(m & 07777));
	if (S_ISBLK(m))
		return file_status(file_type::block, static_cast<perms>(m & 07777));
	if (S_ISCHR(m))
		return file_status(file_type::character, static_cast<perms>(m & 07777));
	if (S_ISFIFO(m))
		return file_status(file_type::fifo, static_cast<perms>(m & 07777));
	if (S_ISSOCK(m))
		return file_status(file_type::socket, static_cast<perms>(m & 07777));
	return file_status(file_type::unknown, perms::none);
}

bool statPath(const path &p, struct ::stat *st, bool followLinks)
{
	return ::stat(p.c_str(), st) == 0;
}

} // namespace

// ── Status ───────────────────────────────────────────────────────────────────

file_status __status(const path &p, error_code *ec)
{
	struct ::stat st;
	if (::stat(p.c_str(), &st) != 0)
	{
		if (ec)
			*ec = ecFromErrno();
		return file_status(file_type::not_found, perms::none);
	}
	if (ec)
		ec->clear();
	return statusFromMode(st.st_mode);
}

file_status __symlink_status(const path &p, error_code *ec)
{
	struct ::stat st;
	if (::lstat(p.c_str(), &st) != 0)
	{
		if (ec)
			*ec = ecFromErrno();
		return file_status(file_type::not_found, perms::none);
	}
	if (ec)
		ec->clear();
	return statusFromMode(st.st_mode);
}

// ── Creation / removal ───────────────────────────────────────────────────────

bool __create_directory(const path &p, error_code *ec)
{
	if (::mkdir(p.c_str(), 0755) == 0)
	{
		if (ec)
			ec->clear();
		return true;
	}
	if (errno == EEXIST && __status(p).type() == file_type::directory)
	{
		if (ec)
			ec->clear();
		return false;
	}
	if (ec)
		*ec = ecFromErrno();
	else
		throw filesystem_error("filesystem error: mkdir failed", p, ecFromErrno());
	return false;
}

bool __create_directories(const path &p, error_code *ec)
{
	error_code local;
	path partial;
	for (const path &part : p) // iterates components including root
	{
		partial /= part;
		if (partial.empty() || partial == p.root_path())
			continue;
		error_code st_ec;
		file_status st = __status(partial, &st_ec);
		if (st.type() == file_type::directory)
			continue;
		if (__create_directory(partial, &local))
			continue;
		st = __status(partial);
		if (st.type() != file_type::directory)
		{
			if (ec)
				*ec = local;
			return false;
		}
		local.clear();
	}
	if (ec)
		ec->clear();
	return true;
}

bool __remove(const path &p, error_code *ec)
{
	file_status st = __symlink_status(p);
	if (st.type() == file_type::not_found)
	{
		if (ec)
			ec->clear();
		return false;
	}
	int rc;
	if (st.type() == file_type::directory)
		rc = ::rmdir(p.c_str());
	else
		rc = ::unlink(p.c_str());
	if (rc != 0)
	{
		if (ec)
			*ec = ecFromErrno();
		return false;
	}
	if (ec)
		ec->clear();
	return true;
}

static uintmax_t removeAllRecursive(const path &p, error_code *ec)
{
	uintmax_t count = 0;
	file_status st = __symlink_status(p);
	if (st.type() == file_type::not_found)
	{
		if (ec)
			*ec = ecFromErrno();
		return 0;
	}
	if (st.type() == file_type::directory)
	{
		DIR *dir = ::opendir(p.c_str());
		if (dir != nullptr)
		{
			while (dirent *e = ::readdir(dir))
			{
				const char *n = e->d_name;
				if (std::strcmp(n, ".") == 0 || std::strcmp(n, "..") == 0)
					continue;
				count += removeAllRecursive(p / n, ec);
			}
			::closedir(dir);
		}
	}
	if (__remove(p, ec))
		++count;
	return count;
}

uintmax_t __remove_all(const path &p, error_code *ec)
{
	return removeAllRecursive(p, ec);
}

void __rename(const path &from, const path &to, error_code *ec)
{
	if (::rename(from.c_str(), to.c_str()) != 0)
	{
		if (ec)
			*ec = ecFromErrno();
		else
			__throw_filesystem_error("filesystem error: rename failed", from, to, ecFromErrno());
	}
	else if (ec)
		ec->clear();
}

void __resize_file(const path &p, uintmax_t size, error_code *ec)
{
	if (::truncate(p.c_str(), static_cast<off_t>(size)) != 0)
	{
		if (ec)
			*ec = ecFromErrno();
	}
	else if (ec)
		ec->clear();
}

// ── Size / links / times ─────────────────────────────────────────────────────

uintmax_t __file_size(const path &p, error_code *ec)
{
	struct ::stat st;
	if (!statPath(p, &st, true))
	{
		if (ec)
			*ec = ecFromErrno();
		return static_cast<uintmax_t>(-1);
	}
	if (!S_ISREG(st.st_mode))
	{
		if (ec)
			*ec = error_code(EINVAL, generic_category());
		return static_cast<uintmax_t>(-1);
	}
	if (ec)
		ec->clear();
	return static_cast<uintmax_t>(st.st_size);
}

uintmax_t __hard_link_count(const path &p, error_code *ec)
{
	struct ::stat st;
	if (!statPath(p, &st, true))
	{
		if (ec)
			*ec = ecFromErrno();
		return static_cast<uintmax_t>(-1);
	}
	if (ec)
		ec->clear();
	return static_cast<uintmax_t>(st.st_nlink);
}

static _FilesystemClock::time_point timespecToFileTime(const struct ::timespec &ts)
{
	// File clock = nanoseconds since Unix epoch (matches filesystem_clock.cpp).
	auto ns = duration_cast<nanoseconds>(seconds(static_cast<long long>(ts.tv_sec))) +
	          nanoseconds(ts.tv_nsec);
	return _FilesystemClock::time_point(
	    duration_cast<_FilesystemClock::duration>(ns));
}

static void fileTimeToTimespec(_FilesystemClock::time_point tp, struct ::timespec *ts)
{
	auto total = duration_cast<nanoseconds>(tp.time_since_epoch()).count();
	ts->tv_sec = static_cast<time_t>(total / 1000000000LL);
	ts->tv_nsec = static_cast<long>(total % 1000000000LL);
}

file_time_type __last_write_time(const path &p, error_code *ec)
{
	struct ::stat st;
	if (!statPath(p, &st, true))
	{
		if (ec)
			*ec = ecFromErrno();
		return file_time_type::min();
	}
	if (ec)
		ec->clear();
#if defined(__APPLE__)
	struct ::timespec ts = st.st_mtimespec;
#else
	struct ::timespec ts{st.st_mtime, 0};
#endif
	return timespecToFileTime(ts);
}

void __last_write_time(const path &p, file_time_type new_time, error_code *ec)
{
	struct ::timeval tv[2];
	struct ::timespec at, mt;
	fileTimeToTimespec(new_time, &mt);
	at = mt;
	tv[0].tv_sec = static_cast<long>(at.tv_sec);
	tv[0].tv_usec = static_cast<long>(at.tv_nsec / 1000);
	tv[1].tv_sec = static_cast<long>(mt.tv_sec);
	tv[1].tv_usec = static_cast<long>(mt.tv_nsec / 1000);

	if (::utimes(p.c_str(), tv) != 0)
	{
		if (ec)
			*ec = ecFromErrno();
		else
			__throw_filesystem_error("filesystem error: utimes failed", p, ecFromErrno());
	}
	else if (ec)
		ec->clear();
}

// ── Paths ────────────────────────────────────────────────────────────────────

path __current_path(error_code *ec)
{
	char buf[PATH_MAX];
	if (::getcwd(buf, sizeof(buf)) == nullptr)
	{
		if (ec)
			*ec = ecFromErrno();
		return path();
	}
	if (ec)
		ec->clear();
	return path(buf);
}

void __current_path(const path &p, error_code *ec)
{
	if (::chdir(p.c_str()) != 0)
	{
		if (ec)
			*ec = ecFromErrno();
	}
	else if (ec)
		ec->clear();
}

path __absolute(const path &p, error_code *ec)
{
	if (p.is_absolute())
	{
		if (ec)
			ec->clear();
		return p;
	}
	path base = __current_path(ec);
	if (ec && *ec)
		return path();
	return base / p;
}

path __canonical(const path &p, error_code *ec)
{
	char buf[PATH_MAX];
	if (::realpath(p.c_str(), buf) == nullptr)
	{
		if (ec)
			*ec = ecFromErrno();
		else
			__throw_filesystem_error("filesystem error: canonical failed", p, ecFromErrno());
		return path();
	}
	if (ec)
		ec->clear();
	return path(buf);
}

path __weakly_canonical(const path &p, error_code *ec)
{
	// Canonicalise the longest existing prefix; append the remainder verbatim.
	path prefix = p;
	path suffix;
	while (!prefix.empty())
	{
		if (__status(prefix).type() != file_type::not_found)
			break;
		suffix = prefix.filename() / suffix;
		prefix = prefix.parent_path();
	}
	path result = prefix.empty() ? p : __canonical(prefix, ec);
	if (ec && *ec)
		return path();
	result /= suffix;
	if (ec)
		ec->clear();
	return result.lexically_normal();
}

bool __equivalent(const path &a, const path &b, error_code *ec)
{
	struct ::stat sa, sb;
	if (::stat(a.c_str(), &sa) != 0 || ::stat(b.c_str(), &sb) != 0)
	{
		if (ec)
			*ec = ecFromErrno();
		else
			__throw_filesystem_error("filesystem error: equivalent failed", a, b, ecFromErrno());
		return false;
	}
	if (ec)
		ec->clear();
	return sa.st_dev == sb.st_dev && sa.st_ino == sb.st_ino;
}

path __temp_directory_path(error_code *ec)
{
	const char *tmp = ::getenv("TMPDIR");
	if (tmp == nullptr || tmp[0] == '\0')
		tmp = "/tmp";
	path p(tmp);
	file_status st = __status(p);
	if (st.type() != file_type::directory)
	{
		if (ec)
			*ec = error_code(ENOENT, generic_category());
		else
			__throw_filesystem_error("filesystem error: temp dir not found", p, error_code(ENOENT, generic_category()));
		return path();
	}
	if (ec)
		ec->clear();
	return p;
}

// ── Links / copy (unused by the game; kept complete) ─────────────────────────

void __create_symlink(const path &to, const path &new_link, error_code *ec)
{
	if (::symlink(to.c_str(), new_link.c_str()) != 0)
	{
		if (ec)
			*ec = ecFromErrno();
	}
	else if (ec)
		ec->clear();
}

void __create_directory_symlink(const path &to, const path &new_link, error_code *ec)
{
	__create_symlink(to, new_link, ec);
}

void __create_hard_link(const path &to, const path &new_hard_link, error_code *ec)
{
	if (::link(to.c_str(), new_hard_link.c_str()) != 0)
	{
		if (ec)
			*ec = ecFromErrno();
	}
	else if (ec)
		ec->clear();
}

path __read_symlink(const path &p, error_code *ec)
{
	char buf[PATH_MAX];
	ssize_t len = ::readlink(p.c_str(), buf, sizeof(buf) - 1);
	if (len < 0)
	{
		if (ec)
			*ec = ecFromErrno();
		else
			__throw_filesystem_error("filesystem error: readlink failed", p, ecFromErrno());
		return path();
	}
	buf[len] = '\0';
	if (ec)
		ec->clear();
	return path(buf);
}

void __copy_symlink(const path &existing, const path &new_symlink, error_code *ec)
{
	path target = __read_symlink(existing, ec);
	if (ec && *ec)
		return;
	__create_symlink(target, new_symlink, ec);
}

bool __copy_file(const path &from, const path &to, copy_options /*opt*/, error_code *ec)
{
	FILE *in = fopen(from.c_str(), "rb");
	if (in == nullptr)
	{
		if (ec)
			*ec = ecFromErrno();
		return false;
	}
	FILE *out = fopen(to.c_str(), "wb");
	if (out == nullptr)
	{
		if (ec)
			*ec = ecFromErrno();
		fclose(in);
		return false;
	}
	char buffer[65536];
	size_t n;
	while ((n = fread(buffer, 1, sizeof(buffer), in)) > 0)
		fwrite(buffer, 1, n, out);
	bool ok = !ferror(out);
	fclose(in);
	fclose(out);
	if (ec)
		*ec = ok ? error_code() : ecFromErrno();
	return ok;
}

void __copy(const path &from, const path &to, copy_options opt, error_code *ec)
{
	file_status st = __symlink_status(from);
	if (st.type() == file_type::regular &&
	    ((opt & copy_options::directories_only) == copy_options::none))
	{
		__copy_file(from, to, opt, ec);
		return;
	}
	if (st.type() == file_type::directory)
	{
		__create_directory(to, ec);
		DIR *dir = opendir(from.c_str());
		if (dir == nullptr)
			return;
		while (dirent *e = readdir(dir))
		{
			if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
				continue;
			__copy(from / e->d_name, to / e->d_name, opt | copy_options::recursive, ec);
		}
		closedir(dir);
	}
}

_LIBCPP_END_NAMESPACE_FILESYSTEM
