#include "ItemStack.h"
#include "Item.h"
#include "Block.h"
#include "EntityPlayer.h"
#include "EntityLiving.h"
#include "World.h"
#include "StatList.h"
#include <sstream>
#include "NBTTagCompound.h"

ItemStack::ItemStack(Block* block) : ItemStack(block, 1) {}
ItemStack::ItemStack(Block* block, int i) : ItemStack(block->blockID, i, 0) {}
ItemStack::ItemStack(Block* block, int i, int j) : ItemStack(block->blockID, i, j) {}
ItemStack::ItemStack(Item* item) : ItemStack(item, 1) {}
ItemStack::ItemStack(Item* item, int i) : ItemStack(item->shiftedIndex, i, 0) {}
ItemStack::ItemStack(Item* item, int i, int j) : ItemStack(item->shiftedIndex, i, j) {}

ItemStack::ItemStack(int i, int j, int k) {
    stackSize = 0;
    itemID = i;
    stackSize = j;
    itemDamage = k;
    animationsToGo = 0;
}

ItemStack::ItemStack(NBTTagCompound* nbttagcompound) {
    stackSize = 0;
    readFromNBT(nbttagcompound);
}

ItemStack* ItemStack::splitStack(int i) {
    stackSize -= i;
    return new ItemStack(itemID, i, itemDamage);
}

Item* ItemStack::getItem() {
    return Item::itemsList[itemID];
}

int ItemStack::getIconIndex() {
    return getItem()->getIconIndex(this);
}

bool ItemStack::useItem(EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
    bool flag = getItem()->onItemUse(this, entityplayer, world, i, j, k, l);
    if (flag) {
        entityplayer->addStat(StatList::useItemStatArray[itemID], 1);
    }
    return flag;
}

float ItemStack::getStrVsBlock(Block* block) {
    return getItem()->getStrVsBlock(this, block);
}

ItemStack* ItemStack::useItemRightClick(World* world, EntityPlayer* entityplayer) {
    return getItem()->onItemRightClick(this, world, entityplayer);
}

NBTTagCompound* ItemStack::writeToNBT(NBTTagCompound* nbttagcompound) {
    nbttagcompound->setShort("id", (short)itemID);
    nbttagcompound->setByte("Count", (char)stackSize);
    nbttagcompound->setShort("Damage", (short)itemDamage);
    return nbttagcompound;
}

void ItemStack::readFromNBT(NBTTagCompound* nbttagcompound) {
    itemID = nbttagcompound->getShort("id");
    stackSize = nbttagcompound->getByte("Count");
    itemDamage = nbttagcompound->getShort("Damage");
}

int ItemStack::getMaxStackSize() {
    return getItem()->getItemStackLimit();
}

bool ItemStack::isStackable() {
    return getMaxStackSize() > 1 && (!isItemStackDamageable() || !isItemDamaged());
}

bool ItemStack::isItemStackDamageable() {
    return Item::itemsList[itemID]->getMaxDamage() > 0;
}

bool ItemStack::getHasSubtypes() {
    return Item::itemsList[itemID]->getHasSubtypes();
}

bool ItemStack::isItemDamaged() {
    return isItemStackDamageable() && itemDamage > 0;
}

int ItemStack::getItemDamageForDisplay() {
    return itemDamage;
}

int ItemStack::getItemDamage() {
    return itemDamage;
}

void ItemStack::setItemDamage(int i) {
    itemDamage = i;
}

int ItemStack::getMaxDamage() {
    return Item::itemsList[itemID]->getMaxDamage();
}

void ItemStack::damageItem(int i, Entity* entity) {
    if (!isItemStackDamageable()) {
        return;
    }
    itemDamage += i;
    if (itemDamage > getMaxDamage()) {
        EntityPlayer* player = dynamic_cast<EntityPlayer*>(entity);
        if (player) {
            player->addStat(StatList::breakItemStatArray[itemID], 1);
        }
        stackSize--;
        if (stackSize < 0) {
            stackSize = 0;
        }
        itemDamage = 0;
    }
}

void ItemStack::hitEntity(EntityLiving* entityliving, EntityPlayer* entityplayer) {
    bool flag = Item::itemsList[itemID]->hitEntity(this, entityliving, entityplayer);
    if (flag) {
        entityplayer->addStat(StatList::useItemStatArray[itemID], 1);
    }
}

void ItemStack::onDestroyBlock(int i, int j, int k, int l, EntityPlayer* entityplayer) {
    bool flag = Item::itemsList[itemID]->onBlockDestroyed(this, i, j, k, l, entityplayer);
    if (flag) {
        entityplayer->addStat(StatList::useItemStatArray[itemID], 1);
    }
}

int ItemStack::getDamageVsEntity(Entity* entity) {
    return Item::itemsList[itemID]->getDamageVsEntity(entity);
}

bool ItemStack::canHarvestBlock(Block* block) {
    return Item::itemsList[itemID]->canHarvestBlock(block);
}

void ItemStack::func_1097_a(EntityPlayer*) {
}

void ItemStack::onItemUse(EntityPlayer* entityplayer) {
}

void ItemStack::useItemOnEntity(EntityLiving* entityliving) {
    Item::itemsList[itemID]->saddleEntity(this, entityliving);
}

ItemStack* ItemStack::copy() {
    return new ItemStack(itemID, stackSize, itemDamage);
}

bool ItemStack::areItemStacksEqual(ItemStack* itemstack, ItemStack* itemstack1) {
    if (itemstack == nullptr && itemstack1 == nullptr) {
        return true;
    }
    if (itemstack == nullptr || itemstack1 == nullptr) {
        return false;
    }
    return itemstack->isItemStackEqual(itemstack1);
}

bool ItemStack::isItemStackEqual(ItemStack* itemstack) {
    if (stackSize != itemstack->stackSize) return false;
    if (itemID != itemstack->itemID) return false;
    return itemDamage == itemstack->itemDamage;
}

bool ItemStack::isItemEqual(ItemStack* itemstack) {
    return itemID == itemstack->itemID && itemDamage == itemstack->itemDamage;
}

std::string ItemStack::getItemName() {
    return Item::itemsList[itemID]->getItemNameIS(this);
}

ItemStack* ItemStack::copyItemStack(ItemStack* itemstack) {
    return itemstack != nullptr ? itemstack->copy() : nullptr;
}

std::string ItemStack::toString() {
    std::ostringstream oss;
    oss << stackSize << "x" << Item::itemsList[itemID]->getItemName() << "@" << itemDamage;
    return oss.str();
}

void ItemStack::updateAnimation(World* world, Entity* entity, int i, bool flag) {
    if (animationsToGo > 0) {
        animationsToGo--;
    }
    Item::itemsList[itemID]->onUpdate(this, world, entity, i, flag);
}

void ItemStack::onCrafting(World* world, EntityPlayer* entityplayer) {
    entityplayer->addStat(StatList::craftItemStatArray[itemID], stackSize);
    Item::itemsList[itemID]->onCreated(this, world, entityplayer);
}

bool ItemStack::isStackEqual(ItemStack* itemstack) {
    return itemID == itemstack->itemID && stackSize == itemstack->stackSize && itemDamage == itemstack->itemDamage;
}
