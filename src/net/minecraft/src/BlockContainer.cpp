#include "BlockContainer.h"

#include "World.h"

BlockContainer::BlockContainer(int_t i, Material *material) :
	Block(i, material)
{
	isBlockContainer[i] = true;
}

BlockContainer::BlockContainer(int_t i, int_t j, Material *material) :
	Block(i, j, material)
{
	isBlockContainer[i] = true;
}

void BlockContainer::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
	Block::onBlockAdded(world, i, j, k);
	world->setBlockTileEntity(i, j, k, getBlockEntity());
}

void BlockContainer::onBlockRemoval(World *world, int_t i, int_t j, int_t k)
{
	Block::onBlockRemoval(world, i, j, k);
	world->removeBlockTileEntity(i, j, k);
}
