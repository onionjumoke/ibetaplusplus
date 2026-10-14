#include "BlockDeadBush.h"
#include "Block.h"

BlockDeadBush::BlockDeadBush(int_t i, int_t j) : BlockFlower(i, j)
{
    float f = 0.4f;
    setBlockBounds(0.5f - f, 0.0f, 0.5f - f, 0.5f + f, 0.8f, 0.5f + f);
}

bool BlockDeadBush::canThisPlantGrowOnThisBlockID(int_t i)
{
    return i == Block::sand->blockID;
}

int_t BlockDeadBush::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
    return blockIndexInTexture;
}

int_t BlockDeadBush::idDropped(int_t i, Random &random)
{
    return -1;
}
