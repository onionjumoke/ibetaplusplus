#pragma once

#include "java/Random.h"
#include "java/Type.h"

class World;

// net.minecraft.src.WorldGenerator
class WorldGenerator
{
public:
	WorldGenerator() = default;
	virtual ~WorldGenerator() = default;

	virtual bool generate(World *world, Random &random, int_t i, int_t j, int_t k) = 0;

	virtual void func_517_a(double d, double d1, double d2) {}
	virtual void getEntityString(double d, double d1, double d2) { func_517_a(d, d1, d2); }
};
