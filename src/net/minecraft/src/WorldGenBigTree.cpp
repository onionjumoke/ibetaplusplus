#include "WorldGenBigTree.h"

#include <cmath>
#include <cstdlib>
#include "MathHelper.h"
#include "World.h"

const byte_t WorldGenBigTree::field_882_a[6] = { 2, 0, 0, 1, 2, 1 };

WorldGenBigTree::WorldGenBigTree()
{
	worldObj = nullptr;
	basePos = { 0, 0, 0 };
	field_878_e = 0;
	height = 0;
	field_876_g = 0.61799999999999999;
	field_875_h = 1.0;
	field_874_i = 0.38100000000000001;
	field_873_j = 1.0;
	field_872_k = 1.0;
	field_871_l = 1;
	field_870_m = 12;
	field_869_n = 4;
}

void WorldGenBigTree::func_521_a()
{
	height = (int_t)((double)field_878_e * field_876_g);
	if (height >= field_878_e)
		height = field_878_e - 1;
	int_t i = (int_t)(1.3819999999999999 + std::pow((field_872_k * (double)field_878_e) / 13.0, 2.0));
	if (i < 1)
		i = 1;
	std::vector<std::array<int_t, 4>> ai(i * field_878_e);
	int_t j = (basePos[1] + field_878_e) - field_869_n;
	int_t k = 1;
	int_t l = basePos[1] + height;
	int_t i1 = j - basePos[1];
	ai[0] = { basePos[0], j, basePos[2], l };
	j--;
	while (i1 >= 0)
	{
		int_t j1 = 0;
		float f = func_528_a(i1);
		if (f < 0.0f)
		{
			j--;
			i1--;
		}
		else
		{
			double d = 0.5;
			for (; j1 < i; j1++)
			{
				double d1 = field_873_j * ((double)f * ((double)field_881_b.nextFloat() + 0.32800000000000001));
				double d2 = (double)field_881_b.nextFloat() * 2.0 * 3.1415899999999999;
				int_t k1 = MathHelper::floor_double(d1 * std::sin(d2) + (double)basePos[0] + d);
				int_t l1 = MathHelper::floor_double(d1 * std::cos(d2) + (double)basePos[2] + d);
				std::array<int_t, 3> ai1 = { k1, j, l1 };
				std::array<int_t, 3> ai2 = { k1, j + field_869_n, l1 };
				if (func_524_a(ai1, ai2) != -1)
					continue;
				std::array<int_t, 3> ai3 = { basePos[0], basePos[1], basePos[2] };
				double d3 = std::sqrt(std::pow(std::abs(basePos[0] - ai1[0]), 2.0) + std::pow(std::abs(basePos[2] - ai1[2]), 2.0));
				double d4 = d3 * field_874_i;
				if ((double)ai1[1] - d4 > (double)l)
					ai3[1] = l;
				else
					ai3[1] = (int_t)((double)ai1[1] - d4);
				if (func_524_a(ai3, ai1) == -1)
					ai[k++] = { k1, j, l1, ai3[1] };
			}
			j--;
			i1--;
		}
	}
	field_868_o.assign(ai.begin(), ai.begin() + k);
}

void WorldGenBigTree::func_523_a(int_t i, int_t j, int_t k, float f, byte_t byte0, int_t l)
{
	int_t i1 = (int_t)((double)f + 0.61799999999999999);
	byte_t byte1 = field_882_a[byte0];
	byte_t byte2 = field_882_a[byte0 + 3];
	std::array<int_t, 3> ai = { i, j, k };
	std::array<int_t, 3> ai1 = { 0, 0, 0 };
	int_t j1 = -i1;
	int_t k1 = -i1;
	ai1[byte0] = ai[byte0];
	for (; j1 <= i1; j1++)
	{
		ai1[byte1] = ai[byte1] + j1;
		for (int_t l1 = -i1; l1 <= i1;)
		{
			double d = std::sqrt(std::pow((double)std::abs(j1) + 0.5, 2.0) + std::pow((double)std::abs(l1) + 0.5, 2.0));
			if (d > (double)f)
			{
				l1++;
			}
			else
			{
				ai1[byte2] = ai[byte2] + l1;
				int_t i2 = worldObj->getBlockId(ai1[0], ai1[1], ai1[2]);
				if (i2 != 0 && i2 != 18)
				{
					l1++;
				}
				else
				{
					worldObj->setBlock(ai1[0], ai1[1], ai1[2], l);
					l1++;
				}
			}
		}
	}
}

float WorldGenBigTree::func_528_a(int_t i)
{
	if ((double)i < (double)(float)field_878_e * 0.29999999999999999)
		return -1.618f;
	float f = (float)field_878_e / 2.0f;
	float f1 = (float)field_878_e / 2.0f - (float)i;
	float f2;
	if (f1 == 0.0f)
		f2 = f;
	else if (std::abs(f1) >= f)
		f2 = 0.0f;
	else
		f2 = (float)std::sqrt(std::pow(std::abs(f), 2.0) - std::pow(std::abs(f1), 2.0));
	f2 *= 0.5f;
	return f2;
}

float WorldGenBigTree::func_526_b(int_t i)
{
	if (i < 0 || i >= field_869_n)
		return -1.0f;
	return i != 0 && i != field_869_n - 1 ? 3.0f : 2.0f;
}

void WorldGenBigTree::func_520_a(int_t i, int_t j, int_t k)
{
	int_t l = j;
	for (int_t i1 = j + field_869_n; l < i1; l++)
	{
		float f = func_526_b(l - j);
		func_523_a(i, l, k, f, (byte_t)1, 18);
	}
}

void WorldGenBigTree::func_522_a(std::array<int_t, 3> ai, std::array<int_t, 3> ai1, int_t i)
{
	std::array<int_t, 3> ai2 = { 0, 0, 0 };
	byte_t byte0 = 0;
	int_t j = 0;
	for (; byte0 < 3; byte0++)
	{
		ai2[byte0] = ai1[byte0] - ai[byte0];
		if (std::abs(ai2[byte0]) > std::abs(ai2[j]))
			j = byte0;
	}
	if (ai2[j] == 0)
		return;
	byte_t byte1 = field_882_a[j];
	byte_t byte2 = field_882_a[j + 3];
	byte_t byte3 = ai2[j] > 0 ? 1 : -1;
	double d = (double)ai2[byte1] / (double)ai2[j];
	double d1 = (double)ai2[byte2] / (double)ai2[j];
	std::array<int_t, 3> ai3 = { 0, 0, 0 };
	int_t k = 0;
	for (int_t l = ai2[j] + byte3; k != l; k += byte3)
	{
		ai3[j] = MathHelper::floor_double((double)(ai[j] + k) + 0.5);
		ai3[byte1] = MathHelper::floor_double((double)ai[byte1] + (double)k * d + 0.5);
		ai3[byte2] = MathHelper::floor_double((double)ai[byte2] + (double)k * d1 + 0.5);
		worldObj->setBlock(ai3[0], ai3[1], ai3[2], i);
	}
}

void WorldGenBigTree::func_518_b()
{
	for (int_t i = 0, j = (int_t)field_868_o.size(); i < j; i++)
		func_520_a(field_868_o[i][0], field_868_o[i][1], field_868_o[i][2]);
}

bool WorldGenBigTree::func_527_c(int_t i)
{
	return (double)i >= (double)field_878_e * 0.20000000000000001;
}

void WorldGenBigTree::func_529_c()
{
	int_t i = basePos[0];
	int_t j = basePos[1];
	int_t k = basePos[1] + height;
	int_t l = basePos[2];
	std::array<int_t, 3> ai = { i, j, l };
	std::array<int_t, 3> ai1 = { i, k, l };
	func_522_a(ai, ai1, 17);
	if (field_871_l == 2)
	{
		ai[0]++;
		ai1[0]++;
		func_522_a(ai, ai1, 17);
		ai[2]++;
		ai1[2]++;
		func_522_a(ai, ai1, 17);
		ai[0]--;
		ai1[0]--;
		func_522_a(ai, ai1, 17);
	}
}

void WorldGenBigTree::func_525_d()
{
	std::array<int_t, 3> ai = { basePos[0], basePos[1], basePos[2] };
	for (int_t i = 0, j = (int_t)field_868_o.size(); i < j; i++)
	{
		std::array<int_t, 4> ai1 = field_868_o[i];
		std::array<int_t, 3> ai2 = { ai1[0], ai1[1], ai1[2] };
		ai[1] = ai1[3];
		int_t k = ai[1] - basePos[1];
		if (func_527_c(k))
			func_522_a(ai, ai2, 17);
	}
}

int_t WorldGenBigTree::func_524_a(std::array<int_t, 3> ai, std::array<int_t, 3> ai1)
{
	std::array<int_t, 3> ai2 = { 0, 0, 0 };
	byte_t byte0 = 0;
	int_t i = 0;
	for (; byte0 < 3; byte0++)
	{
		ai2[byte0] = ai1[byte0] - ai[byte0];
		if (std::abs(ai2[byte0]) > std::abs(ai2[i]))
			i = byte0;
	}
	if (ai2[i] == 0)
		return -1;
	byte_t byte1 = field_882_a[i];
	byte_t byte2 = field_882_a[i + 3];
	byte_t byte3 = ai2[i] > 0 ? 1 : -1;
	double d = (double)ai2[byte1] / (double)ai2[i];
	double d1 = (double)ai2[byte2] / (double)ai2[i];
	std::array<int_t, 3> ai3 = { 0, 0, 0 };
	int_t j = 0;
	int_t k = ai2[i] + byte3;
	do
	{
		if (j == k)
			break;
		ai3[i] = ai[i] + j;
		ai3[byte1] = MathHelper::floor_double((double)ai[byte1] + (double)j * d);
		ai3[byte2] = MathHelper::floor_double((double)ai[byte2] + (double)j * d1);
		int_t l = worldObj->getBlockId(ai3[0], ai3[1], ai3[2]);
		if (l != 0 && l != 18)
			break;
		j += byte3;
	} while (true);
	if (j == k)
		return -1;
	return std::abs(j);
}

bool WorldGenBigTree::func_519_e()
{
	std::array<int_t, 3> ai = { basePos[0], basePos[1], basePos[2] };
	std::array<int_t, 3> ai1 = { basePos[0], (basePos[1] + field_878_e) - 1, basePos[2] };
	int_t i = worldObj->getBlockId(basePos[0], basePos[1] - 1, basePos[2]);
	if (i != 2 && i != 3)
		return false;
	int_t j = func_524_a(ai, ai1);
	if (j == -1)
		return true;
	if (j < 6)
		return false;
	field_878_e = j;
	return true;
}

void WorldGenBigTree::func_517_a(double d, double d1, double d2)
{
	field_870_m = (int_t)(d * 12.0);
	if (d > 0.5)
		field_869_n = 5;
	field_873_j = d1;
	field_872_k = d2;
}

bool WorldGenBigTree::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	worldObj = world;
	long_t l = random.nextLong();
	field_881_b.setSeed(l);
	basePos[0] = i;
	basePos[1] = j;
	basePos[2] = k;
	if (field_878_e == 0)
		field_878_e = 5 + field_881_b.nextInt(field_870_m);
	if (!func_519_e())
		return false;
	func_521_a();
	func_518_b();
	func_529_c();
	func_525_d();
	return true;
}
