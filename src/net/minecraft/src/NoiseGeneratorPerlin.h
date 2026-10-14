#pragma once

#include <vector>

#include <array>
#include "NoiseGenerator.h"
#include "java/Random.h"
#include "java/Type.h"

// net.minecraft.src.NoiseGeneratorPerlin
class NoiseGeneratorPerlin : public NoiseGenerator
{
public:
	NoiseGeneratorPerlin();
	NoiseGeneratorPerlin(Random &random);

	double generateNoise(double d, double d1, double d2);
	double lerp(double d, double d1, double d2);
	double getBlockMetadata(int_t i, double d, double d1);
	double grad(int_t i, double d, double d1, double d2);
	double getDropItemId(double d, double d1);
	void getCanSpawnHere(std::vector<double> &ad, double d, double d1, double d2,
	                int_t i, int_t j, int_t k,
	                double d3, double d4, double d5, double d6);

	double xCoord;
	double yCoord;
	double zCoord;

private:
	std::array<int_t, 512> permutations;
};
