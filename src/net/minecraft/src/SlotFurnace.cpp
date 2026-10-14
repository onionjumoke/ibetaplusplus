#include "SlotFurnace.h"

#include "EntityPlayer.h"
#include "IInventory.h"
#include "ItemStack.h"
#include "Item.h"
#include "AchievementList.h"

SlotFurnace::SlotFurnace(EntityPlayer *entityplayer, IInventory *iinventory, int_t i, int_t j, int_t k) :
	Slot(iinventory, i, j, k),
	thePlayer(entityplayer)
{
}

bool SlotFurnace::isItemValid(ItemStack *itemstack)
{
	return false;
}

void SlotFurnace::onPickupFromSlot(ItemStack *itemstack)
{
	itemstack->onCrafting(thePlayer->worldObj, thePlayer);
	if(itemstack->itemID == Item::ingotIron->shiftedIndex)
	{
		thePlayer->addStat(AchievementList::acquireIron, 1);
	}
	if(itemstack->itemID == Item::fishCooked->shiftedIndex)
	{
		thePlayer->addStat(AchievementList::cookFish, 1);
	}
	Slot::onPickupFromSlot(itemstack);
}
