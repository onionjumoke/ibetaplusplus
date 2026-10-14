#include "MapGenBase.h"

#include "World.h"

MapGenBase::MapGenBase() :
	range(8)
{
}

void MapGenBase::generate(IChunkProvider *ichunkprovider, World *world, int_t i, int_t j, byte_t abyte0[])
{
	int_t k = range;
	rand.setSeed(world->getRandomSeed());
	long_t l = (rand.nextLong() / 2LL) * 2LL + 1LL;
	long_t l1 = (rand.nextLong() / 2LL) * 2LL + 1LL;
	for (int_t i1 = i - k; i1 <= i + k; i1++)
	{
		for (int_t j1 = j - k; j1 <= j + k; j1++)
		{
			rand.setSeed((long_t)i1 * l + (long_t)j1 * l1 ^ world->getRandomSeed());
			generateChunk(world, i1, j1, i, j, abyte0);
		}
	}
}

void MapGenBase::generateChunk(World *world, int_t i, int_t j, int_t k, int_t l, byte_t abyte0[])
{
}
