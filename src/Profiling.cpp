// Profiling.cpp — see Profiling.h. Plain C++ so any TU can include it.
#include "Profiling.h"

#include <time.h>
#include <sys/time.h>

namespace Profiling
{

namespace
{
double g_accNs[S_COUNT] = {0};
int g_count = 0;
} // namespace

int64_t Auto::now()
{
	// gettimeofday: the iOS 5 SDK gates CLOCK_MONOTONIC behind
	// __DARWIN_UNIX03; microsecond resolution is plenty at ms granularity.
	timeval tv;
	::gettimeofday(&tv, nullptr);
	return static_cast<int64_t>(tv.tv_sec) * 1000000000LL +
	       static_cast<int64_t>(tv.tv_usec) * 1000LL;
}

void add(int slot, int64_t nanos)
{
	if (slot >= 0 && slot < S_COUNT)
		g_accNs[slot] += static_cast<double>(nanos);
}

void resetAll()
{
	for (int i = 0; i < S_COUNT; ++i)
		g_accNs[i] = 0;
	g_count = 0;
}

double takeAvgMs(int slot, int *countOut)
{
	if (countOut != nullptr)
		*countOut = g_count;
	double ms = g_count > 0 ? g_accNs[slot] / g_count / 1e6 : 0.0;
	g_accNs[slot] = 0;
	return ms;
}

void advanceFrame() { g_count++; }

int currentCount() { return g_count; }

} // namespace Profiling
