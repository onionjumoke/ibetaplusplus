#include "TileEntityFurnace.h"
#include "World.h"
#include "EntityPlayer.h"
#include "NBTTagCompound.h"
#include "ItemStack.h"
#include "NBTTagList.h"
#include "Item.h"
#include "Block.h"
#include "Material.h"
#include "FurnaceRecipes.h"
#include "BlockFurnace.h"

TileEntityFurnace::TileEntityFurnace() {
    for (int i = 0; i < 3; i++) furnaceItemStacks[i] = nullptr;
    furnaceBurnTime = 0;
    currentItemBurnTime = 0;
    furnaceCookTime = 0;
}

int TileEntityFurnace::getSizeInventory() {
    return 3;
}

ItemStack* TileEntityFurnace::getStackInSlot(int i) {
    return furnaceItemStacks[i];
}

ItemStack* TileEntityFurnace::decrStackSize(int i, int j) {
    if (furnaceItemStacks[i] != nullptr) {
        if (furnaceItemStacks[i]->stackSize <= j) {
            ItemStack* itemstack = furnaceItemStacks[i];
            furnaceItemStacks[i] = nullptr;
            return itemstack;
        }
        ItemStack* itemstack1 = furnaceItemStacks[i]->splitStack(j);
        if (furnaceItemStacks[i]->stackSize == 0) {
            furnaceItemStacks[i] = nullptr;
        }
        return itemstack1;
    } else {
        return nullptr;
    }
}

void TileEntityFurnace::setInventorySlotContents(int i, ItemStack* itemstack) {
    furnaceItemStacks[i] = itemstack;
    if (itemstack != nullptr && itemstack->stackSize > getInventoryStackLimit()) {
        itemstack->stackSize = getInventoryStackLimit();
    }
}

std::string TileEntityFurnace::getInvName() {
    return "Furnace";
}

int TileEntityFurnace::getInventoryStackLimit() {
    return 64;
}

void TileEntityFurnace::readFromNBT(NBTTagCompound* nbttagcompound) {
    TileEntity::readFromNBT(nbttagcompound);
    NBTTagList* nbttaglist = nbttagcompound->getTagList("Items");
    for (int i = 0; i < getSizeInventory(); i++) furnaceItemStacks[i] = nullptr;
    for (int i = 0; i < nbttaglist->tagCount(); i++) {
        NBTTagCompound* nbttagcompound1 = (NBTTagCompound*)nbttaglist->tagAt(i);
        int j = nbttagcompound1->getByte("Slot") & 0xff;
        if (j >= 0 && j < getSizeInventory()) {
            furnaceItemStacks[j] = new ItemStack(nbttagcompound1);
        }
    }
    furnaceBurnTime = nbttagcompound->getShort("BurnTime");
    furnaceCookTime = nbttagcompound->getShort("CookTime");
    currentItemBurnTime = getItemBurnTime(furnaceItemStacks[1]);
}

void TileEntityFurnace::writeToNBT(NBTTagCompound* nbttagcompound) {
    TileEntity::writeToNBT(nbttagcompound);
    nbttagcompound->setShort("BurnTime", (short)furnaceBurnTime);
    nbttagcompound->setShort("CookTime", (short)furnaceCookTime);
    NBTTagList* nbttaglist = new NBTTagList();
    for (int i = 0; i < getSizeInventory(); i++) {
        if (furnaceItemStacks[i] != nullptr) {
            NBTTagCompound* nbttagcompound1 = new NBTTagCompound();
            nbttagcompound1->setByte("Slot", (char)i);
            furnaceItemStacks[i]->writeToNBT(nbttagcompound1);
            nbttaglist->appendTag(nbttagcompound1);
        }
    }
    nbttagcompound->setTag("Items", nbttaglist);
}

int TileEntityFurnace::getCookProgressScaled(int i) {
    return (furnaceCookTime * i) / 200;
}

int TileEntityFurnace::getBurnTimeRemainingScaled(int i) {
    if (currentItemBurnTime == 0) {
        currentItemBurnTime = 200;
    }
    return (furnaceBurnTime * i) / currentItemBurnTime;
}

bool TileEntityFurnace::isBurning() {
    return furnaceBurnTime > 0;
}

void TileEntityFurnace::updateEntity() {
    bool flag = furnaceBurnTime > 0;
    bool flag1 = false;
    if (furnaceBurnTime > 0) {
        furnaceBurnTime--;
    }
    if (!worldObj->multiplayerWorld) {
        if (furnaceBurnTime == 0 && canSmelt()) {
            currentItemBurnTime = furnaceBurnTime = getItemBurnTime(furnaceItemStacks[1]);
            if (furnaceBurnTime > 0) {
                flag1 = true;
                if (furnaceItemStacks[1] != nullptr) {
                    furnaceItemStacks[1]->stackSize--;
                    if (furnaceItemStacks[1]->stackSize == 0) {
                        furnaceItemStacks[1] = nullptr;
                    }
                }
            }
        }
        if (isBurning() && canSmelt()) {
            furnaceCookTime++;
            if (furnaceCookTime == 200) {
                furnaceCookTime = 0;
                smeltItem();
                flag1 = true;
            }
        } else {
            furnaceCookTime = 0;
        }
        if (flag != (furnaceBurnTime > 0)) {
            flag1 = true;
            BlockFurnace::updateFurnaceBlockState(furnaceBurnTime > 0, worldObj, xCoord, yCoord, zCoord);
        }
    }
    if (flag1) {
        onInventoryChanged();
    }
}

bool TileEntityFurnace::canSmelt() {
    if (furnaceItemStacks[0] == nullptr) {
        return false;
    }
    ItemStack* itemstack = FurnaceRecipes::smelting()->getSmeltingResult(furnaceItemStacks[0]->getItem()->shiftedIndex);
    if (itemstack == nullptr) {
        return false;
    }
    if (furnaceItemStacks[2] == nullptr) {
        return true;
    }
    if (!furnaceItemStacks[2]->isItemEqual(itemstack)) {
        return false;
    }
    if (furnaceItemStacks[2]->stackSize < getInventoryStackLimit() && furnaceItemStacks[2]->stackSize < furnaceItemStacks[2]->getMaxStackSize()) {
        return true;
    }
    return furnaceItemStacks[2]->stackSize < itemstack->getMaxStackSize();
}

void TileEntityFurnace::smeltItem() {
    if (!canSmelt()) {
        return;
    }
    ItemStack* itemstack = FurnaceRecipes::smelting()->getSmeltingResult(furnaceItemStacks[0]->getItem()->shiftedIndex);
    if (furnaceItemStacks[2] == nullptr) {
        furnaceItemStacks[2] = itemstack->copy();
    } else if (furnaceItemStacks[2]->itemID == itemstack->itemID) {
        furnaceItemStacks[2]->stackSize++;
    }
    furnaceItemStacks[0]->stackSize--;
    if (furnaceItemStacks[0]->stackSize <= 0) {
        furnaceItemStacks[0] = nullptr;
    }
}

int TileEntityFurnace::getItemBurnTime(ItemStack* itemstack) {
    if (itemstack == nullptr) {
        return 0;
    }
    int i = itemstack->getItem()->shiftedIndex;
    if (i < 256 && Block::blocksList[i]->blockMaterial == Material::wood) {
        return 300;
    }
    if (i == Item::stick->shiftedIndex) {
        return 100;
    }
    if (i == Item::coal->shiftedIndex) {
        return 1600;
    }
    if (i == Item::bucketLava->shiftedIndex) {
        return 20000;
    }
    return i != Block::sapling->blockID ? 0 : 100;
}

bool TileEntityFurnace::isItemFuel(ItemStack* itemstack) {
    return getItemBurnTime(itemstack) > 0;
}

bool TileEntityFurnace::canInteractWith(EntityPlayer *entityplayer)
{
    if (worldObj->getBlockTileEntity(xCoord, yCoord, zCoord) != this) {
        return false;
    }
    return entityplayer->getDistanceSq(xCoord + 0.5, yCoord + 0.5, zCoord + 0.5) <= 64.0;
}
