#include "Timer.h"

#include "java/System.h"

Timer::Timer(float f) :
	ticksPerSecond(f),
	elapsedTicks(0),
	renderPartialTicks(0.0f),
	timerSpeed(1.0f),
	elapsedPartialTicks(0.0f),
	lastHRTime(0.0),
	lastSyncSysClock(System::currentTimeMillis()),
	lastSyncHRClock(System::nanoTime() / 0xf4240LL),
	accumulatedSysClock(0),
	timeSyncAdjustment(1.0)
{
}

void Timer::updateTimer()
{
	long_t l = System::currentTimeMillis();
	long_t l1 = l - lastSyncSysClock;
	long_t l2 = System::nanoTime() / 0xf4240LL;
	double d = (double)l2 / 1000.0;
	if (l1 > 1000LL)
	{
		lastHRTime = d;
	}
	else if (l1 < 0LL)
	{
		lastHRTime = d;
	}
	else
	{
		accumulatedSysClock += l1;
		if (accumulatedSysClock > 1000LL)
		{
			long_t l3 = l2 - lastSyncHRClock;
			double d2 = (double)accumulatedSysClock / (double)l3;
			timeSyncAdjustment += (d2 - timeSyncAdjustment) * 0.20000000298023224;
			lastSyncHRClock = l2;
			accumulatedSysClock = 0LL;
		}
		if (accumulatedSysClock < 0LL)
		{
			lastSyncHRClock = l2;
		}
	}
	lastSyncSysClock = l;
	double d1 = (d - lastHRTime) * timeSyncAdjustment;
	lastHRTime = d;
	if (d1 < 0.0)
	{
		d1 = 0.0;
	}
	if (d1 > 1.0)
	{
		d1 = 1.0;
	}
	elapsedPartialTicks += d1 * (double)timerSpeed * (double)ticksPerSecond;
	elapsedTicks = (int_t)elapsedPartialTicks;
	elapsedPartialTicks -= elapsedTicks;
	if (elapsedTicks > 10)
	{
		elapsedTicks = 10;
	}
	renderPartialTicks = elapsedPartialTicks;
}
