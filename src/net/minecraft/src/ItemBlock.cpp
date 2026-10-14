#include "ItemBlock.h"
#include "ItemStack.h"
#include "Block.h"
#include "World.h"
#include "Material.h"
#include "StepSound.h"
#include "EntityPlayer.h"

ItemBlock::ItemBlock(int i)
    : Item(i) {
    blockID = i + 256;
    setIconIndex(Block::blocksList[i + 256]->getBlockTextureFromSide(2));
}

bool ItemBlock::onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
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
    if (j == 127 && Block::blocksList[blockID]->blockMaterial->isSolid()) {
        return false;
    }
    if (world->canBlockBePlacedAt(blockID, i, j, k, false, l)) {
        Block* block = Block::blocksList[blockID];
        if (world->setBlockAndMetadataWithNotify(i, j, k, blockID, getPlacedBlockMetadata(itemstack->getItemDamage()))) {
            Block::blocksList[blockID]->onBlockPlaced(world, i, j, k, l);
            Block::blocksList[blockID]->onBlockPlacedBy(world, i, j, k, entityplayer);
            world->playSoundEffect((float)i + 0.5f, (float)j + 0.5f, (float)k + 0.5f, block->stepSound->getBreakSound(), (block->stepSound->getVolume() + 1.0f) / 2.0f, block->stepSound->getPitch() * 0.8f);
            itemstack->stackSize--;
        }
        return true;
    } else {
        return false;
    }
}

std::string ItemBlock::getItemNameIS(ItemStack* itemstack) {
    return Block::blocksList[blockID]->getBlockName();
}

std::string ItemBlock::getItemName() {
    return Block::blocksList[blockID]->getBlockName();
}
