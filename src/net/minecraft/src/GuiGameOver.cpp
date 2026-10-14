#include "GuiGameOver.h"
#include "GuiButton.h"
#include "GuiMainMenu.h"
#include "FontRenderer.h"
#include "Minecraft.h"
#include "EntityPlayerSP.h"
#include "pc/OpenGL.h"
#include <string>

GuiGameOver::GuiGameOver()
{
}

void GuiGameOver::initGui()
{
	controlList.clear();
	controlList.push_back(new GuiButton(1, width / 2 - 100, height / 4 + 72, "Respawn"));
	controlList.push_back(new GuiButton(2, width / 2 - 100, height / 4 + 96, "Title menu"));
	if (mc->session == nullptr)
	{
		controlList[1]->enabled = false;
	}
}

void GuiGameOver::keyTyped(char_t c, int_t i)
{
}

void GuiGameOver::actionPerformed(GuiButton *guibutton)
{
	if (guibutton->id == 1)
	{
		mc->thePlayer->respawnPlayer();
		mc->displayGuiScreen(nullptr);
	}
	if (guibutton->id == 2)
	{
		mc->changeWorld1(nullptr);
		mc->displayGuiScreen(new GuiMainMenu());
	}
}

void GuiGameOver::drawScreen(int_t i, int_t j, float_t f)
{
	drawGradientRect(0, 0, width, height, 0x60500000, 0xa0803030);
	glPushMatrix();
	glScalef(2.0f, 2.0f, 2.0f);
	drawCenteredString(fontRenderer, "Game over!", width / 2 / 2, 30, 0xffffff);
	glPopMatrix();
	drawCenteredString(fontRenderer, "Score: &e" + std::to_string(mc->thePlayer->getScore()), width / 2, 100, 0xffffff);
	GuiScreen::drawScreen(i, j, f);
}

bool GuiGameOver::doesGuiPauseGame()
{
	return false;
}
