#include "SlotCrafting.h"

#include "EntityPlayer.h"
#include "IInventory.h"
#include "ItemStack.h"
#include "Block.h"
#include "Item.h"
#include "AchievementList.h"

SlotCrafting::SlotCrafting(EntityPlayer *entityplayer, IInventory *iinventory, IInventory *iinventory1, int_t i, int_t j, int_t k) :
	Slot(iinventory1, i, j, k),
	thePlayer(entityplayer),
	craftMatrix(iinventory)
{
}

bool SlotCrafting::isItemValid(ItemStack *itemstack)
{
	return false;
}

void SlotCrafting::onPickupFromSlot(ItemStack *itemstack)
{
	itemstack->onCrafting(thePlayer->worldObj, thePlayer);
	if(itemstack->itemID == Block::workbench->blockID)
	{
		thePlayer->addStat(AchievementList::buildWorkBench, 1);
	} else
	if(itemstack->itemID == Item::pickaxeWood->shiftedIndex)
	{
		thePlayer->addStat(AchievementList::buildPickaxe, 1);
	} else
	if(itemstack->itemID == Block::stoneOvenIdle->blockID)
	{
		thePlayer->addStat(AchievementList::buildFurnace, 1);
	} else
	if(itemstack->itemID == Item::hoeWood->shiftedIndex)
	{
		thePlayer->addStat(AchievementList::buildHoe, 1);
	} else
	if(itemstack->itemID == Item::bread->shiftedIndex)
	{
		thePlayer->addStat(AchievementList::makeBread, 1);
	} else
	if(itemstack->itemID == Item::cake->shiftedIndex)
	{
		thePlayer->addStat(AchievementList::bakeCake, 1);
	} else
	if(itemstack->itemID == Item::pickaxeStone->shiftedIndex)
	{
		thePlayer->addStat(AchievementList::buildBetterPickaxe, 1);
	} else
	if(itemstack->itemID == Item::swordWood->shiftedIndex)
	{
		thePlayer->addStat(AchievementList::buildSword, 1);
	}
	for(int_t i = 0; i < craftMatrix->getSizeInventory(); i++)
	{
		ItemStack *itemstack1 = craftMatrix->getStackInSlot(i);
		if(itemstack1 == nullptr)
		{
			continue;
		}
		craftMatrix->decrStackSize(i, 1);
		if(itemstack1->getItem()->hasContainerItem())
		{
			craftMatrix->setInventorySlotContents(i, new ItemStack(itemstack1->getItem()->getContainerItem()));
		}
	}
}
