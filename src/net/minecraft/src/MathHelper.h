#pragma once

#include "java/Type.h"
#include "java/String.h"

// net.minecraft.src.MathHelper
class MathHelper
{
public:
	MathHelper() = default;

	static float sin(float f);
	static float cos(float f);
	static float sqrt_float(float f);
	static float sqrt_double(double d);
	static int_t floor_float(float f);
	static int_t floor_double(double d);
	static float abs(float f);
	static double abs_max(double d, double d1);
	static int_t bucketInt(int_t i, int_t j);
	static bool stringNullOrLengthZero(const jstring &s);

private:
	static float SIN_TABLE[0x10000];
	static bool initSinTable();
	static bool sinTableInitialized;
};
