#pragma once

#include "BiomeGenBase.h"

class WorldGenBigTree;
class WorldGenTrees;

// net.minecraft.src.BiomeGenRainforest
class BiomeGenRainforest : public BiomeGenBase
{
public:
	BiomeGenRainforest() = default;
	WorldGenerator *getRandomWorldGenForTrees(Random &random) override;
};
