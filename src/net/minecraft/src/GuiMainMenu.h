#pragma once

#include "GuiScreen.h"
#include <string>

class GuiButton;

// net.minecraft.src.GuiMainMenu
class GuiMainMenu : public GuiScreen
{
public:
	GuiMainMenu();

	void updateScreen() override;

protected:
	void keyTyped(char_t c, int_t key) override;

public:
	void initGui() override;

protected:
	void actionPerformed(GuiButton *button) override;

public:
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

private:
	float_t updateCounter;
	std::string splashText;
	GuiButton *multiplayerButton;
};
