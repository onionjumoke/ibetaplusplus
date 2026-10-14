#pragma once

#include "WorldGenerator.h"

// net.minecraft.src.WorldGenTrees
class WorldGenTrees : public WorldGenerator
{
public:
	bool generate(World *world, Random &random, int_t i, int_t j, int_t k) override;
};
