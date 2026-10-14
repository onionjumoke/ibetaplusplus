#include "BlockFence.h"
#include "Material.h"
#include "World.h"
#include "AxisAlignedBB.h"

BlockFence::BlockFence(int_t i, int_t j) : Block(i, j, Material::wood)
{
}

bool BlockFence::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
	if (world->getBlockId(i, j - 1, k) == blockID) return true;
	if (!world->getBlockMaterial(i, j - 1, k)->isSolid()) return false;
	return Block::canPlaceBlockAt(world, i, j, k);
}

AxisAlignedBB *BlockFence::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	return AxisAlignedBB::getBoundingBoxFromPool(i, j, k, i + 1, (float)j + 1.5f, k + 1);
}

bool  BlockFence::isOpaqueCube()        { return false; }
bool  BlockFence::renderAsNormalBlock() { return false; }
int_t BlockFence::getRenderType()       { return 11; }
