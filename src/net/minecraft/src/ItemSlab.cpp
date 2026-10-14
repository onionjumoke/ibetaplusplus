#include "ItemSlab.h"
#include "ItemStack.h"
#include "Block.h"
#include "BlockStep.h"
#include "java/String.h"

ItemSlab::ItemSlab(int i)
    : ItemBlock(i) {
    setMaxDamage(0);
    setHasSubtypes(true);
}

int ItemSlab::getIconFromDamage(int i) {
    return Block::stairSingle->getBlockTextureFromSideAndMetadata(2, i);
}

int ItemSlab::getPlacedBlockMetadata(int i) {
    return i;
}

std::string ItemSlab::getItemNameIS(ItemStack* itemstack) {
    return getItemName() + "." + String::toUTF8(BlockStep::bedDirection[itemstack->getItemDamage()]);
}
