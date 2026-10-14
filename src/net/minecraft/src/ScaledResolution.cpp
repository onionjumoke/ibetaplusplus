#include "ScaledResolution.h"

#include <cmath>
#include "GameSettings.h"

float ScaledResolution::forcedScale = 0.0f;

ScaledResolution::ScaledResolution(GameSettings *gamesettings, int_t i, int_t j)
{
	scaledWidth = i;
	scaledHeight = j;
	scaleFactor = 1;

	int_t k = gamesettings->guiScale;
	if (k == 0)
		k = 1000;
	for (; scaleFactor < k && scaledWidth / (scaleFactor + 1) >= 320 && scaledHeight / (scaleFactor + 1) >= 240; scaleFactor++)
	{
	}
	field_25121_a = (double)scaledWidth / (double)scaleFactor;
	field_25120_b = (double)scaledHeight / (double)scaleFactor;
	scaledWidth = (int_t)std::ceil(field_25121_a);
	scaledHeight = (int_t)std::ceil(field_25120_b);
}

int_t ScaledResolution::getScaledWidth()
{
	return scaledWidth;
}

int_t ScaledResolution::getScaledHeight()
{
	return scaledHeight;
}
