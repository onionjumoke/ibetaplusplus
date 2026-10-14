#include "ContainerPlayer.h"

#include "InventoryCrafting.h"
#include "InventoryCraftResult.h"
#include "SlotCrafting.h"
#include "InventoryPlayer.h"
#include "Slot.h"
#include "SlotArmor.h"
#include "CraftingManager.h"
#include "IInventory.h"
#include "EntityPlayer.h"
#include "ItemStack.h"

ContainerPlayer::ContainerPlayer(InventoryPlayer *inventoryplayer) :
	ContainerPlayer(inventoryplayer, true)
{
}

ContainerPlayer::ContainerPlayer(InventoryPlayer *inventoryplayer, bool_t flag)
{
	craftMatrix = new InventoryCrafting(this, 2, 2);
	craftResult = new InventoryCraftResult();
	isSinglePlayer = false;
	isSinglePlayer = flag;
	addSlot(new SlotCrafting(inventoryplayer->player, craftMatrix, craftResult, 0, 144, 36));
	for(int_t i = 0; i < 2; i++)
	{
		for(int_t i1 = 0; i1 < 2; i1++)
		{
			addSlot(new Slot(craftMatrix, i1 + i * 2, 88 + i1 * 18, 26 + i * 18));
		}
	}
	for(int_t j = 0; j < 4; j++)
	{
		int_t j1 = j;
		addSlot(new SlotArmor(this, inventoryplayer, inventoryplayer->getSizeInventory() - 1 - j, 8, 8 + j * 18, j1));
	}
	for(int_t k = 0; k < 3; k++)
	{
		for(int_t k1 = 0; k1 < 9; k1++)
		{
			addSlot(new Slot(inventoryplayer, k1 + (k + 1) * 9, 8 + k1 * 18, 84 + k * 18));
		}
	}
	for(int_t l = 0; l < 9; l++)
	{
		addSlot(new Slot(inventoryplayer, l, 8 + l * 18, 142));
	}
	onCraftMatrixChanged(craftMatrix);
}

ContainerPlayer::~ContainerPlayer()
{
	delete craftMatrix;
	delete craftResult;
}

void ContainerPlayer::onCraftMatrixChanged(IInventory *iinventory)
{
	craftResult->setInventorySlotContents(0, CraftingManager::getInstance()->findMatchingRecipe(craftMatrix));
}

void ContainerPlayer::onCraftGuiClosed(EntityPlayer *entityplayer)
{
	Container::onCraftGuiClosed(entityplayer);
	for(int_t i = 0; i < 4; i++)
	{
		ItemStack *itemstack = craftMatrix->getStackInSlot(i);
		if(itemstack != nullptr)
		{
			entityplayer->dropPlayerItem(itemstack);
			craftMatrix->setInventorySlotContents(i, nullptr);
		}
	}
}

bool ContainerPlayer::isUsableByPlayer(EntityPlayer *entityplayer)
{
	return true;
}

ItemStack *ContainerPlayer::getStackInSlot(int_t i)
{
	ItemStack *itemstack = nullptr;
	Slot *slot = slots[i];
	if(slot != nullptr && slot->getHasStack())
	{
		ItemStack *itemstack1 = slot->getStack();
		itemstack = itemstack1->copy();
		if(i == 0)
		{
			mergeItemStack(itemstack1, 9, 45, true);
		}
		else if(i >= 9 && i < 36)
		{
			mergeItemStack(itemstack1, 36, 45, false);
		}
		else if(i >= 36 && i < 45)
		{
			mergeItemStack(itemstack1, 9, 36, false);
		}
		else
		{
			mergeItemStack(itemstack1, 9, 45, false);
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
