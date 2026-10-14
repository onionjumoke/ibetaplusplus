#pragma once

#include <vector>
#include "NoiseGenerator.h"
#include "java/Random.h"
#include "java/Type.h"

class NoiseGeneratorPerlin;

// net.minecraft.src.NoiseGeneratorOctaves
class NoiseGeneratorOctaves : public NoiseGenerator
{
public:
	NoiseGeneratorOctaves(Random &random, int_t i);

	double getExperiencePoints(double d, double d1);

	// ad is filled in-place and returned by reference to avoid copying the
	// (potentially large) noise buffer on every call. Callers keep the
	// `field = gen.generateNoiseOctaves(field, ...)` idiom (self-assignment).
	std::vector<double> &generateNoiseOctaves(std::vector<double> &ad, double d, double d1, double d2,
	                                          int_t i, int_t j, int_t k,
	                                          double d3, double d4, double d5);

	std::vector<double> &getBiomeGenForCoords(std::vector<double> &ad, int_t i, int_t j, int_t k, int_t l,
	                                 double d, double d1, double d2);

private:
	std::vector<NoiseGeneratorPerlin *> generatorCollection;
	int_t biomeName;
};
