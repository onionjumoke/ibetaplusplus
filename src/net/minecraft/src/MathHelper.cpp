#include "MathHelper.h"

#include <cmath>

float MathHelper::SIN_TABLE[0x10000];

bool MathHelper::initSinTable()
{
	for (int_t i = 0; i < 0x10000; i++)
		SIN_TABLE[i] = (float)std::sin(((double)i * 3.1415926535897931 * 2.0) / 65536.0);
	return true;
}

bool MathHelper::sinTableInitialized = MathHelper::initSinTable();

float MathHelper::sin(float f)
{
	return SIN_TABLE[(int_t)(f * 10430.38f) & 0xffff];
}

float MathHelper::cos(float f)
{
	return SIN_TABLE[(int_t)(f * 10430.38f + 16384.0f) & 0xffff];
}

float MathHelper::sqrt_float(float f)
{
	return (float)std::sqrt(f);
}

float MathHelper::sqrt_double(double d)
{
	return (float)std::sqrt(d);
}

int_t MathHelper::floor_float(float f)
{
	int_t i = (int_t)f;
	return f >= (float)i ? i : i - 1;
}

int_t MathHelper::floor_double(double d)
{
	int_t i = (int_t)d;
	return d >= (double)i ? i : i - 1;
}

float MathHelper::abs(float f)
{
	return f < 0.0f ? -f : f;
}

double MathHelper::abs_max(double d, double d1)
{
	if (d < 0.0)
		d = -d;
	if (d1 < 0.0)
		d1 = -d1;
	return d <= d1 ? d1 : d;
}

int_t MathHelper::bucketInt(int_t i, int_t j)
{
	if (i < 0)
		return -((-i - 1) / j) - 1;
	else
		return i / j;
}

bool MathHelper::stringNullOrLengthZero(const jstring &s)
{
	return s.length() == 0;
}
