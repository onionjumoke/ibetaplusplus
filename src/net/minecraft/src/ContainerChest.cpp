#include "ContainerChest.h"

#include "IInventory.h"
#include "Slot.h"
#include "ItemStack.h"
#include "EntityPlayer.h"

ContainerChest::ContainerChest(IInventory *iinventory, IInventory *iinventory1)
{
	lowerChestInventory = iinventory1;
	numRows = iinventory1->getSizeInventory() / 9;
	int_t i = (numRows - 4) * 18;
	for(int_t j = 0; j < numRows; j++)
	{
		for(int_t i1 = 0; i1 < 9; i1++)
		{
			addSlot(new Slot(iinventory1, i1 + j * 9, 8 + i1 * 18, 18 + j * 18));
		}
	}
	for(int_t k = 0; k < 3; k++)
	{
		for(int_t j1 = 0; j1 < 9; j1++)
		{
			addSlot(new Slot(iinventory, j1 + k * 9 + 9, 8 + j1 * 18, 103 + k * 18 + i));
		}
	}
	for(int_t l = 0; l < 9; l++)
	{
		addSlot(new Slot(iinventory, l, 8 + l * 18, 161 + i));
	}
}

bool ContainerChest::isUsableByPlayer(EntityPlayer *entityplayer)
{
	return lowerChestInventory->canInteractWith(entityplayer);
}

ItemStack *ContainerChest::getStackInSlot(int_t i)
{
	ItemStack *itemstack = nullptr;
	Slot *slot = slots[i];
	if(slot != nullptr && slot->getHasStack())
	{
		ItemStack *itemstack1 = slot->getStack();
		itemstack = itemstack1->copy();
		if(i < numRows * 9)
		{
			mergeItemStack(itemstack1, numRows * 9, slots.size(), true);
		}
		else
		{
			mergeItemStack(itemstack1, 0, numRows * 9, false);
		}
		if(itemstack1->stackSize == 0)
		{
			slot->putStack(nullptr);
		}
		else
		{
			slot->onSlotChanged();
		}
	}
	return itemstack;
}
