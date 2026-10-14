#pragma once

#include <vector>
#include "NoiseGenerator.h"
#include "java/Random.h"
#include "java/Type.h"

class NoiseGenerator2;

// net.minecraft.src.NoiseGeneratorOctaves2
class NoiseGeneratorOctaves2 : public NoiseGenerator
{
public:
	NoiseGeneratorOctaves2(Random &random, int_t i);

	// ad filled in-place and returned by reference (avoids per-call copy).
	std::vector<double> &getBlockMaterial(std::vector<double> &ad, double d, double d1, int_t i, int_t j,
	                                 double d2, double d3, double d4);

	std::vector<double> &setBlockMetadata(std::vector<double> &ad, double d, double d1, int_t i, int_t j,
	                                 double d2, double d3, double d4, double d5);

private:
	std::vector<NoiseGenerator2 *> needleTexture;
	int_t dialTexture;
};
