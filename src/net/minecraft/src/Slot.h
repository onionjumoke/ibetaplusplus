#pragma once

#include "java/Type.h"

class IInventory;
class ItemStack;

// net.minecraft.src.Slot
class Slot
{
public:
	Slot(IInventory *iinventory, int_t i, int_t j, int_t k);
	virtual ~Slot() = default;

	virtual void onPickupFromSlot(ItemStack *itemstack);
	virtual bool isItemValid(ItemStack *itemstack);
	ItemStack *getStack();
	bool getHasStack();
	void putStack(ItemStack *itemstack);
	virtual void onSlotChanged();
	virtual int_t getSlotStackLimit();
	virtual int_t getBackgroundIconIndex();
	ItemStack *decrStackSize(int_t i);

private:
	const int_t slotIndex;
	IInventory * const inventory;

public:
	int_t slotNumber = 0;
	int_t xDisplayPosition;
	int_t yDisplayPosition;
};
