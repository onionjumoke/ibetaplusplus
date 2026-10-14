#include "EntityCow.h"

#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "World.h"

EntityCow::EntityCow(World *world)
	: EntityAnimal(world)
{
	texture = String::fromUTF8("/mob/cow.png");
	setSize(0.9f, 1.3f);
}

jstring EntityCow::getLivingSound() { return u"mob.cow"; }
jstring EntityCow::getHurtSound()   { return u"mob.cowhurt"; }
jstring EntityCow::getDeathSound()  { return u"mob.cowhurt"; }
float   EntityCow::getSoundVolume() { return 0.4f; }
int_t   EntityCow::getDropItemId()  { return Item::leather->shiftedIndex; }

bool EntityCow::interact(EntityPlayer *entityplayer)
{
	ItemStack *itemstack = entityplayer->inventory->getCurrentItem();
	if (itemstack != nullptr && itemstack->itemID == Item::bucketEmpty->shiftedIndex)
	{
		entityplayer->inventory->setInventorySlotContents(entityplayer->inventory->currentItem, new ItemStack(Item::bucketMilk));
		return true;
	}
	return false;
}
