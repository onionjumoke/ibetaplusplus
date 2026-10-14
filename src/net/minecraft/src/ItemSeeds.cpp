#include "ItemSeeds.h"
#include "ItemStack.h"
#include "EntityPlayer.h"
#include "World.h"
#include "Block.h"

ItemSeeds::ItemSeeds(int i, int j)
    : Item(i) {
    field_318_a = j;
}

bool ItemSeeds::onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
    if (l != 1) {
        return false;
    }
    int i1 = world->getBlockId(i, j, k);
    if (i1 == Block::tilledField->blockID && world->isAirBlock(i, j + 1, k)) {
        world->setBlockWithNotify(i, j + 1, k, field_318_a);
        itemstack->stackSize--;
        return true;
    } else {
        return false;
    }
}
