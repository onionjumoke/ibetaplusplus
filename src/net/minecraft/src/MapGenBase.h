#pragma once

#include "java/Type.h"
#include "java/Random.h"

class World;
class IChunkProvider;

// net.minecraft.src.MapGenBase
class MapGenBase
{
public:
	MapGenBase();

	virtual void generate(IChunkProvider *ichunkprovider, World *world, int_t i, int_t j, byte_t abyte0[]);

protected:
	virtual void generateChunk(World *world, int_t i, int_t j, int_t k, int_t l, byte_t abyte0[]);

	int_t range; // range
	Random rand;
};
