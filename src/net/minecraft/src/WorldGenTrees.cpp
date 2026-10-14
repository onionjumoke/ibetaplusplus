#include "WorldGenTrees.h"

#include <cstdlib>
#include "Block.h"
#include "BlockGrass.h"
#include "BlockLeaves.h"
#include "World.h"

bool WorldGenTrees::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	int_t l = random.nextInt(3) + 4;
	bool flag = true;
	if (j < 1 || j + l + 1 > 128)
		return false;
	for (int_t i1 = j; i1 <= j + 1 + l; i1++)
	{
		int_t byte0 = 1;
		if (i1 == j) byte0 = 0;
		if (i1 >= (j + 1 + l) - 2) byte0 = 2;
		for (int_t i2 = i - byte0; i2 <= i + byte0 && flag; i2++)
		{
			for (int_t l2 = k - byte0; l2 <= k + byte0 && flag; l2++)
			{
				if (i1 >= 0 && i1 < 128)
				{
					int_t j3 = world->getBlockId(i2, i1, l2);
					if (j3 != 0 && j3 != Block::leaves->blockID)
						flag = false;
				}
				else
				{
					flag = false;
				}
			}
		}
	}
	if (!flag)
		return false;
	int_t j1 = world->getBlockId(i, j - 1, k);
	if (j1 != Block::grass->blockID && j1 != Block::dirt->blockID || j >= 128 - l - 1)
		return false;
	world->setBlock(i, j - 1, k, Block::dirt->blockID);
	for (int_t k1 = (j - 3) + l; k1 <= j + l; k1++)
	{
		int_t j2 = k1 - (j + l);
		int_t i3 = 1 - j2 / 2;
		for (int_t k3 = i - i3; k3 <= i + i3; k3++)
		{
			int_t l3 = k3 - i;
			for (int_t i4 = k - i3; i4 <= k + i3; i4++)
			{
				int_t j4 = i4 - k;
				if ((std::abs(l3) != i3 || std::abs(j4) != i3 || random.nextInt(2) != 0 && j2 != 0) && !Block::opaqueCubeLookup[world->getBlockId(k3, k1, i4)])
					world->setBlock(k3, k1, i4, Block::leaves->blockID);
			}
		}
	}
	for (int_t l1 = 0; l1 < l; l1++)
	{
		int_t k2 = world->getBlockId(i, j + l1, k);
		if (k2 == 0 || k2 == Block::leaves->blockID)
			world->setBlock(i, j + l1, k, Block::wood->blockID);
	}
	return true;
}
