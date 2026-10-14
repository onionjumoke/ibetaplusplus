#include "WorldGenHellLava.h"

#include "Block.h"
#include "World.h"

WorldGenHellLava::WorldGenHellLava(int_t i)
{
	field_4158_a = i;
}

bool WorldGenHellLava::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	if (world->getBlockId(i, j + 1, k) != Block::netherrack->blockID)
		return false;
	if (world->getBlockId(i, j, k) != 0 && world->getBlockId(i, j, k) != Block::netherrack->blockID)
		return false;
	int_t l = 0;
	if (world->getBlockId(i - 1, j, k) == Block::netherrack->blockID) l++;
	if (world->getBlockId(i + 1, j, k) == Block::netherrack->blockID) l++;
	if (world->getBlockId(i, j, k - 1) == Block::netherrack->blockID) l++;
	if (world->getBlockId(i, j, k + 1) == Block::netherrack->blockID) l++;
	if (world->getBlockId(i, j - 1, k) == Block::netherrack->blockID) l++;
	int_t i1 = 0;
	if (world->isAirBlock(i - 1, j, k)) i1++;
	if (world->isAirBlock(i + 1, j, k)) i1++;
	if (world->isAirBlock(i, j, k - 1)) i1++;
	if (world->isAirBlock(i, j, k + 1)) i1++;
	if (world->isAirBlock(i, j - 1, k)) i1++;
	if (l == 4 && i1 == 1)
	{
		world->setBlockWithNotify(i, j, k, field_4158_a);
		world->scheduledUpdatesAreImmediate = true;
		if (Block::blocksList[field_4158_a] == nullptr)
			return false;
		Block::blocksList[field_4158_a]->updateTick(world, i, j, k, random);
		world->scheduledUpdatesAreImmediate = false;
	}
	return true;
}
