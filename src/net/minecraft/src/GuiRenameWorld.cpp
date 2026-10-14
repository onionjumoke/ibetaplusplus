#include "GuiRenameWorld.h"
#include "GuiTextField.h"
#include "GuiButton.h"
#include "StringTranslate.h"
#include "ISaveFormat.h"
#include "WorldInfo.h"
#include "Minecraft.h"
#include "pc/lwjgl/Keyboard.h"
#include "java/String.h"

GuiRenameWorld::GuiRenameWorld(GuiScreen *parent, const std::string &fname)
	: parentScreen(parent)
	, saveFileName(fname)
	, textbox(nullptr)
{
}

void GuiRenameWorld::updateScreen()
{
	textbox->updateCursorCounter();
}

void GuiRenameWorld::initGui()
{
	StringTranslate *tr = StringTranslate::getInstance();
	lwjgl::Keyboard::enableRepeatEvents(true);
	controlList.clear();
	controlList.push_back(new GuiButton(0, width / 2 - 100, height / 4 + 96  + 12, tr->translateKey("selectWorld.renameButton")));
	controlList.push_back(new GuiButton(1, width / 2 - 100, height / 4 + 120 + 12, tr->translateKey("gui.cancel")));
	ISaveFormat *fmt = mc->getSaveLoader();
	WorldInfo *info = fmt->getWorldInfo(String::fromUTF8(saveFileName));
	std::string name = info ? String::toUTF8(info->getWorldName()) : "";
	textbox = new GuiTextField(this, fontRenderer, width / 2 - 100, 60, 200, 20, name);
	textbox->isFocused = true;
	textbox->setMaxStringLength(32);
}

void GuiRenameWorld::onGuiClosed()
{
	lwjgl::Keyboard::enableRepeatEvents(false);
}

void GuiRenameWorld::actionPerformed(GuiButton *button)
{
	if (!button->enabled) return;
	if (button->id == 1)
	{
		mc->displayGuiScreen(parentScreen);
	}
	else if (button->id == 0)
	{
		ISaveFormat *fmt = mc->getSaveLoader();
		std::string newName = textbox->getText();
		while (!newName.empty() && newName.back() == ' ') newName.pop_back();
		while (!newName.empty() && newName.front() == ' ') newName.erase(newName.begin());
		fmt->renameWorld(String::fromUTF8(saveFileName), String::fromUTF8(newName));
		mc->displayGuiScreen(parentScreen);
	}
}

void GuiRenameWorld::keyTyped(char_t c, int_t key)
{
	textbox->textboxKeyTyped(c, key);
	std::string t = textbox->getText();
	while (!t.empty() && t.back() == ' ') t.pop_back();
	while (!t.empty() && t.front() == ' ') t.erase(t.begin());
	controlList[0]->enabled = !t.empty();
	if (c == '\r')
		actionPerformed(controlList[0]);
}

void GuiRenameWorld::mouseClicked(int_t x, int_t y, int_t button)
{
	GuiScreen::mouseClicked(x, y, button);
	textbox->mouseClicked(x, y, button);
}

void GuiRenameWorld::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	StringTranslate *tr = StringTranslate::getInstance();
	drawDefaultBackground();
	drawCenteredString(fontRenderer, tr->translateKey("selectWorld.renameTitle"), width / 2, (height / 4 - 60) + 20, 0xffffff);
	drawString(fontRenderer, tr->translateKey("selectWorld.enterName"), width / 2 - 100, 47, 0xa0a0a0);
	textbox->drawTextBox();
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}

GuiTextField *GuiRenameWorld::getFocusedTextField()
{
	return (textbox != nullptr && textbox->isFocused) ? textbox : nullptr;
}
