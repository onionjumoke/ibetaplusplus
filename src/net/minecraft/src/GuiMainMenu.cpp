#include "GuiMainMenu.h"
#include "GuiButton.h"
#include "GuiOptions.h"
#include "GuiSelectWorld.h"
#include "GuiMultiplayer.h"
#include "GuiTexturePacks.h"
#include "StringTranslate.h"
#include "Tessellator.h"
#include "MathHelper.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "Minecraft.h"
#include "GameResources.h"
#include "java/System.h"
#include "java/Random.h"
#include "pc/OpenGL.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>
#include <cmath>

static Random g_mainMenuRand;

GuiMainMenu::GuiMainMenu()
	: updateCounter(0.0f)
	, splashText("missingno")
	, multiplayerButton(nullptr)
{
	try
	{
		std::string resolved = GameResources::resolve("/title/splashes.txt");
		std::ifstream f(resolved);
		std::vector<std::string> lines;
		if (f.is_open())
		{
			std::string line;
			while (std::getline(f, line))
			{
				while (!line.empty() && (line.front() == ' ' || line.front() == '\r')) line.erase(line.begin());
				while (!line.empty() && (line.back()  == ' ' || line.back()  == '\r')) line.pop_back();
				if (!line.empty()) lines.push_back(line);
			}
		}
		if (!lines.empty())
			splashText = lines[g_mainMenuRand.nextInt((int_t)lines.size())];
	}
	catch (...) {}
}

void GuiMainMenu::updateScreen()
{
	updateCounter++;
}

void GuiMainMenu::keyTyped(char_t c, int_t key)
{
}

void GuiMainMenu::initGui()
{
	// Check special dates
	time_t t = time(nullptr);
	struct tm *now = localtime(&t);
	int month = now->tm_mon + 1;
	int day   = now->tm_mday;
	if      (month == 11 && day == 9)  splashText = "Happy birthday, ez!";
	else if (month ==  6 && day == 1)  splashText = "Happy birthday, Notch!";
	else if (month == 12 && day == 24) splashText = "Merry X-mas!";
	else if (month ==  1 && day == 1)  splashText = "Happy new year!";

	StringTranslate *tr = StringTranslate::getInstance();
	int_t y = height / 4 + 48;
	controlList.push_back(new GuiButton(1, width / 2 - 100, y,      tr->translateKey("menu.singleplayer")));
	controlList.push_back(multiplayerButton = new GuiButton(2, width / 2 - 100, y + 24,  tr->translateKey("menu.multiplayer")));
	controlList.push_back(new GuiButton(3, width / 2 - 100, y + 48, tr->translateKey("menu.mods")));
	if (mc->hideQuitButton)
	{
		controlList.push_back(new GuiButton(0, width / 2 - 100, y + 72, tr->translateKey("menu.options")));
	}
	else
	{
		controlList.push_back(new GuiButton(0, width / 2 - 100, y + 72 + 12, 98, 20, tr->translateKey("menu.options")));
		controlList.push_back(new GuiButton(4, width / 2 + 2,   y + 72 + 12, 98, 20, tr->translateKey("menu.quit")));
	}
	if (mc->session == nullptr)
		multiplayerButton->enabled = false;
}

void GuiMainMenu::actionPerformed(GuiButton *button)
{
	if (button->id == 0) mc->displayGuiScreen(new GuiOptions(this, mc->gameSettings));
	if (button->id == 1) mc->displayGuiScreen(new GuiSelectWorld(this));
	if (button->id == 2) mc->displayGuiScreen(new GuiMultiplayer(this));
	if (button->id == 3) mc->displayGuiScreen(new GuiTexturePacks(this));
	if (button->id == 4) mc->shutdown();
}

void GuiMainMenu::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawDefaultBackground();
	Tessellator *tess = &Tessellator::instance;
	int_t logoW = 274; // 0x0112
	int_t logoX = width / 2 - logoW / 2;
	int_t logoY = 30;
	glBindTexture(GL_TEXTURE_2D, mc->renderEngine->getTexture("/title/mclogo.png"));
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	drawTexturedModalRect(logoX + 0,   logoY, 0,   0,  155, 44);
	drawTexturedModalRect(logoX + 155, logoY, 0,  45,  155, 44);

	tess->setColorOpaque_I(0xffffff);
	glPushMatrix();
	glTranslatef((float_t)(width / 2 + 90), 70.0f, 0.0f);
	glRotatef(-20.0f, 0.0f, 0.0f, 1.0f);
	float_t f1 = 1.8f - MathHelper::abs(MathHelper::sin(
		((float_t)(System::currentTimeMillis() % 1000LL) / 1000.0f) * 3.141593f * 2.0f) * 0.1f);
	f1 = (f1 * 100.0f) / (float_t)(fontRenderer->getStringWidth(splashText) + 32);
	glScalef(f1, f1, f1);
	drawCenteredString(fontRenderer, splashText, 0, -8, 0xffff00);
	glPopMatrix();

	drawString(fontRenderer, "iBetaPlusPlus", 2, 2, 0x505050);
	std::string copy = "Copyright Mojang AB. Do not distribute.";
	drawString(fontRenderer, copy, width - fontRenderer->getStringWidth(copy) - 2, height - 10, 0xffffff);

	// Non-affiliation disclaimer (kept short so it fits the 4S width).
	drawCenteredString(fontRenderer, "Not affiliated with Mojang or Microsoft",
	                   width / 2, height - 22, 0x707070);

	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
