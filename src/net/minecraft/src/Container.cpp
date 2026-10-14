#include "Container.h"

#include "Slot.h"
#include "ItemStack.h"
#include "ICrafting.h"
#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "IInventory.h"

Container::Container()
{
	inventoryItemStacks.clear();
	slots.clear();
	windowId = 0;
	transactionID = 0;
	crafters.clear();
	playerList.clear();
}

Container::~Container()
{
	for (Slot *slot : slots)
		delete slot;
	for (ItemStack *stack : inventoryItemStacks)
		delete stack;
}

void Container::addSlot(Slot *slot)
{
	slot->slotNumber = slots.size();
	slots.push_back(slot);
	inventoryItemStacks.push_back(nullptr);
}

void Container::updateCraftingResults()
{
	for(size_t i = 0; i < slots.size(); i++)
	{
		ItemStack *itemstack = slots[i]->getStack();
		ItemStack *itemstack1 = inventoryItemStacks[i];
		if(ItemStack::areItemStacksEqual(itemstack1, itemstack))
		{
			continue;
		}
		delete itemstack1;
		itemstack1 = itemstack != nullptr ? itemstack->copy() : nullptr;
		inventoryItemStacks[i] = itemstack1;
		for(size_t j = 0; j < crafters.size(); j++)
		{
			crafters[j]->sendSlotContents(this, i, itemstack1);
		}
	}
}

Slot *Container::getSlot(int_t i)
{
	return slots[i];
}

ItemStack *Container::getStackInSlot(int_t i)
{
	Slot *slot = slots[i];
	if(slot != nullptr)
	{
		return slot->getStack();
	}
	else
	{
		return nullptr;
	}
}

ItemStack *Container::slotClick(int_t slotIndex, int_t mouseButton, bool_t shiftClick, EntityPlayer *entityplayer)
{
	ItemStack *itemstack = nullptr;
	if(mouseButton == 0 || mouseButton == 1)
	{
		InventoryPlayer *inventoryplayer = entityplayer->inventory;
		if(slotIndex == -999)
		{
			if(inventoryplayer->getItemStack() != nullptr && slotIndex == -999)
			{
				if(mouseButton == 0)
				{
					entityplayer->dropPlayerItem(inventoryplayer->getItemStack());
					inventoryplayer->setItemStack(nullptr);
				}
				if(mouseButton == 1)
				{
					entityplayer->dropPlayerItem(inventoryplayer->getItemStack()->splitStack(1));
					if(inventoryplayer->getItemStack()->stackSize == 0)
					{
						inventoryplayer->setItemStack(nullptr);
					}
				}
			}
		}
		else if(shiftClick)
		{
			ItemStack *itemstack1 = getStackInSlot(slotIndex);
			if(itemstack1 != nullptr)
			{
				int_t k = itemstack1->stackSize;
				itemstack = itemstack1->copy();
				Slot *slot1 = slots[slotIndex];
				if(slot1 != nullptr && slot1->getStack() != nullptr)
				{
					int_t l = slot1->getStack()->stackSize;
					if(l < k)
					{
						slotClick(slotIndex, mouseButton, shiftClick, entityplayer);
					}
				}
			}
		}
		else
		{
			Slot *slot = slots[slotIndex];
			if(slot != nullptr)
			{
				slot->onSlotChanged();
				ItemStack *itemstack2 = slot->getStack();
				ItemStack *itemstack3 = inventoryplayer->getItemStack();
				if(itemstack2 != nullptr)
				{
					itemstack = itemstack2->copy();
				}
				if(itemstack2 == nullptr)
				{
					if(itemstack3 != nullptr && slot->isItemValid(itemstack3))
					{
						int_t i1 = mouseButton != 0 ? 1 : itemstack3->stackSize;
						if(i1 > slot->getSlotStackLimit())
						{
							i1 = slot->getSlotStackLimit();
						}
						slot->putStack(itemstack3->splitStack(i1));
						if(itemstack3->stackSize == 0)
						{
							inventoryplayer->setItemStack(nullptr);
						}
					}
				}
				else if(itemstack3 == nullptr)
				{
					int_t j1 = mouseButton != 0 ? (itemstack2->stackSize + 1) / 2 : itemstack2->stackSize;
					ItemStack *itemstack5 = slot->decrStackSize(j1);
					inventoryplayer->setItemStack(itemstack5);
					if(itemstack2->stackSize == 0)
					{
						slot->putStack(nullptr);
					}
					slot->onPickupFromSlot(inventoryplayer->getItemStack());
				}
				else if(slot->isItemValid(itemstack3))
				{
					if(itemstack2->itemID != itemstack3->itemID || itemstack2->getHasSubtypes() && itemstack2->getItemDamage() != itemstack3->getItemDamage())
					{
						if(itemstack3->stackSize <= slot->getSlotStackLimit())
						{
							ItemStack *itemstack4 = itemstack2;
							slot->putStack(itemstack3);
							inventoryplayer->setItemStack(itemstack4);
						}
					}
					else
					{
						int_t k1 = mouseButton != 0 ? 1 : itemstack3->stackSize;
						if(k1 > slot->getSlotStackLimit() - itemstack2->stackSize)
						{
							k1 = slot->getSlotStackLimit() - itemstack2->stackSize;
						}
						if(k1 > itemstack3->getMaxStackSize() - itemstack2->stackSize)
						{
							k1 = itemstack3->getMaxStackSize() - itemstack2->stackSize;
						}
						itemstack3->splitStack(k1);
						if(itemstack3->stackSize == 0)
						{
							inventoryplayer->setItemStack(nullptr);
						}
						itemstack2->stackSize += k1;
					}
				}
				else if(itemstack2->itemID == itemstack3->itemID && itemstack3->getMaxStackSize() > 1 && (!itemstack2->getHasSubtypes() || itemstack2->getItemDamage() == itemstack3->getItemDamage()))
				{
					int_t l1 = itemstack2->stackSize;
					if(l1 > 0 && l1 + itemstack3->stackSize <= itemstack3->getMaxStackSize())
					{
						itemstack3->stackSize += l1;
						itemstack2->splitStack(l1);
						if(itemstack2->stackSize == 0)
						{
							slot->putStack(nullptr);
						}
						slot->onPickupFromSlot(inventoryplayer->getItemStack());
					}
				}
			}
		}
	}
	return itemstack;
}

void Container::onCraftGuiClosed(EntityPlayer *entityplayer)
{
	InventoryPlayer *inventoryplayer = entityplayer->inventory;
	if(inventoryplayer->getItemStack() != nullptr)
	{
		entityplayer->dropPlayerItem(inventoryplayer->getItemStack());
		inventoryplayer->setItemStack(nullptr);
	}
}

void Container::onCraftMatrixChanged(IInventory *iinventory)
{
	updateCraftingResults();
}

void Container::putStackInSlot(int_t i, ItemStack *itemstack)
{
	getSlot(i)->putStack(itemstack);
}

void Container::putStacksInSlots(ItemStack **aitemstack, int_t length)
{
	for(int_t i = 0; i < length; i++)
	{
		getSlot(i)->putStack(aitemstack[i]);
	}
}

void Container::updateProgressBar(int_t id, int_t value)
{
}

short_t Container::getNextTransactionID(InventoryPlayer *inventoryplayer)
{
	transactionID++;
	return transactionID;
}

void Container::updateTransaction(short_t transactionID)
{
}

void Container::onTransactionResponse(short_t transactionID)
{
}

void Container::mergeItemStack(ItemStack *itemstack, int_t startSlot, int_t endSlot, bool_t lastToFirst)
{
	int_t k = startSlot;
	if(lastToFirst)
	{
		k = endSlot - 1;
	}
	if(itemstack->isStackable())
	{
		while(itemstack->stackSize > 0 && (!lastToFirst && k < endSlot || lastToFirst && k >= startSlot))
		{
			Slot *slot = slots[k];
			ItemStack *itemstack1 = slot->getStack();
			if(itemstack1 != nullptr && itemstack1->itemID == itemstack->itemID && (!itemstack->getHasSubtypes() || itemstack->getItemDamage() == itemstack1->getItemDamage()))
			{
				int_t i1 = itemstack1->stackSize + itemstack->stackSize;
				if(i1 <= itemstack->getMaxStackSize())
				{
					itemstack->stackSize = 0;
					itemstack1->stackSize = i1;
					slot->onSlotChanged();
				}
				else if(itemstack1->stackSize < itemstack->getMaxStackSize())
				{
					itemstack->stackSize -= itemstack->getMaxStackSize() - itemstack1->stackSize;
					itemstack1->stackSize = itemstack->getMaxStackSize();
					slot->onSlotChanged();
				}
			}
			if(lastToFirst)
			{
				k--;
			}
			else
			{
				k++;
			}
		}
	}
	if(itemstack->stackSize > 0)
	{
		int_t l;
		if(lastToFirst)
		{
			l = endSlot - 1;
		}
		else
		{
			l = startSlot;
		}
		do
		{
			if((lastToFirst || l >= endSlot) && (!lastToFirst || l < startSlot))
			{
				break;
			}
			Slot *slot1 = slots[l];
			ItemStack *itemstack2 = slot1->getStack();
			if(itemstack2 == nullptr)
			{
				slot1->putStack(itemstack->copy());
				slot1->onSlotChanged();
				itemstack->stackSize = 0;
				break;
			}
			if(lastToFirst)
			{
				l--;
			}
			else
			{
				l++;
			}
		} while(true);
	}
}
