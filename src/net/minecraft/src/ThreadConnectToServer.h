#pragma once

#include <string>
#include <thread>

#include "java/Type.h"

class GuiConnecting;
class Minecraft;

// net.minecraft.src.ThreadConnectToServer
class ThreadConnectToServer
{
public:
	ThreadConnectToServer(GuiConnecting *guiconnecting, Minecraft *minecraft, const std::string &s, int_t i);
	~ThreadConnectToServer();

	void start();
	void run();

private:
	Minecraft *mc;
	std::string hostName;
	int_t port;
	GuiConnecting *connectingGui;
	std::thread worker;
};
