#pragma once

#include <string>
#include <thread>

class Minecraft;

// net.minecraft.src.ThreadSleepForever
class ThreadSleepForever
{
public:
	ThreadSleepForever(Minecraft *minecraft, const std::string &s);
	~ThreadSleepForever();

	void run();

private:
	Minecraft *mc;
	std::thread worker;
};
