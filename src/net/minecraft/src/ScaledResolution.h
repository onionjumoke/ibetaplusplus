#pragma once

#include "java/Type.h"

class GameSettings;

// net.minecraft.src.ScaledResolution
class ScaledResolution
{
public:
	ScaledResolution(GameSettings *gamesettings, int_t i, int_t j);

	// iOS override: a fractional GUI scale (e.g. 1.5 on phones, 2 on tablets)
	// so touch GUIs are finger-sized. 0 disables the override.
	static float forcedScale;

	int_t getScaledWidth();
	int_t getScaledHeight();

private:
	int_t scaledWidth;
	int_t scaledHeight;

public:
	double field_25121_a;
	double field_25120_b;
	int_t scaleFactor;
};
