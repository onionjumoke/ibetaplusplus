#include "GuiOptions.h"
#include "GuiTextField.h"
#ifndef MC_IOS
#include "pc/stub/IosBridge_stub.h"
#else
#include "pc/ios/IosBridge.h"
#endif
#include <string>
#include <filesystem>
#include <cstdio>
#include "GuiSmallButton.h"
#include "GuiSlider.h"
#include "GuiButton.h"
#include "GuiVideoSettings.h"
#include "GuiControls.h"
#include "GameSettings.h"
#include "StringTranslate.h"
#include "Minecraft.h"
#include "EntityPlayerSP.h"
#include "Session.h"
#include "net/minecraft/src/GameResources.h"

GuiOptions::GuiOptions(GuiScreen *parent, GameSettings *settings)
	: screenTitle("Options")
	, parentScreen(parent)
	, options(settings)
{
}

void GuiOptions::initGui()
{
	// Avoid cross-translation-unit static initialization order dependencies.
	EnumOptions *shownOptions[] = {
		EnumOptions::MUSIC,
		EnumOptions::SOUND,
		EnumOptions::INVERT_MOUSE,
		EnumOptions::SENSITIVITY,
		EnumOptions::DIFFICULTY
	};

	StringTranslate *tr = StringTranslate::getInstance();
	screenTitle = tr->translateKey("options.title");
	int_t i = 0;
	for (EnumOptions *opt : shownOptions)
	{
		int_t bx = (width / 2 - 155) + (i % 2) * 160;
		int_t by = height / 6 + 24 * (i >> 1);
		if (!opt->getEnumFloat())
			controlList.push_back(new GuiSmallButton(opt->returnEnumOrdinal(), bx, by, opt, options->getKeyBinding(opt)));
		else
			controlList.push_back(new GuiSlider(opt->returnEnumOrdinal(), bx, by, opt, options->getKeyBinding(opt), options->getOptionFloatValue(opt)));
		i++;
	}
	textboxUsername = new GuiTextField(this, fontRenderer, width / 2 - 100,
	                                   height / 6 + 96 + 12, 156, 20, "");
	textboxUsername->setMaxStringLength(16);
	textboxUsername->setText((mc != nullptr && mc->session != nullptr && !mc->session->username.empty())
	                         ? mc->session->username
	                         : iosapp::currentUsername());
	textboxUsername->isFocused = false;
	usernameWasFocused = false;
	controlList.push_back(new GuiButton(103, width / 2 + 62, height / 6 + 96 + 12, 38, 20, "Save"));
	// Video/Controls side by side and Done above the bottom edge: the old
	// stacked layout put Done at h/6+192 (y=232 on a 240-tall 4S), leaving only
	// a few pixels on screen.
	controlList.push_back(new GuiButton(101, width / 2 - 100, height / 6 + 132, 98, 20, tr->translateKey("options.video")));
	controlList.push_back(new GuiButton(100, width / 2 + 2,   height / 6 + 132, 98, 20, tr->translateKey("options.controls")));
	controlList.push_back(new GuiButton(200, width / 2 - 100, height / 6 + 164, 200, 20, tr->translateKey("gui.done")));
}

void GuiOptions::actionPerformed(GuiButton *button)
{
	if (!button->enabled) return;

	if (button->id < 100 && dynamic_cast<GuiSmallButton *>(button))
	{
		options->setOptionValue(static_cast<GuiSmallButton *>(button)->returnEnumOptions(), 1);
		button->displayString = options->getKeyBinding(EnumOptions::getEnumOptions(button->id));
	}
	if (button->id == 103 && textboxUsername != nullptr)
	{
		keyPressed('\0', 0); // commit path shared with RETURN
	}
	if (button->id == 101)
	{
		mc->gameSettings->saveOptions();
		mc->displayGuiScreen(new GuiVideoSettings(this, options));
	}
	if (button->id == 100)
	{
		mc->gameSettings->saveOptions();
		mc->displayGuiScreen(new GuiControls(this, options));
	}
	if (button->id == 200)
	{
		mc->gameSettings->saveOptions();
		mc->displayGuiScreen(parentScreen);
	}
}

void GuiOptions::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawDefaultBackground();
	drawString(fontRenderer, "Username:", width / 2 - 100, height / 6 + 96 + 4, 0xa0a0a0);
	if (textboxUsername != nullptr)
		textboxUsername->drawTextBox();
	drawCenteredString(fontRenderer, screenTitle, width / 2, 20, 0xffffff);
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}

// ─── Username editing (touch) ────────────────────────────────────────────────

void GuiOptions::mouseClicked(int_t x, int_t y, int_t button)
{
	GuiScreen::mouseClicked(x, y, button);
	if (textboxUsername != nullptr)
	{
		textboxUsername->mouseClicked(x, y, button);
		// The generic iOS path (Minecraft::runTick) detects the focused field and
		// opens the edit popup prefilled + live-synced, so nothing to do here.
		usernameWasFocused = textboxUsername->isFocused;
		(void)0;
	}
}

bool GuiOptions::keyPressed(char_t c, int_t key)
{
	(void)c;
	(void)key;
	if (textboxUsername == nullptr)
		return false;
	std::string name = textboxUsername->getText();
	// trim
	size_t b = name.find_first_not_of(" \t\r\n");
	size_t e = name.find_last_not_of(" \t\r\n");
	name = (b == std::string::npos) ? "" : name.substr(b, e - b + 1);
	if (!name.empty())
	{
		if (mc->session != nullptr)
			mc->session->username = name;
		if (mc->thePlayer != nullptr)
			mc->thePlayer->username = String::fromUTF8(name);
		FILE *uf = fopen(
		    (std::filesystem::path(GameResources::getExeDir()) / "username.txt")
		        .string()
		        .c_str(),
		    "w");
		if (uf != nullptr)
		{
			fputs(name.c_str(), uf);
			fclose(uf);
		}
		(void)0;
	}
	return true;
}

void GuiOptions::keyTyped(char_t c, int_t key)
{
	if (textboxUsername != nullptr && textboxUsername->isFocused)
	{
		textboxUsername->textboxKeyTyped(c, key);
		if (key == 0x1C) // RETURN from the keyboard alert's Send
		{
			keyPressed(c, key);
			// stay focused: further KB taps keep editing this field
			usernameWasFocused = true;
		}
		return;
	}
	GuiScreen::keyTyped(c, key);
}

void GuiOptions::updateScreen()
{
	if (textboxUsername != nullptr)
		textboxUsername->updateCursorCounter();
}

void GuiOptions::onGuiClosed()
{
	// Always persist the username when leaving Options (live editing no longer
	// ends with a RETURN key).
	keyPressed('\0', 0);
	GuiScreen::onGuiClosed();
}

GuiTextField *GuiOptions::getFocusedTextField()
{
	return (textboxUsername != nullptr && textboxUsername->isFocused) ? textboxUsername : nullptr;
}
