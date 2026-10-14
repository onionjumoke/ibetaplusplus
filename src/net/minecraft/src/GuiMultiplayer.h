#pragma once

#include "GuiScreen.h"
#include <string>

class GuiTextField;

// net.minecraft.src.GuiMultiplayer
class GuiMultiplayer : public GuiScreen
{
public:
	GuiMultiplayer(GuiScreen *parent);

	void updateScreen() override;
	void initGui() override;
	void onGuiClosed() override;

protected:
	void actionPerformed(GuiButton *button) override;
	void keyTyped(char_t c, int_t key) override;
	void mouseClicked(int_t x, int_t y, int_t button) override;

public:
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

private:
	int_t parseIntWithDefault(const std::string &s, int_t def);

	GuiScreen *parentScreen;
	GuiTextField *serverAddressField;

public:
	GuiTextField *getFocusedTextField() override;
};
