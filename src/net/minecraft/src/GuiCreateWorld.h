#pragma once

#include "GuiScreen.h"
#include <string>

class GuiTextField;
class ISaveFormat;

// net.minecraft.src.GuiCreateWorld
class GuiCreateWorld : public GuiScreen
{
public:
	GuiCreateWorld(GuiScreen *parent);

	void updateScreen() override;
	void initGui() override;
	void onGuiClosed() override;

	static std::string generateUnusedFolderName(ISaveFormat *fmt, const std::string &base);

protected:
	void actionPerformed(GuiButton *button) override;
	void keyTyped(char_t c, int_t key) override;
	void mouseClicked(int_t x, int_t y, int_t button) override;

public:
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;
	void selectNextField() override;

private:
	void updateFolderName();

	GuiScreen *parentScreen;
	GuiTextField *textboxWorldName;
	GuiTextField *textboxSeed;
	std::string folderName;
	bool createClicked;
	int_t worldType = 0; // 0 default, 1 superflat

public:
	GuiTextField *getFocusedTextField() override;
};
