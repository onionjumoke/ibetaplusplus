#pragma once

#include "GuiScreen.h"
#include "EnumOptions.h"
#include <string>

class GuiTextField;

class GameSettings;

// net.minecraft.src.GuiOptions
class GuiOptions : public GuiScreen
{
public:
	GuiOptions(GuiScreen *parent, GameSettings *settings);

	void initGui() override;

protected:
	void actionPerformed(GuiButton *button) override;

public:
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

private:
	GuiScreen *parentScreen;
	GameSettings *options;

protected:
	std::string screenTitle;

	void keyTyped(char_t c, int_t key) override;
	void mouseClicked(int_t x, int_t y, int_t button) override;
	void updateScreen() override;
	void onGuiClosed() override;
	bool keyPressed(char_t c, int_t key); // shared commit path

public:
	GuiTextField *textboxUsername = nullptr;
	bool usernameWasFocused = false;


public:
	GuiTextField *getFocusedTextField() override;
};
