#include "GuiIngameMenu.h"
#include "GuiButton.h"
#include "GameSettings.h"
#include "GuiOptions.h"
#include "GuiMainMenu.h"
#include "GuiAchievements.h"
#include "GuiStats.h"
#include "StatCollector.h"
#include "StatList.h"
#include "MathHelper.h"
#include "FontRenderer.h"
#include "Minecraft.h"
#include "StatFileWriter.h"
#include "World.h"
#include "EntityPlayerSP.h"

GuiIngameMenu::GuiIngameMenu()
	: updateCounter2(0)
	, updateCounter(0)
{
}

void GuiIngameMenu::initGui()
{
	updateCounter2 = 0;
	controlList.clear();
	int_t off = -16;
	controlList.push_back(new GuiButton(1, width / 2 - 100, height / 4 + 120 + off, "Save and quit to title"));
	if (mc->isMultiplayerWorld())
		controlList[0]->displayString = "Disconnect";
	controlList.push_back(new GuiButton(4, width / 2 - 100, height / 4 + 24 + off, "Back to game"));
	controlList.push_back(new GuiButton(0, width / 2 - 100, height / 4 + 96 + off, 98, 20, "Options..."));
	if (!mc->isMultiplayerWorld())
		controlList.push_back(new GuiButton(102, width / 2 + 2, height / 4 + 96 + off, 98, 20, "Unstuck"));
	controlList.push_back(new GuiButton(5, width / 2 - 100, height / 4 + 48 + off, 98, 20, StatCollector::translateToLocal("gui.achievements")));
	controlList.push_back(new GuiButton(6, width / 2 + 2,   height / 4 + 48 + off, 98, 20, StatCollector::translateToLocal("gui.stats")));

}

void GuiIngameMenu::actionPerformed(GuiButton *button)
{
	if (button->id == 0)
	{
		mc->displayGuiScreen(new GuiOptions(this, mc->gameSettings));
	}
	if (button->id == 1)
	{
		mc->statFileWriter->readStat(StatList::leaveGameStat, 1);
		if (mc->isMultiplayerWorld())
			mc->theWorld->sendQuittingDisconnectingPacket();
		mc->changeWorld1(nullptr);
		mc->displayGuiScreen(new GuiMainMenu());
	}
	if (button->id == 4)
	{
		mc->displayGuiScreen(nullptr);
		mc->setIngameFocus();
	}
	if (button->id == 5)
	{
		mc->displayGuiScreen(new GuiAchievements(mc->statFileWriter));
	}
	if (button->id == 6)
	{
		mc->displayGuiScreen(new GuiStats(this, mc->statFileWriter));
	}
	if (button->id == 102)
	{
		if (mc->theWorld != nullptr && mc->thePlayer != nullptr)
			mc->theWorld->unstickPlayer(mc->thePlayer);
		mc->displayGuiScreen(nullptr);
		mc->setIngameFocus();
	}
}

void GuiIngameMenu::updateScreen()
{
	GuiScreen::updateScreen();
	updateCounter++;
}

void GuiIngameMenu::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawDefaultBackground();
	bool saving = !mc->theWorld->isSafeToSave(updateCounter2++);
	if (saving || updateCounter < 20)
	{
		float_t f1 = ((float_t)(updateCounter % 10) + partialTick) / 10.0f;
		f1 = MathHelper::sin(f1 * 3.141593f * 2.0f) * 0.2f + 0.8f;
		int_t k = (int_t)(255.0f * f1);
		drawString(fontRenderer, "Saving level..", 8, height - 16, k << 16 | k << 8 | k);
	}
	drawCenteredString(fontRenderer, "Game menu", width / 2, 40, 0xffffff);
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
