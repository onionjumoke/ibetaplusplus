#pragma once

#include "Slot.h"

class EntityPlayer;
class IInventory;
class ItemStack;

// net.minecraft.src.SlotCrafting
class SlotCrafting : public Slot
{
public:
	SlotCrafting(EntityPlayer *entityplayer, IInventory *iinventory, IInventory *iinventory1, int_t i, int_t j, int_t k);

	bool isItemValid(ItemStack *itemstack) override;
	void onPickupFromSlot(ItemStack *itemstack) override;

private:
	IInventory *craftMatrix;
	EntityPlayer *thePlayer;
};
