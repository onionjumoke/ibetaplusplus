#include "ThreadConnectToServer.h"

#include <exception>
#include <iostream>

#include "GuiConnectFailed.h"
#include "GuiConnecting.h"
#include "Minecraft.h"
#include "NetClientHandler.h"
#include "Packet2Handshake.h"
#include "Session.h"
#include "java/String.h"

ThreadConnectToServer::ThreadConnectToServer(GuiConnecting *guiconnecting, Minecraft *minecraft, const std::string &s, int_t i)
	: mc(minecraft)
	, hostName(s)
	, port(i)
	, connectingGui(guiconnecting)
{
}

ThreadConnectToServer::~ThreadConnectToServer()
{
	if (worker.joinable())
		worker.detach();
}

void ThreadConnectToServer::start()
{
	worker = std::thread(&ThreadConnectToServer::run, this);
	worker.detach();
}

void ThreadConnectToServer::run()
{
	try
	{
		GuiConnecting::setNetClientHandler(connectingGui, new NetClientHandler(mc, hostName, port));
		if (GuiConnecting::isCancelled(connectingGui))
			return;
		GuiConnecting::getNetClientHandler(connectingGui)->addToSendQueue(new Packet2Handshake(String::fromUTF8(mc->session->username)));
	}
	catch (std::exception &exception)
	{
		if (GuiConnecting::isCancelled(connectingGui))
			return;
		std::cerr << exception.what() << std::endl;
		mc->displayGuiScreen(new GuiConnectFailed("connect.failed", "disconnect.genericReason", exception.what()));
	}
}
