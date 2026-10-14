#pragma once

#include "Slot.h"

class EntityPlayer;
class IInventory;
class ItemStack;

// net.minecraft.src.SlotFurnace
class SlotFurnace : public Slot
{
public:
	SlotFurnace(EntityPlayer *entityplayer, IInventory *iinventory, int_t i, int_t j, int_t k);

	bool isItemValid(ItemStack *itemstack) override;
	void onPickupFromSlot(ItemStack *itemstack) override;

private:
	EntityPlayer *thePlayer;
};
