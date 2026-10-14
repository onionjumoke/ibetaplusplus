#pragma once

#include <array>
#include <vector>
#include "java/Random.h"
#include "java/Type.h"

// net.minecraft.src.NoiseGenerator2
class NoiseGenerator2
{
public:
	NoiseGenerator2();
	NoiseGenerator2(Random &random);

	void generatePerlinNoise(std::vector<double> &ad, double d, double d1, int_t i, int_t j,
	                  double d2, double d3, double d4);

	double seed;
	double permutations;
	double lacunarity;

private:
	static int_t wrap(double d);
	static double generateNoiseOctaves(const int_t ai[2], double d, double d1);

	static const int_t amplitude[12][3];
	static const double persistence;
	static const double octaves;

	std::array<int_t, 512> frequency;
};
