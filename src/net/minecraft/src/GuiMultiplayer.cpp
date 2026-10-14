#include "GuiMultiplayer.h"
#include "GuiTextField.h"
#include "GuiButton.h"
#include "GuiConnecting.h"
#include "StringTranslate.h"
#include "GameSettings.h"
#include "Minecraft.h"
#include "pc/lwjgl/Keyboard.h"
#include <algorithm>
#include <stdexcept>

GuiMultiplayer::GuiMultiplayer(GuiScreen *parent)
	: parentScreen(parent)
	, serverAddressField(nullptr)
{
}

void GuiMultiplayer::updateScreen()
{
	serverAddressField->updateCursorCounter();
}

void GuiMultiplayer::initGui()
{
	StringTranslate *tr = StringTranslate::getInstance();
	lwjgl::Keyboard::enableRepeatEvents(true);
	controlList.clear();
	controlList.push_back(new GuiButton(0, width / 2 - 100, height / 4 + 96  + 12, tr->translateKey("multiplayer.connect")));
	controlList.push_back(new GuiButton(1, width / 2 - 100, height / 4 + 120 + 12, tr->translateKey("gui.cancel")));
	std::string lastServer = mc->gameSettings->lastServer;
	// Java: replaceAll("_", ":")
	std::replace(lastServer.begin(), lastServer.end(), '_', ':');
	controlList[0]->enabled = !lastServer.empty();
	serverAddressField = new GuiTextField(this, fontRenderer,
	    width / 2 - 100, (height / 4 - 10) + 50 + 18, 200, 20, lastServer);
	serverAddressField->isFocused = true;
	serverAddressField->setMaxStringLength(128);
}

void GuiMultiplayer::onGuiClosed()
{
	lwjgl::Keyboard::enableRepeatEvents(false);
}

void GuiMultiplayer::actionPerformed(GuiButton *button)
{
	if (!button->enabled) return;
	if (button->id == 1)
	{
		mc->displayGuiScreen(parentScreen);
	}
	else if (button->id == 0)
	{
		std::string s = serverAddressField->getText();
		while (!s.empty() && s.front() == ' ') s.erase(s.begin());
		while (!s.empty() && s.back()  == ' ') s.pop_back();
		std::string saved = s;
		std::replace(saved.begin(), saved.end(), ':', '_');
		mc->gameSettings->lastServer = saved;
		mc->gameSettings->saveOptions();

		std::string host = s;
		int_t port = 25565;

		// Parse IPv6 [::1]:port
		if (!s.empty() && s.front() == '[')
		{
			size_t bracket = s.find(']');
			if (bracket != std::string::npos)
			{
				host = s.substr(1, bracket - 1);
				std::string rest = s.substr(bracket + 1);
				if (!rest.empty() && rest.front() == ':' && rest.length() > 1)
					port = parseIntWithDefault(rest.substr(1), 25565);
			}
		}
		else
		{
			size_t colon = s.rfind(':');
			if (colon != std::string::npos)
			{
				host = s.substr(0, colon);
				port = parseIntWithDefault(s.substr(colon + 1), 25565);
			}
		}
		mc->displayGuiScreen(new GuiConnecting(mc, host, port));
	}
}

int_t GuiMultiplayer::parseIntWithDefault(const std::string &s, int_t def)
{
	try { return std::stoi(s); }
	catch (...) { return def; }
}

void GuiMultiplayer::keyTyped(char_t c, int_t key)
{
	serverAddressField->textboxKeyTyped(c, key);
	if (c == '\r')
		actionPerformed(controlList[0]);
	controlList[0]->enabled = !serverAddressField->getText().empty();
}

void GuiMultiplayer::mouseClicked(int_t x, int_t y, int_t button)
{
	GuiScreen::mouseClicked(x, y, button);
	serverAddressField->mouseClicked(x, y, button);
}

void GuiMultiplayer::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	StringTranslate *tr = StringTranslate::getInstance();
	drawDefaultBackground();
	drawCenteredString(fontRenderer, tr->translateKey("multiplayer.title"),    width / 2, (height / 4 - 60) + 20, 0xffffff);
	drawString(fontRenderer, tr->translateKey("multiplayer.info1"),  width / 2 - 140, (height / 4 - 60) + 60 + 0, 0xa0a0a0);
	drawString(fontRenderer, tr->translateKey("multiplayer.info2"),  width / 2 - 140, (height / 4 - 60) + 60 + 9, 0xa0a0a0);
	drawString(fontRenderer, tr->translateKey("multiplayer.ipinfo"), width / 2 - 140, (height / 4 - 60) + 60 + 36, 0xa0a0a0);
	serverAddressField->drawTextBox();
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}

GuiTextField *GuiMultiplayer::getFocusedTextField()
{
	return (serverAddressField != nullptr && serverAddressField->isFocused) ? serverAddressField : nullptr;
}
