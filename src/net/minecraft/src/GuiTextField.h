#pragma once

#include "Gui.h"
#include <string>

class GuiScreen;
class FontRenderer;

// net.minecraft.src.GuiTextField
class GuiTextField : public Gui
{
public:
	GuiTextField(GuiScreen *parent, FontRenderer *fontrenderer,
	             int_t x, int_t y, int_t w, int_t h, const std::string &initialText);

	void setText(const std::string &s);
	const std::string &getText() const;
	void updateCursorCounter();
	void textboxKeyTyped(char_t c, int_t key);
	void mouseClicked(int_t x, int_t y, int_t button);
	void setFocused(bool focused);
	void drawTextBox();
	void setMaxStringLength(int_t maxLen);

public:
	bool isFocused;
	bool isEnabled;

private:
	FontRenderer *fontRenderer;
	const int_t xPos;
	const int_t yPos;
	const int_t width;
	const int_t height;
	std::string text;
	int_t maxStringLength;
	int_t cursorCounter;
	GuiScreen *parentGuiScreen;
};
