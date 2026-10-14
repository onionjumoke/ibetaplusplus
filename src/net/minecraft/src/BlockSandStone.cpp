#include "BlockSandStone.h"
#include "Material.h"

BlockSandStone::BlockSandStone(int_t i) : Block(i, 192, Material::rock)
{
}

int_t BlockSandStone::getBlockTextureFromSide(int_t i)
{
    if (i == 1) return blockIndexInTexture - 16;
    if (i == 0) return blockIndexInTexture + 16;
    return blockIndexInTexture;
}
