#include "BlockGlass.h"

BlockGlass::BlockGlass(int_t i, int_t j, Material *material, bool flag)
    : BlockBreakable(i, j, material, flag)
{
}

int_t BlockGlass::quantityDropped(Random &random)
{
    return 0;
}

int_t BlockGlass::getRenderBlockPass()
{
    return 0;
}
