#include "GuiCreateWorld.h"
#include "GuiTextField.h"
#include "GuiButton.h"
#include "StringTranslate.h"
#include "ChatAllowedCharacters.h"
#include "MathHelper.h"
#include "ISaveFormat.h"
#include "PlayerControllerSP.h"
#include "Minecraft.h"
#include "World.h"
#include "java/Random.h"
#include "java/String.h"
#include "pc/lwjgl/Keyboard.h"
#include <algorithm>
#include <stdexcept>

GuiCreateWorld::GuiCreateWorld(GuiScreen *parent)
	: parentScreen(parent)
	, textboxWorldName(nullptr)
	, textboxSeed(nullptr)
	, createClicked(false)
{
}

void GuiCreateWorld::updateScreen()
{
	textboxWorldName->updateCursorCounter();
	textboxSeed->updateCursorCounter();
}

void GuiCreateWorld::initGui()
{
	StringTranslate *tr = StringTranslate::getInstance();
	lwjgl::Keyboard::enableRepeatEvents(true);
	controlList.clear();
	controlList.push_back(new GuiButton(0, width / 2 - 100, height / 4 + 96  + 12, tr->translateKey("selectWorld.create")));
	controlList.push_back(new GuiButton(1, width / 2 - 100, height / 4 + 120 + 12, tr->translateKey("gui.cancel")));
	controlList.push_back(new GuiButton(2, width / 2 - 100, 140, 200, 20,
	                                    worldType == 1 ? "World Type: Superflat" : "World Type: Default"));
	textboxWorldName = new GuiTextField(this, fontRenderer, width / 2 - 100, 60, 200, 20, tr->translateKey("selectWorld.newWorld"));
	textboxWorldName->isFocused = true;
	textboxWorldName->setMaxStringLength(32);
	textboxSeed = new GuiTextField(this, fontRenderer, width / 2 - 100, 116, 200, 20, "");
	updateFolderName();
}

void GuiCreateWorld::updateFolderName()
{
	folderName = textboxWorldName->getText();
	// trim
	while (!folderName.empty() && folderName.front() == ' ') folderName.erase(folderName.begin());
	while (!folderName.empty() && folderName.back()  == ' ') folderName.pop_back();
	// Replace illegal filename characters
	for (char c : ChatAllowedCharacters::allowedCharactersArray)
		std::replace(folderName.begin(), folderName.end(), c, '_');
	if (MathHelper::stringNullOrLengthZero(String::fromUTF8(folderName)))
		folderName = "World";
	folderName = generateUnusedFolderName(mc->getSaveLoader(), folderName);
}

std::string GuiCreateWorld::generateUnusedFolderName(ISaveFormat *fmt, const std::string &base)
{
	std::string s = base;
	while (fmt->getWorldInfo(String::fromUTF8(s)) != nullptr)
		s += "-";
	return s;
}

void GuiCreateWorld::onGuiClosed()
{
	lwjgl::Keyboard::enableRepeatEvents(false);
}

void GuiCreateWorld::actionPerformed(GuiButton *button)
{
	if (!button->enabled) return;
	if (button->id == 1)
	{
		mc->displayGuiScreen(parentScreen);
	}
	else if (button->id == 2)
	{
		worldType = (worldType + 1) % 2;
		button->displayString = worldType == 1 ? "World Type: Superflat" : "World Type: Default";
	}
	else if (button->id == 0)
	{
		mc->displayGuiScreen(nullptr);
		if (createClicked) return;
		createClicked = true;
		Random rand;
		long_t seed = rand.nextLong();
		std::string seedStr = textboxSeed->getText();
		if (!MathHelper::stringNullOrLengthZero(String::fromUTF8(seedStr)))
		{
			try
			{
				long_t parsed = std::stoll(seedStr);
				if (parsed != 0LL) seed = parsed;
			}
			catch (...)
			{
				// hash the string
				seed = 0;
				for (char c : seedStr) seed = seed * 31 + c;
			}
		}
		mc->playerController = new PlayerControllerSP(mc);
		World::setNextWorldType(worldType);
		mc->startWorld(folderName, textboxWorldName->getText(), seed);
		mc->displayGuiScreen(nullptr);
	}
}

void GuiCreateWorld::keyTyped(char_t c, int_t key)
{
	if (textboxWorldName->isFocused)
		textboxWorldName->textboxKeyTyped(c, key);
	else
		textboxSeed->textboxKeyTyped(c, key);

	if (c == '\r')
		actionPerformed(controlList[0]);

	controlList[0]->enabled = !textboxWorldName->getText().empty();
	updateFolderName();
}

void GuiCreateWorld::mouseClicked(int_t x, int_t y, int_t button)
{
	GuiScreen::mouseClicked(x, y, button);
	textboxWorldName->mouseClicked(x, y, button);
	textboxSeed->mouseClicked(x, y, button);
}

void GuiCreateWorld::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	StringTranslate *tr = StringTranslate::getInstance();
	drawDefaultBackground();
	drawCenteredString(fontRenderer, tr->translateKey("selectWorld.create"), width / 2, (height / 4 - 60) + 20, 0xffffff);
	drawString(fontRenderer, tr->translateKey("selectWorld.enterName"), width / 2 - 100, 47, 0xa0a0a0);
	drawString(fontRenderer, tr->translateKey("selectWorld.resultFolder") + " " + folderName, width / 2 - 100, 85, 0xa0a0a0);
	drawString(fontRenderer, tr->translateKey("selectWorld.enterSeed"),   width / 2 - 100, 104, 0xa0a0a0);
	drawString(fontRenderer, tr->translateKey("selectWorld.seedInfo"),    width / 2 - 100, 140, 0xa0a0a0);
	textboxWorldName->drawTextBox();
	textboxSeed->drawTextBox();
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}

void GuiCreateWorld::selectNextField()
{
	if (textboxWorldName->isFocused)
	{
		textboxWorldName->setFocused(false);
		textboxSeed->setFocused(true);
	}
	else
	{
		textboxWorldName->setFocused(true);
		textboxSeed->setFocused(false);
	}
}

GuiTextField *GuiCreateWorld::getFocusedTextField()
{
	if (textboxWorldName != nullptr && textboxWorldName->isFocused) return textboxWorldName;
	if (textboxSeed != nullptr && textboxSeed->isFocused) return textboxSeed;
	return nullptr;
}
