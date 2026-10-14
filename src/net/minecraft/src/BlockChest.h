#pragma once
#include "BlockContainer.h"
#include "java/Random.h"

class EntityPlayer;
class TileEntityChest;

// net.minecraft.src.BlockChest
class BlockChest : public BlockContainer
{
public:
    BlockChest(int_t i);
    int_t getBlockTexture(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l) override;
    int_t getBlockTextureFromSide(int_t i) override;
    bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k) override;
    void onBlockRemoval(World *world, int_t i, int_t j, int_t k) override;
    bool blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer) override;

protected:
    TileEntity *getBlockEntity() override;

private:
    bool isThereANeighborChest(World *world, int_t i, int_t j, int_t k);
    Random random;
};
