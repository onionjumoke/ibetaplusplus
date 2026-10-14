#pragma once

#include "GuiScreen.h"
#include <string>

// net.minecraft.src.GuiChat
class GuiChat : public GuiScreen
{
public:
	GuiChat();

	void initGui() override;
	void onGuiClosed() override;
	void updateScreen() override;

protected:
	void keyTyped(char_t c, int_t key) override;

public:
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

public:
	const std::string &getChatMessage() const { return message; }

protected:
	void mouseClicked(int_t x, int_t y, int_t button) override;

protected:
	std::string message;

private:
	int_t updateCounter;
};
