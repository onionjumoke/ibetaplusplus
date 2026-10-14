#include "GuiChat.h"
#include "ChatAllowedCharacters.h"
#include "GuiIngame.h"
#include "EntityPlayerSP.h"
#include "Minecraft.h"
#include "pc/lwjgl/Keyboard.h"

GuiChat::GuiChat()
	: message("")
	, updateCounter(0)
{
}

void GuiChat::initGui()
{
	lwjgl::Keyboard::enableRepeatEvents(true);
}

void GuiChat::onGuiClosed()
{
	lwjgl::Keyboard::enableRepeatEvents(false);
}

void GuiChat::updateScreen()
{
	updateCounter++;
}

void GuiChat::keyTyped(char_t c, int_t key)
{
	if (key == 1)
	{
		mc->displayGuiScreen(nullptr);
		return;
	}
	if (key == 28) // Enter
	{
		std::string s = message;
		// trim
		while (!s.empty() && s.front() == ' ') s.erase(s.begin());
		while (!s.empty() && s.back()  == ' ') s.pop_back();
		if (!s.empty())
		{
			std::string trimmed = message;
			while (!trimmed.empty() && trimmed.front() == ' ') trimmed.erase(trimmed.begin());
			while (!trimmed.empty() && trimmed.back()  == ' ') trimmed.pop_back();
			if (!mc->lineIsCommand(trimmed))
				mc->thePlayer->sendChatMessage(trimmed);
		}
		mc->displayGuiScreen(nullptr);
		return;
	}
	if (key == 14 && !message.empty()) // Backspace
		message = message.substr(0, message.length() - 1);

	if (ChatAllowedCharacters::allowedCharacters.find(c) != std::string::npos && (int_t)message.length() < 100)
		message += c;
}

void GuiChat::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawRect(2, height - 14, width - 2, height - 2, 0x80000000);
	std::string cursor = ((updateCounter / 6) % 2 != 0) ? "" : "_";
	drawString(fontRenderer, "> " + message + cursor, 4, height - 12, 0xe0e0e0);
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}

void GuiChat::mouseClicked(int_t x, int_t y, int_t button)
{
	if (button != 0) return;

	if (mc->ingameGUI->field_933_a.empty())
	{
		GuiScreen::mouseClicked(x, y, button);
		return;
	}
	if (!message.empty() && message.back() != ' ')
		message += ' ';
	message += mc->ingameGUI->field_933_a;
	if ((int_t)message.length() > 100)
		message = message.substr(0, 100);
}
