#include "WorldGenLiquids.h"

#include "Block.h"
#include "World.h"

WorldGenLiquids::WorldGenLiquids(int_t i)
{
	liquidBlockId = i;
}

bool WorldGenLiquids::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	if (world->getBlockId(i, j + 1, k) != Block::stone->blockID)
		return false;
	if (world->getBlockId(i, j - 1, k) != Block::stone->blockID)
		return false;
	if (world->getBlockId(i, j, k) != 0 && world->getBlockId(i, j, k) != Block::stone->blockID)
		return false;
	int_t l = 0;
	if (world->getBlockId(i - 1, j, k) == Block::stone->blockID) l++;
	if (world->getBlockId(i + 1, j, k) == Block::stone->blockID) l++;
	if (world->getBlockId(i, j, k - 1) == Block::stone->blockID) l++;
	if (world->getBlockId(i, j, k + 1) == Block::stone->blockID) l++;
	int_t i1 = 0;
	if (world->isAirBlock(i - 1, j, k)) i1++;
	if (world->isAirBlock(i + 1, j, k)) i1++;
	if (world->isAirBlock(i, j, k - 1)) i1++;
	if (world->isAirBlock(i, j, k + 1)) i1++;
	if (l == 3 && i1 == 1)
	{
		world->setBlockWithNotify(i, j, k, liquidBlockId);
		world->scheduledUpdatesAreImmediate = true;
		if (Block::blocksList[liquidBlockId] == nullptr)
			return false;
		Block::blocksList[liquidBlockId]->updateTick(world, i, j, k, random);
		world->scheduledUpdatesAreImmediate = false;
	}
	return true;
}
