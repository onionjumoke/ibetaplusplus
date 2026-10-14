#include "GuiYesNo.h"
#include "GuiSmallButton.h"

GuiYesNo::GuiYesNo(GuiScreen *parent, const std::string &msg1, const std::string &msg2,
                   const std::string &yes, const std::string &no, int_t worldNum)
	: parentScreen(parent)
	, message1(msg1)
	, message2(msg2)
	, yesText(yes)
	, noText(no)
	, worldNumber(worldNum)
{
}

void GuiYesNo::initGui()
{
	controlList.push_back(new GuiSmallButton(0, (width / 2 - 155) + 0,   height / 6 + 96, yesText));
	controlList.push_back(new GuiSmallButton(1, (width / 2 - 155) + 160, height / 6 + 96, noText));
}

void GuiYesNo::actionPerformed(GuiButton *button)
{
	parentScreen->deleteWorld(button->id == 0, worldNumber);
}

void GuiYesNo::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawDefaultBackground();
	drawCenteredString(fontRenderer, message1, width / 2, 70, 0xffffff);
	drawCenteredString(fontRenderer, message2, width / 2, 90, 0xffffff);
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
