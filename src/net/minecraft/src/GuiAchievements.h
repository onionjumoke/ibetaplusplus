#pragma once

#include "GuiScreen.h"

class StatFileWriter;

// net.minecraft.src.GuiAchievements
class GuiAchievements : public GuiScreen
{
public:
	GuiAchievements(StatFileWriter *statfilewriter);

	void initGui() override;
	void updateScreen() override;
	bool doesGuiPauseGame() override;
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
	void actionPerformed(GuiButton *button) override;
	void keyTyped(char_t c, int_t key) override;

private:
	void drawTitle();
	void drawAchievementsPanel(int_t mouseX, int_t mouseY, float_t partialTick);

	static const int_t minScrollX;
	static const int_t minScrollY;
	static const int_t maxScrollX;
	static const int_t maxScrollY;

protected:
	int_t bgWidth;
	int_t bgHeight;
	int_t lastMouseX;
	int_t lastMouseY;
	double lastScrollX;
	double lastScrollY;
	double scrollX;
	double scrollY;
	double targetScrollX;
	double targetScrollY;

private:
	int_t mouseState;
	StatFileWriter *statFileWriter;
};
