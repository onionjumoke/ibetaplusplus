#include "BlockMushroom.h"
#include "World.h"
#include "Block.h"

BlockMushroom::BlockMushroom(int_t i, int_t j) : BlockFlower(i, j)
{
    float f = 0.2f;
    setBlockBounds(0.5f - f, 0.0f, 0.5f - f, 0.5f + f, f * 2.0f, 0.5f + f);
    setTickOnLoad(true);
}

void BlockMushroom::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
    if (random.nextInt(100) == 0)
    {
        int_t l  = (i + random.nextInt(3)) - 1;
        int_t i1 = (j + random.nextInt(2)) - random.nextInt(2);
        int_t j1 = (k + random.nextInt(3)) - 1;
        if (world->isAirBlock(l, i1, j1) && canBlockStay(world, l, i1, j1))
        {
            i += random.nextInt(3) - 1;
            k += random.nextInt(3) - 1;
            if (world->isAirBlock(l, i1, j1) && canBlockStay(world, l, i1, j1))
                world->setBlockWithNotify(l, i1, j1, blockID);
        }
    }
}

bool BlockMushroom::canThisPlantGrowOnThisBlockID(int_t i)
{
    return Block::opaqueCubeLookup[i];
}

bool BlockMushroom::canBlockStay(World *world, int_t i, int_t j, int_t k)
{
    if (j < 0 || j >= 128) return false;
    return world->getFullBlockLightValue(i, j, k) < 13
           && canThisPlantGrowOnThisBlockID(world->getBlockId(i, j - 1, k));
}
