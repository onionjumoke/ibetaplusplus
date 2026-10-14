#include "TileEntityChest.h"
#include "NBTTagCompound.h"
#include "ItemStack.h"
#include "NBTTagList.h"
#include "Block.h"
#include "World.h"
#include "EntityPlayer.h"

TileEntityChest::TileEntityChest() {
    for (int i = 0; i < 36; i++) chestContents[i] = nullptr;
}

int TileEntityChest::getSizeInventory() {
    return 27;
}

ItemStack* TileEntityChest::getStackInSlot(int i) {
    return chestContents[i];
}

ItemStack* TileEntityChest::decrStackSize(int i, int j) {
    if (chestContents[i] != nullptr) {
        if (chestContents[i]->stackSize <= j) {
            ItemStack* itemstack = chestContents[i];
            chestContents[i] = nullptr;
            onInventoryChanged();
            return itemstack;
        }
        ItemStack* itemstack1 = chestContents[i]->splitStack(j);
        if (chestContents[i]->stackSize == 0) {
            chestContents[i] = nullptr;
        }
        onInventoryChanged();
        return itemstack1;
    } else {
        return nullptr;
    }
}

void TileEntityChest::setInventorySlotContents(int i, ItemStack* itemstack) {
    chestContents[i] = itemstack;
    if (itemstack != nullptr && itemstack->stackSize > getInventoryStackLimit()) {
        itemstack->stackSize = getInventoryStackLimit();
    }
    onInventoryChanged();
}

std::string TileEntityChest::getInvName() {
    return "Chest";
}

int TileEntityChest::getInventoryStackLimit() {
    return 64;
}

void TileEntityChest::readFromNBT(NBTTagCompound* nbttagcompound) {
    TileEntity::readFromNBT(nbttagcompound);
    NBTTagList* nbttaglist = nbttagcompound->getTagList("Items");
    for (int i = 0; i < nbttaglist->tagCount(); i++) {
        NBTTagCompound* nbttagcompound1 = (NBTTagCompound*)nbttaglist->tagAt(i);
        int j = nbttagcompound1->getByte("Slot") & 0xff;
        if (j >= 0 && j < getSizeInventory()) {
            chestContents[j] = new ItemStack(nbttagcompound1);
        }
    }
}

void TileEntityChest::writeToNBT(NBTTagCompound* nbttagcompound) {
    TileEntity::writeToNBT(nbttagcompound);
    NBTTagList* nbttaglist = new NBTTagList();
    for (int i = 0; i < getSizeInventory(); i++) {
        if (chestContents[i] != nullptr) {
            NBTTagCompound* nbttagcompound1 = new NBTTagCompound();
            nbttagcompound1->setByte("Slot", (char)i);
            chestContents[i]->writeToNBT(nbttagcompound1);
            nbttaglist->setTag(nbttagcompound1);
        }
    }
    nbttagcompound->setTag("Items", nbttaglist);
}

void TileEntityChest::onInventoryChanged()
{
    TileEntity::onInventoryChanged();
}

bool TileEntityChest::canInteractWith(EntityPlayer *entityplayer)
{
    if (worldObj == nullptr || worldObj->getBlockId(xCoord, yCoord, zCoord) != Block::chest->blockID)
        return false;
    return entityplayer->getDistanceSq(xCoord + 0.5, yCoord + 0.5, zCoord + 0.5) <= 64.0;
}
