#pragma once

#include <vector>
#include "java/Type.h"

// net.minecraft.src.ColorizerFoliage
class ColorizerFoliage
{
public:
	static void func_28152_a(const std::vector<int_t> &ai);
	static int_t getFoliageColor(double d, double d1);
	static int_t getFoliageColorPine();
	static int_t getFoliageColorBirch();
	static int_t func_31073_c();
	static int_t getSpawnDelay();

private:
	static std::vector<int_t> foliageBuffer;
};
