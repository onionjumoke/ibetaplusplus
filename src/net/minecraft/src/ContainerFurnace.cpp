#include "ContainerFurnace.h"

#include "Slot.h"
#include "SlotFurnace.h"
#include "InventoryPlayer.h"
#include "ICrafting.h"
#include "TileEntityFurnace.h"
#include "ItemStack.h"
#include "EntityPlayer.h"

ContainerFurnace::ContainerFurnace(InventoryPlayer *inventoryplayer, TileEntityFurnace *tileentityfurnace)
{
	cookTime = 0;
	burnTime = 0;
	itemBurnTime = 0;
	furnace = tileentityfurnace;
	addSlot(new Slot(tileentityfurnace, 0, 56, 17));
	addSlot(new Slot(tileentityfurnace, 1, 56, 53));
	addSlot(new SlotFurnace(inventoryplayer->player, tileentityfurnace, 2, 116, 35));
	for(int_t i = 0; i < 3; i++)
	{
		for(int_t k = 0; k < 9; k++)
		{
			addSlot(new Slot(inventoryplayer, k + i * 9 + 9, 8 + k * 18, 84 + i * 18));
		}
	}
	for(int_t j = 0; j < 9; j++)
	{
		addSlot(new Slot(inventoryplayer, j, 8 + j * 18, 142));
	}
}

void ContainerFurnace::updateCraftingResults()
{
	Container::updateCraftingResults();
	for(size_t i = 0; i < crafters.size(); i++)
	{
		ICrafting *icrafting = crafters[i];
		if(cookTime != furnace->furnaceCookTime)
		{
			icrafting->sendProgressBarUpdate(this, 0, furnace->furnaceCookTime);
		}
		if(burnTime != furnace->furnaceBurnTime)
		{
			icrafting->sendProgressBarUpdate(this, 1, furnace->furnaceBurnTime);
		}
		if(itemBurnTime != furnace->currentItemBurnTime)
		{
			icrafting->sendProgressBarUpdate(this, 2, furnace->currentItemBurnTime);
		}
	}
	cookTime = furnace->furnaceCookTime;
	burnTime = furnace->furnaceBurnTime;
	itemBurnTime = furnace->currentItemBurnTime;
}

void ContainerFurnace::updateProgressBar(int_t id, int_t value)
{
	if(id == 0)
	{
		furnace->furnaceCookTime = value;
	}
	if(id == 1)
	{
		furnace->furnaceBurnTime = value;
	}
	if(id == 2)
	{
		furnace->currentItemBurnTime = value;
	}
}

bool ContainerFurnace::isUsableByPlayer(EntityPlayer *entityplayer)
{
	return furnace->canInteractWith(entityplayer);
}

ItemStack *ContainerFurnace::getStackInSlot(int_t i)
{
	ItemStack *itemstack = nullptr;
	Slot *slot = slots[i];
	if(slot != nullptr && slot->getHasStack())
	{
		ItemStack *itemstack1 = slot->getStack();
		itemstack = itemstack1->copy();
		if(i == 2)
		{
			mergeItemStack(itemstack1, 3, 39, true);
		}
		else if(i >= 3 && i < 30)
		{
			mergeItemStack(itemstack1, 30, 39, false);
		}
		else if(i >= 30 && i < 39)
		{
			mergeItemStack(itemstack1, 3, 30, false);
		}
		else
		{
			mergeItemStack(itemstack1, 3, 39, false);
		}
		if(itemstack1->stackSize == 0)
		{
			slot->putStack(nullptr);
		}
		else
		{
			slot->onSlotChanged();
		}
		if(itemstack1->stackSize != itemstack->stackSize)
		{
			slot->onPickupFromSlot(itemstack1);
		}
		else
		{
			return nullptr;
		}
	}
	return itemstack;
}
