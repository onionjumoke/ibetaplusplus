#include "ContainerWorkbench.h"

#include "InventoryCrafting.h"
#include "InventoryCraftResult.h"
#include "SlotCrafting.h"
#include "InventoryPlayer.h"
#include "Slot.h"
#include "CraftingManager.h"
#include "IInventory.h"
#include "World.h"
#include "EntityPlayer.h"
#include "Block.h"
#include "ItemStack.h"

ContainerWorkbench::ContainerWorkbench(InventoryPlayer *inventoryplayer, World *world, int_t i, int_t j, int_t k)
{
	craftMatrix = new InventoryCrafting(this, 3, 3);
	craftResult = new InventoryCraftResult();
	this->world = world;
	posX = i;
	posY = j;
	posZ = k;
	addSlot(new SlotCrafting(inventoryplayer->player, craftMatrix, craftResult, 0, 124, 35));
	for(int_t l = 0; l < 3; l++)
	{
		for(int_t k1 = 0; k1 < 3; k1++)
		{
			addSlot(new Slot(craftMatrix, k1 + l * 3, 30 + k1 * 18, 17 + l * 18));
		}
	}
	for(int_t i1 = 0; i1 < 3; i1++)
	{
		for(int_t l1 = 0; l1 < 9; l1++)
		{
			addSlot(new Slot(inventoryplayer, l1 + i1 * 9 + 9, 8 + l1 * 18, 84 + i1 * 18));
		}
	}
	for(int_t j1 = 0; j1 < 9; j1++)
	{
		addSlot(new Slot(inventoryplayer, j1, 8 + j1 * 18, 142));
	}
	onCraftMatrixChanged(craftMatrix);
}

void ContainerWorkbench::onCraftMatrixChanged(IInventory *iinventory)
{
	craftResult->setInventorySlotContents(0, CraftingManager::getInstance()->findMatchingRecipe(craftMatrix));
}

void ContainerWorkbench::onCraftGuiClosed(EntityPlayer *entityplayer)
{
	Container::onCraftGuiClosed(entityplayer);
	if(world->multiplayerWorld)
	{
		return;
	}
	for(int_t i = 0; i < 9; i++)
	{
		ItemStack *itemstack = craftMatrix->getStackInSlot(i);
		if(itemstack != nullptr)
		{
			entityplayer->dropPlayerItem(itemstack);
		}
	}
}

bool ContainerWorkbench::isUsableByPlayer(EntityPlayer *entityplayer)
{
	if(world->getBlockId(posX, posY, posZ) != Block::workbench->blockID)
	{
		return false;
	}
	return entityplayer->getDistanceSq((double)posX + 0.5, (double)posY + 0.5, (double)posZ + 0.5) <= 64.0;
}

ItemStack *ContainerWorkbench::getStackInSlot(int_t i)
{
	ItemStack *itemstack = nullptr;
	Slot *slot = slots[i];
	if(slot != nullptr && slot->getHasStack())
	{
		ItemStack *itemstack1 = slot->getStack();
		itemstack = itemstack1->copy();
		if(i == 0)
		{
			mergeItemStack(itemstack1, 10, 46, true);
		}
		else if(i >= 10 && i < 37)
		{
			mergeItemStack(itemstack1, 37, 46, false);
		}
		else if(i >= 37 && i < 46)
		{
			mergeItemStack(itemstack1, 10, 37, false);
		}
		else
		{
			mergeItemStack(itemstack1, 10, 46, false);
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
