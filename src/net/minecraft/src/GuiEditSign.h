#pragma once

#include "GuiScreen.h"
#include <string>

class TileEntitySign;

// net.minecraft.src.GuiEditSign
class GuiEditSign : public GuiScreen
{
public:
	GuiEditSign(TileEntitySign *sign);

	void initGui() override;
	void onGuiClosed() override;
	void updateScreen() override;

protected:
	void actionPerformed(GuiButton *button) override;
	void keyTyped(char_t c, int_t key) override;

public:
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
	std::string screenTitle;

private:
	TileEntitySign *entitySign;
	int_t updateCounter;
	int_t editLine;
};
