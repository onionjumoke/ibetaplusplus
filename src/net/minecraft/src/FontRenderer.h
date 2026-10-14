#pragma once

#include <string>
#include <vector>

#include "java/Type.h"

class GameSettings;
class RenderEngine;

// net.minecraft.src.FontRenderer
class FontRenderer
{
public:
	FontRenderer(GameSettings *gamesettings, const std::string &s, RenderEngine *renderengine);

	void drawStringWithShadow(const std::string &s, int_t i, int_t j, int_t k);
	void drawString(const std::string &s, int_t i, int_t j, int_t k);
	void renderString(const std::string &s, int_t i, int_t j, int_t k, bool flag);
	int_t getStringWidth(const std::string &s);
	void func_27278_a(const std::string &s, int_t i, int_t j, int_t k, int_t l);
	int_t func_27277_a(const std::string &s, int_t i);

	void drawSplitString(const std::string &s, int_t i, int_t j, int_t k, int_t l) { func_27278_a(s, i, j, k, l); }
	int_t splitStringWidth(const std::string &s, int_t i) { return func_27277_a(s, i); }

private:
	std::vector<std::string> split(const std::string &s, char delimiter);
	int_t getCharIndex(char c);

	int_t charWidth[256];

public:
	int_t fontTextureName;

private:
	int_t fontDisplayLists;
	std::vector<int_t> buffer;
	// Precomputed colour-code table (RGB, 0..255), indexed by code 0..31
	// (16 normal + 16 shadow-dimmed), matching the old per-code display lists.
	int_t colorCodeR[32];
	int_t colorCodeG[32];
	int_t colorCodeB[32];
};
