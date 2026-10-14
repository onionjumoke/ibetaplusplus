#include "GuiConnecting.h"
#include "ThreadConnectToServer.h"
#include "NetClientHandler.h"
#include "StringTranslate.h"
#include "GuiButton.h"
#include "GuiMainMenu.h"
#include "FontRenderer.h"
#include "Minecraft.h"
#include <iostream>

GuiConnecting::GuiConnecting(Minecraft *minecraft, const std::string &host, int_t port)
	: clientHandler(nullptr)
	, cancelled(false)
{
	std::cout << "Connecting to " << host << ", " << port << std::endl;
	minecraft->changeWorld1(nullptr);
	(new ThreadConnectToServer(this, minecraft, host, port))->start();
}

void GuiConnecting::updateScreen()
{
	if (clientHandler != nullptr)
	{
		clientHandler->processReadPackets();
	}
}

void GuiConnecting::keyTyped(char_t c, int_t i)
{
}

void GuiConnecting::initGui()
{
	StringTranslate *stringtranslate = StringTranslate::getInstance();
	controlList.clear();
	controlList.push_back(new GuiButton(0, width / 2 - 100, height / 4 + 120 + 12, stringtranslate->translateKey("gui.cancel")));
}

void GuiConnecting::actionPerformed(GuiButton *guibutton)
{
	if (guibutton->id == 0)
	{
		cancelled = true;
		if (clientHandler != nullptr)
		{
			clientHandler->disconnect();
		}
		mc->displayGuiScreen(new GuiMainMenu());
	}
}

void GuiConnecting::drawScreen(int_t i, int_t j, float_t f)
{
	drawDefaultBackground();
	StringTranslate *stringtranslate = StringTranslate::getInstance();
	if (clientHandler == nullptr)
	{
		drawCenteredString(fontRenderer, stringtranslate->translateKey("connect.connecting"),   width / 2, height / 2 - 50, 0xffffff);
		drawCenteredString(fontRenderer, "",                                                    width / 2, height / 2 - 10, 0xffffff);
	}
	else
	{
		drawCenteredString(fontRenderer, stringtranslate->translateKey("connect.authorizing"),  width / 2, height / 2 - 50, 0xffffff);
		drawCenteredString(fontRenderer, clientHandler->getServerHostname(),                   width / 2, height / 2 - 10, 0xffffff);
	}
	GuiScreen::drawScreen(i, j, f);
}

void GuiConnecting::setNetClientHandler(GuiConnecting *guiconnecting, NetClientHandler *netclienthandler)
{
	guiconnecting->clientHandler = netclienthandler;
}

NetClientHandler *GuiConnecting::getNetClientHandler(GuiConnecting *guiconnecting)
{
	return guiconnecting->clientHandler;
}

bool GuiConnecting::isCancelled(GuiConnecting *guiconnecting)
{
	return guiconnecting->cancelled;
}
