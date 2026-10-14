#include "SlotArmor.h"

#include "ContainerPlayer.h"
#include "IInventory.h"
#include "ItemStack.h"
#include "ItemArmor.h"
#include "Item.h"
#include "Block.h"

SlotArmor::SlotArmor(ContainerPlayer *containerplayer, IInventory *iinventory, int_t i, int_t j, int_t k, int_t l) :
	Slot(iinventory, i, j, k),
	inventory(containerplayer),
	armorType(l)
{
}

int_t SlotArmor::getSlotStackLimit()
{
	return 1;
}

bool SlotArmor::isItemValid(ItemStack *itemstack)
{
	if(dynamic_cast<ItemArmor *>(itemstack->getItem()) != nullptr)
	{
		return ((ItemArmor *)itemstack->getItem())->armorType == armorType;
	}
	if(itemstack->getItem()->shiftedIndex == Block::pumpkin->blockID)
	{
		return armorType == 0;
	} else
	{
		return false;
	}
}
