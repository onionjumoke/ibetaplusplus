#include "GuiTextField.h"
#include "GuiScreen.h"
#include "FontRenderer.h"
#include "ChatAllowedCharacters.h"

GuiTextField::GuiTextField(GuiScreen *parent, FontRenderer *fontrenderer,
                           int_t x, int_t y, int_t w, int_t h, const std::string &initialText)
	: isFocused(false)
	, isEnabled(true)
	, fontRenderer(fontrenderer)
	, xPos(x)
	, yPos(y)
	, width(w)
	, height(h)
	, maxStringLength(0)
	, cursorCounter(0)
	, parentGuiScreen(parent)
{
	setText(initialText);
}

void GuiTextField::setText(const std::string &s)
{
	text = s;
}

const std::string &GuiTextField::getText() const
{
	return text;
}

void GuiTextField::updateCursorCounter()
{
	cursorCounter++;
}

void GuiTextField::textboxKeyTyped(char_t c, int_t key)
{
	if (!isEnabled || !isFocused) return;

	if (c == '\t')
	{
		parentGuiScreen->selectNextField();
	}

	// Ctrl+V (char 0x16)
	if (c == '\026')
	{
		std::string clipboard = GuiScreen::getClipboardString();
		int_t space = 32 - (int_t)text.length();
		if (space > (int_t)clipboard.length())
			space = (int_t)clipboard.length();
		if (space > 0)
			text += clipboard.substr(0, space);
	}

	// Backspace
	if (key == 14 && !text.empty())
	{
		text = text.substr(0, text.length() - 1);
	}

	// Printable character
	if (ChatAllowedCharacters::allowedCharacters.find(c) != std::string::npos
	    && (maxStringLength == 0 || (int_t)text.length() < maxStringLength))
	{
		text += c;
	}
}

void GuiTextField::mouseClicked(int_t x, int_t y, int_t button)
{
	bool focused = isEnabled && x >= xPos && x < xPos + width && y >= yPos && y < yPos + height;
	setFocused(focused);
}

void GuiTextField::setFocused(bool focused)
{
	if (focused && !isFocused)
		cursorCounter = 0;
	isFocused = focused;
}

void GuiTextField::drawTextBox()
{
	drawRect(xPos - 1, yPos - 1, xPos + width + 1, yPos + height + 1, 0xffa0a0a0);
	drawRect(xPos, yPos, xPos + width, yPos + height, 0xff000000);
	if (isEnabled)
	{
		bool blink = isFocused && (cursorCounter / 6) % 2 == 0;
		std::string display = text + (blink ? "_" : "");
		drawString(fontRenderer, display, xPos + 4, yPos + (height - 8) / 2, 0xe0e0e0);
	}
	else
	{
		drawString(fontRenderer, text, xPos + 4, yPos + (height - 8) / 2, 0x707070);
	}
}

void GuiTextField::setMaxStringLength(int_t maxLen)
{
	maxStringLength = maxLen;
}
