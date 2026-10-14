#include "ColorizerGrass.h"

std::vector<int_t> ColorizerGrass::grassBuffer(0x10000);

void ColorizerGrass::func_28181_a(const std::vector<int_t> &ai)
{
	grassBuffer = ai;
}

int_t ColorizerGrass::getGrassColor(double d, double d1)
{
	d1 *= d;
	int_t i = (int_t)((1.0 - d) * 255.0);
	int_t j = (int_t)((1.0 - d1) * 255.0);
	return grassBuffer[j << 8 | i];
}
