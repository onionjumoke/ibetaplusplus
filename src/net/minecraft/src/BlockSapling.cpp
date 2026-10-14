#include "BlockSapling.h"
#include "World.h"
#include "WorldGenTaiga2.h"
#include "WorldGenForest.h"
#include "WorldGenTrees.h"
#include "WorldGenBigTree.h"

BlockSapling::BlockSapling(int_t i, int_t j) : BlockFlower(i, j)
{
	float f = 0.4f;
	setBlockBounds(0.5f - f, 0.0f, 0.5f - f, 0.5f + f, f * 2.0f, 0.5f + f);
}

void BlockSapling::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	if (world->multiplayerWorld) return;
	BlockFlower::updateTick(world, i, j, k, random);
	if (world->getBlockLightValue(i, j + 1, k) >= 9 && random.nextInt(30) == 0)
	{
		int_t l = world->getBlockMetadata(i, j, k);
		if ((l & 8) == 0)
		{
			world->setBlockMetadataWithNotify(i, j, k, l | 8);
		}
		else
		{
			growTree(world, i, j, k, random);
		}
	}
}

int_t BlockSapling::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
	j &= 3;
	if (j == 1) return 63;
	if (j == 2) return 79;
	return BlockFlower::getBlockTextureFromSideAndMetadata(i, j);
}

void BlockSapling::growTree(World *world, int_t i, int_t j, int_t k, Random &random)
{
	int_t l = world->getBlockMetadata(i, j, k) & 3;
	world->setBlock(i, j, k, 0);
	WorldGenerator *obj = nullptr;
	if (l == 1)
	{
		obj = new WorldGenTaiga2();
	}
	else if (l == 2)
	{
		obj = new WorldGenForest();
	}
	else
	{
		obj = new WorldGenTrees();
		if (random.nextInt(10) == 0)
		{
			delete obj;
			obj = new WorldGenBigTree();
		}
	}
	if (!obj->generate(world, random, i, j, k))
	{
		world->setBlockAndMetadata(i, j, k, blockID, l);
	}
	delete obj;
}

int_t BlockSapling::damageDropped(int_t i) { return i & 3; }
