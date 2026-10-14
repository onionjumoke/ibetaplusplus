#include "BlockGlowStone.h"
#include "Item.h"
#include "java/Random.h"

BlockGlowStone::BlockGlowStone(int_t i, int_t j, Material *material)
    : Block(i, j, material)
{
}

int_t BlockGlowStone::quantityDropped(Random &random)
{
    return 2 + random.nextInt(3);
}

int_t BlockGlowStone::idDropped(int_t i, Random &random)
{
    return Item::lightStoneDust->shiftedIndex;
}
