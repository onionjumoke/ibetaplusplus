#pragma once

#include "Block.h"

// net.minecraft.src.BlockSandStone
class BlockSandStone : public Block
{
public:
	BlockSandStone(int_t i);
	int_t getBlockTextureFromSide(int_t i) override;
};
