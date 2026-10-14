#pragma once

#include "BlockContainer.h"

class TileEntityRecordPlayer;
class EntityPlayer;

// net.minecraft.src.BlockJukeBox
class BlockJukeBox : public BlockContainer
{
public:
	BlockJukeBox(int_t i, int_t j);
	int_t getBlockTextureFromSide(int_t i) override;
	bool blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;
	void ejectRecord(World *world, int_t i, int_t j, int_t k, int_t l);
	void getRecordItem(World *world, int_t i, int_t j, int_t k);
	void onBlockRemoval(World *world, int_t i, int_t j, int_t k) override;
	void dropBlockAsItemWithChance(World *world, int_t i, int_t j, int_t k, int_t l, float f) override;

protected:
	TileEntity *getBlockEntity() override;
};
