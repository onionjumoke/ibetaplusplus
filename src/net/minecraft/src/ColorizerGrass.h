#pragma once

#include <vector>
#include "java/Type.h"

// net.minecraft.src.ColorizerGrass
class ColorizerGrass
{
public:
	static void func_28181_a(const std::vector<int_t> &ai);
	static int_t getGrassColor(double d, double d1);

private:
	static std::vector<int_t> grassBuffer;
};
