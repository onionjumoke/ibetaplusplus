#include "ItemReed.h"
#include "Block.h"
#include "World.h"
#include "ItemStack.h"
#include "StepSound.h"
#include "EntityPlayer.h"

ItemReed::ItemReed(int i, Block* block)
    : Item(i), itemID(block->blockID) {
}

bool ItemReed::onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
    if (world->getBlockId(i, j, k) == Block::snow->blockID) {
        l = 0;
    } else {
        if (l == 0) j--;
        if (l == 1) j++;
        if (l == 2) k--;
        if (l == 3) k++;
        if (l == 4) i--;
        if (l == 5) i++;
    }
    if (itemstack->stackSize == 0) {
        return false;
    }
    if (world->canBlockBePlacedAt(itemID, i, j, k, false, l)) {
        Block* block = Block::blocksList[itemID];
        if (world->setBlockWithNotify(i, j, k, itemID)) {
            Block::blocksList[itemID]->onBlockPlaced(world, i, j, k, l);
            Block::blocksList[itemID]->onBlockPlacedBy(world, i, j, k, entityplayer);
            world->playSoundEffect((float)i + 0.5f, (float)j + 0.5f, (float)k + 0.5f, block->stepSound->getBreakSound(), (block->stepSound->getVolume() + 1.0f) / 2.0f, block->stepSound->getPitch() * 0.8f);
            itemstack->stackSize--;
        }
    }
    return true;
}
