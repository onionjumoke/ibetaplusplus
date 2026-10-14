#include "Slot.h"

#include "IInventory.h"
#include "ItemStack.h"

Slot::Slot(IInventory *iinventory, int_t i, int_t j, int_t k) :
	inventory(iinventory),
	slotIndex(i),
	xDisplayPosition(j),
	yDisplayPosition(k)
{
}

void Slot::onPickupFromSlot(ItemStack *itemstack)
{
	onSlotChanged();
}

bool Slot::isItemValid(ItemStack *itemstack)
{
	return true;
}

ItemStack *Slot::getStack()
{
	return inventory->getStackInSlot(slotIndex);
}

bool Slot::getHasStack()
{
	return getStack() != nullptr;
}

void Slot::putStack(ItemStack *itemstack)
{
	inventory->setInventorySlotContents(slotIndex, itemstack);
	onSlotChanged();
}

void Slot::onSlotChanged()
{
	inventory->onInventoryChanged();
}

int_t Slot::getSlotStackLimit()
{
	return inventory->getInventoryStackLimit();
}

int_t Slot::getBackgroundIconIndex()
{
	return -1;
}

ItemStack *Slot::decrStackSize(int_t i)
{
	return inventory->decrStackSize(slotIndex, i);
}
