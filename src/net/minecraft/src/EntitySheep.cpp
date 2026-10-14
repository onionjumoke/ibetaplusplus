#include "EntitySheep.h"

#include "Block.h"
#include "DataWatcher.h"
#include "EntityItem.h"
#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "NBTTagCompound.h"
#include "World.h"

const float EntitySheep::fleeceColorTable[16][3] = {
	{1.00f, 1.00f, 1.00f},
	{0.95f, 0.70f, 0.20f},
	{0.90f, 0.50f, 0.85f},
	{0.60f, 0.70f, 0.95f},
	{0.90f, 0.90f, 0.20f},
	{0.50f, 0.80f, 0.10f},
	{0.95f, 0.70f, 0.80f},
	{0.30f, 0.30f, 0.30f},
	{0.60f, 0.60f, 0.60f},
	{0.30f, 0.60f, 0.70f},
	{0.70f, 0.40f, 0.90f},
	{0.20f, 0.40f, 0.80f},
	{0.50f, 0.40f, 0.30f},
	{0.40f, 0.50f, 0.20f},
	{0.80f, 0.30f, 0.30f},
	{0.10f, 0.10f, 0.10f},
};

EntitySheep::EntitySheep(World *world)
	: EntityAnimal(world)
{
	ensureEntityInit();
	texture = String::fromUTF8("/mob/sheep.png");
	setSize(0.9f, 1.3f);
}

void EntitySheep::entityInit()
{
	EntityAnimal::entityInit();
	dataWatcher->addObject(16, (byte_t)0);
}

void EntitySheep::dropFewItems()
{
	if (!getSheared())
	{
		entityDropItem(new ItemStack(((Block *)Block::cloth)->blockID, 1, getFleeceColor()), 0.0f);
	}
}

int_t EntitySheep::getDropItemId()
{
	return ((Block *)Block::cloth)->blockID;
}

bool EntitySheep::interact(EntityPlayer *entityplayer)
{
	ItemStack *itemstack = entityplayer->inventory->getCurrentItem();
	if (itemstack != nullptr && itemstack->itemID == Item::shears->shiftedIndex && !getSheared())
	{
		if (!worldObj->multiplayerWorld)
		{
			setSheared(true);
			int_t i = 2 + rand.nextInt(3);
			for (int_t j = 0; j < i; j++)
			{
				EntityItem *entityitem = entityDropItem(new ItemStack(((Block *)Block::cloth)->blockID, 1, getFleeceColor()), 1.0f);
				entityitem->motionY += (double)(rand.nextFloat() * 0.05f);
				entityitem->motionX += (double)((rand.nextFloat() - rand.nextFloat()) * 0.1f);
				entityitem->motionZ += (double)((rand.nextFloat() - rand.nextFloat()) * 0.1f);
			}
		}
		itemstack->damageItem(1, entityplayer);
	}
	return false;
}

void EntitySheep::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	EntityAnimal::writeEntityToNBT(nbttagcompound);
	nbttagcompound->setBoolean(u"Sheared", getSheared());
	nbttagcompound->setByte(u"Color", (byte_t)getFleeceColor());
}

void EntitySheep::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	EntityAnimal::readEntityFromNBT(nbttagcompound);
	setSheared(nbttagcompound->getBoolean(u"Sheared"));
	setFleeceColor(nbttagcompound->getByte(u"Color"));
}

jstring EntitySheep::getLivingSound() { return u"mob.sheep"; }
jstring EntitySheep::getHurtSound()   { return u"mob.sheep"; }
jstring EntitySheep::getDeathSound()  { return u"mob.sheep"; }

int_t EntitySheep::getFleeceColor()
{
	return dataWatcher->getWatchableObjectByte(16) & 0xf;
}

void EntitySheep::setFleeceColor(int_t i)
{
	byte_t byte0 = dataWatcher->getWatchableObjectByte(16);
	dataWatcher->updateObject(16, (byte_t)((byte0 & 0xf0) | (i & 0xf)));
}

bool EntitySheep::getSheared()
{
	return (dataWatcher->getWatchableObjectByte(16) & 0x10) != 0;
}

void EntitySheep::setSheared(bool flag)
{
	byte_t byte0 = dataWatcher->getWatchableObjectByte(16);
	if (flag)
	{
		dataWatcher->updateObject(16, (byte_t)(byte0 | 0x10));
	}
	else
	{
		dataWatcher->updateObject(16, (byte_t)(byte0 & 0xef));
	}
}

int_t EntitySheep::getRandomFleeceColor(Random &random)
{
	int_t i = random.nextInt(100);
	if (i < 5)  { return 15; }
	if (i < 10) { return  7; }
	if (i < 15) { return  8; }
	if (i < 18) { return 12; }
	return random.nextInt(500) != 0 ? 0 : 6;
}
