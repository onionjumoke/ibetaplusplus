#include "WorldGenReed.h"

#include "Block.h"
#include "Material.h"
#include "World.h"

bool WorldGenReed::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	for (int_t l = 0; l < 20; l++)
	{
		int_t i1 = (i + random.nextInt(4)) - random.nextInt(4);
		int_t j1 = j;
		int_t k1 = (k + random.nextInt(4)) - random.nextInt(4);
		if (!world->isAirBlock(i1, j1, k1) || world->getBlockMaterial(i1 - 1, j1 - 1, k1) != Material::water && world->getBlockMaterial(i1 + 1, j1 - 1, k1) != Material::water && world->getBlockMaterial(i1, j1 - 1, k1 - 1) != Material::water && world->getBlockMaterial(i1, j1 - 1, k1 + 1) != Material::water)
			continue;
		int_t l1 = 2 + random.nextInt(random.nextInt(3) + 1);
		for (int_t i2 = 0; i2 < l1; i2++)
		{
			if (Block::reed->canBlockStay(world, i1, j1 + i2, k1))
				world->setBlock(i1, j1 + i2, k1, Block::reed->blockID);
		}
	}
	return true;
}
