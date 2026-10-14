#include "ThreadSleepForever.h"

#include <chrono>

#include "Minecraft.h"

ThreadSleepForever::ThreadSleepForever(Minecraft *minecraft, const std::string &)
	: mc(minecraft)
	, worker(&ThreadSleepForever::run, this)
{
	worker.detach();
}

ThreadSleepForever::~ThreadSleepForever()
{
	if (worker.joinable())
		worker.detach();
}

void ThreadSleepForever::run()
{
	while (mc->running)
		std::this_thread::sleep_for(std::chrono::milliseconds(0x7fffffff));
}
