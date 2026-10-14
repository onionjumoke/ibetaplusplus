#pragma once

#include "Block.h"

class Material;
class World;
class TileEntity;

// net.minecraft.src.BlockContainer
class BlockContainer : public Block
{
protected:
	BlockContainer(int_t i, Material *material);
	BlockContainer(int_t i, int_t j, Material *material);

public:
	void onBlockAdded(World *world, int_t i, int_t j, int_t k) override;
	void onBlockRemoval(World *world, int_t i, int_t j, int_t k) override;

protected:
	virtual TileEntity *getBlockEntity() = 0;
};
