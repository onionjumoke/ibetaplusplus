#include "ColorizerFoliage.h"

std::vector<int_t> ColorizerFoliage::foliageBuffer(0x10000);

void ColorizerFoliage::func_28152_a(const std::vector<int_t> &ai)
{
	foliageBuffer = ai;
}

int_t ColorizerFoliage::getFoliageColor(double d, double d1)
{
	d1 *= d;
	int_t i = (int_t)((1.0 - d) * 255.0);
	int_t j = (int_t)((1.0 - d1) * 255.0);
	return foliageBuffer[j << 8 | i];
}

int_t ColorizerFoliage::getFoliageColorPine()
{
	return 0x619961;
}

int_t ColorizerFoliage::getFoliageColorBirch()
{
	return 0x80a755;
}

int_t ColorizerFoliage::func_31073_c()
{
	return 0x48b518;
}

int_t ColorizerFoliage::getSpawnDelay()
{
	return func_31073_c();
}
