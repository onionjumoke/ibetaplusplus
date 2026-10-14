#include "WorldGenTaiga2.h"

#include <cstdlib>
#include "Block.h"
#include "BlockGrass.h"
#include "BlockLeaves.h"
#include "World.h"

bool WorldGenTaiga2::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	int_t l = random.nextInt(4) + 6;
	int_t i1 = 1 + random.nextInt(2);
	int_t j1 = l - i1;
	int_t k1 = 2 + random.nextInt(2);
	bool flag = true;
	if (j < 1 || j + l + 1 > 128)
		return false;
	for (int_t l1 = j; l1 <= j + 1 + l && flag; l1++)
	{
		int_t j2 = 1;
		if (l1 - j < i1)
			j2 = 0;
		else
			j2 = k1;
		for (int_t l2 = i - j2; l2 <= i + j2 && flag; l2++)
		{
			for (int_t j3 = k - j2; j3 <= k + j2 && flag; j3++)
			{
				if (l1 >= 0 && l1 < 128)
				{
					int_t k3 = world->getBlockId(l2, l1, j3);
					if (k3 != 0 && k3 != Block::leaves->blockID)
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
	int_t i2 = world->getBlockId(i, j - 1, k);
	if (i2 != Block::grass->blockID && i2 != Block::dirt->blockID || j >= 128 - l - 1)
		return false;
	world->setBlock(i, j - 1, k, Block::dirt->blockID);
	int_t k2 = random.nextInt(2);
	int_t i3 = 1;
	bool flag1 = false;
	for (int_t l3 = 0; l3 <= j1; l3++)
	{
		int_t j4 = (j + l) - l3;
		for (int_t l4 = i - k2; l4 <= i + k2; l4++)
		{
			int_t j5 = l4 - i;
			for (int_t k5 = k - k2; k5 <= k + k2; k5++)
			{
				int_t l5 = k5 - k;
				if ((std::abs(j5) != k2 || std::abs(l5) != k2 || k2 <= 0) && !Block::opaqueCubeLookup[world->getBlockId(l4, j4, k5)])
					world->setBlockAndMetadata(l4, j4, k5, Block::leaves->blockID, 1);
			}
		}
		if (k2 >= i3)
		{
			k2 = flag1 ? 1 : 0;
			flag1 = true;
			if (++i3 > k1)
				i3 = k1;
		}
		else
		{
			k2++;
		}
	}
	int_t i4 = random.nextInt(3);
	for (int_t k4 = 0; k4 < l - i4; k4++)
	{
		int_t i5 = world->getBlockId(i, j + k4, k);
		if (i5 == 0 || i5 == Block::leaves->blockID)
			world->setBlockAndMetadata(i, j + k4, k, Block::wood->blockID, 1);
	}
	return true;
}
