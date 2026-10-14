#pragma once

#include "Block.h"

class IBlockAccess;

// net.minecraft.src.BlockStep
class BlockStep : public Block
{
public:
	BlockStep(int_t i, bool flag);
	int_t getBlockTextureFromSideAndMetadata(int_t i, int_t j) override;
	int_t getBlockTextureFromSide(int_t i) override;
	bool isOpaqueCube() override;
	void onBlockAdded(World *world, int_t i, int_t j, int_t k) override;
	int_t idDropped(int_t i, Random &random) override;
	int_t quantityDropped(Random &random) override;
	bool renderAsNormalBlock() override;
	bool shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;

	static const jstring bedDirection[4];

protected:
	int_t damageDropped(int_t i) override;

private:
	bool blockType;
};
