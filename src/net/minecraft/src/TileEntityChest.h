#pragma once

#include <string>

#include "IInventory.h"
#include "TileEntity.h"
#include "java/Type.h"

class EntityPlayer;
class ItemStack;
class NBTTagCompound;

// net.minecraft.src.TileEntityChest
class TileEntityChest : public TileEntity, public IInventory
{
public:
	TileEntityChest();

	int_t getSizeInventory() override;
	ItemStack *getStackInSlot(int_t i) override;
	ItemStack *decrStackSize(int_t i, int_t j) override;
	void setInventorySlotContents(int_t i, ItemStack *itemstack) override;
	std::string getInvName() override;
	void readFromNBT(NBTTagCompound *nbttagcompound) override;
	void writeToNBT(NBTTagCompound *nbttagcompound) override;
	int_t getInventoryStackLimit() override;
	void onInventoryChanged() override;
	bool canInteractWith(EntityPlayer *entityplayer) override;

	// Java had no `checkForAdjacentChests` / `updateEntity` on TileEntityChest in
	// 1.7.3; only `canInteractWith` and the inventory methods above exist.

public:
	ItemStack *chestContents[36];
};
