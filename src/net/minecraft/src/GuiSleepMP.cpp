#include "GuiSleepMP.h"
#include "GuiButton.h"
#include "StringTranslate.h"
#include "EntityClientPlayerMP.h"
#include "NetClientHandler.h"
#include "Packet19EntityAction.h"
#include "Minecraft.h"
#include "pc/lwjgl/Keyboard.h"

GuiSleepMP::GuiSleepMP()
{
}

void GuiSleepMP::initGui()
{
	lwjgl::Keyboard::enableRepeatEvents(true);
	StringTranslate *tr = StringTranslate::getInstance();
	controlList.push_back(new GuiButton(1, width / 2 - 100, height - 40, tr->translateKey("multiplayer.stopSleeping")));
}

void GuiSleepMP::onGuiClosed()
{
	lwjgl::Keyboard::enableRepeatEvents(false);
}

void GuiSleepMP::keyTyped(char_t c, int_t key)
{
	if (key == 1)
	{
		stopSleeping();
	}
	else if (key == 28) // Enter
	{
		std::string s = message;
		while (!s.empty() && s.front() == ' ') s.erase(s.begin());
		while (!s.empty() && s.back()  == ' ') s.pop_back();
		if (!s.empty())
			mc->thePlayer->sendChatMessage(s);
		message = "";
	}
	else
	{
		GuiChat::keyTyped(c, key);
	}
}

void GuiSleepMP::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	GuiChat::drawScreen(mouseX, mouseY, partialTick);
}

void GuiSleepMP::actionPerformed(GuiButton *button)
{
	if (button->id == 1)
	{
		stopSleeping();
	}
	else
	{
		GuiChat::actionPerformed(button);
	}
}

void GuiSleepMP::stopSleeping()
{
	EntityClientPlayerMP *mp = dynamic_cast<EntityClientPlayerMP *>(mc->thePlayer);
	if (mp != nullptr)
	{
		mp->sendQueue->addToSendQueue(new Packet19EntityAction(mc->thePlayer, 3));
	}
}
