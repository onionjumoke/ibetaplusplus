#pragma once

#include <array>
#include <vector>
#include "WorldGenerator.h"

// net.minecraft.src.WorldGenBigTree
class WorldGenBigTree : public WorldGenerator
{
public:
	WorldGenBigTree();

	void func_517_a(double d, double d1, double d2) override;
	bool generate(World *world, Random &random, int_t i, int_t j, int_t k) override;

private:
	void func_521_a();
	void func_523_a(int_t i, int_t j, int_t k, float f, byte_t byte0, int_t l);
	float func_528_a(int_t i);
	float func_526_b(int_t i);
	void func_520_a(int_t i, int_t j, int_t k);
	void func_522_a(std::array<int_t, 3> ai, std::array<int_t, 3> ai1, int_t i);
	void func_518_b();
	bool func_527_c(int_t i);
	void func_529_c();
	void func_525_d();
	int_t func_524_a(std::array<int_t, 3> ai, std::array<int_t, 3> ai1);
	bool func_519_e();

	static const byte_t field_882_a[6];
	Random field_881_b;
	World *worldObj;
	std::array<int_t, 3> basePos;
	int_t field_878_e;
	int_t height;
	double field_876_g;
	double field_875_h;
	double field_874_i;
	double field_873_j;
	double field_872_k;
	int_t field_871_l;
	int_t field_870_m;
	int_t field_869_n;
	std::vector<std::array<int_t, 4>> field_868_o;
};
