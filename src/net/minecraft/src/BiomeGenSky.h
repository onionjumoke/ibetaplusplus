#pragma once

#include "BiomeGenBase.h"

class EntityChicken;

// net.minecraft.src.BiomeGenSky
class BiomeGenSky : public BiomeGenBase
{
public:
	BiomeGenSky();
	int_t getSkyColorByTemp(float f) override;
};
