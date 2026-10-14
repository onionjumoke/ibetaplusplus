#pragma once

#include "Block.h"

class AxisAlignedBB;

// net.minecraft.src.BlockFence
class BlockFence : public Block
{
public:
	BlockFence(int_t i, int_t j);
	bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
	AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k) override;
	bool isOpaqueCube() override;
	bool renderAsNormalBlock() override;
	int_t getRenderType() override;
};
