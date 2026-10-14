#include "EntityCreeper.h"

#include "EntityLightningBolt.h"
#include "EntitySkeleton.h"
#include "Item.h"
#include "NBTTagCompound.h"
#include "World.h"
#include "DataWatcher.h"
#include "java/String.h"

EntityCreeper::EntityCreeper(World *world)
	: EntityMob(world)
	, timeSinceIgnited(0)
	, lastActiveTime(0)
{
	ensureEntityInit();
	texture = String::fromUTF8("/mob/creeper.png");
}

void EntityCreeper::entityInit()
{
	EntityMob::entityInit();
	dataWatcher->addObject(16, (byte_t)-1);
	dataWatcher->addObject(17, (byte_t)0);
}

void EntityCreeper::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	EntityMob::writeEntityToNBT(nbttagcompound);
	if (dataWatcher->getWatchableObjectByte(17) == 1)
	{
		nbttagcompound->setBoolean(u"powered", true);
	}
}

void EntityCreeper::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	EntityMob::readEntityFromNBT(nbttagcompound);
	dataWatcher->updateObject(17, (byte_t)(nbttagcompound->getBoolean(u"powered") ? 1 : 0));
}

void EntityCreeper::attackBlockedEntity(Entity *, float)
{
	if (worldObj->multiplayerWorld)
		return;
	if (timeSinceIgnited > 0)
	{
		setCreeperState(-1);
		timeSinceIgnited--;
		if (timeSinceIgnited < 0)
			timeSinceIgnited = 0;
	}
}

void EntityCreeper::onUpdate()
{
	lastActiveTime = timeSinceIgnited;
	if (worldObj->multiplayerWorld)
	{
		int_t i = getCreeperState();
		if (i > 0 && timeSinceIgnited == 0)
		{
			worldObj->playSoundAtEntity(this, u"random.fuse", 1.0f, 0.5f);
		}
		timeSinceIgnited += i;
		if (timeSinceIgnited < 0)
			timeSinceIgnited = 0;
		if (timeSinceIgnited >= 30)
			timeSinceIgnited = 30;
	}
	EntityMob::onUpdate();
	if (playerToAttack == nullptr && timeSinceIgnited > 0)
	{
		setCreeperState(-1);
		timeSinceIgnited--;
		if (timeSinceIgnited < 0)
			timeSinceIgnited = 0;
	}
}

jstring EntityCreeper::getHurtSound()   { return u"mob.creeper"; }
jstring EntityCreeper::getDeathSound()  { return u"mob.creeperdeath"; }

void EntityCreeper::onDeath(Entity *entity)
{
	EntityMob::onDeath(entity);
	if (dynamic_cast<EntitySkeleton *>(entity) != nullptr)
	{
		dropItem(Item::record13->shiftedIndex + rand.nextInt(2), 1);
	}
}

void EntityCreeper::attackEntity(Entity *, float f)
{
	if (worldObj->multiplayerWorld)
		return;
	int_t i = getCreeperState();
	if ((i <= 0 && f < 3.0f) || (i > 0 && f < 7.0f))
	{
		if (timeSinceIgnited == 0)
		{
			worldObj->playSoundAtEntity(this, u"random.fuse", 1.0f, 0.5f);
		}
		setCreeperState(1);
		timeSinceIgnited++;
		if (timeSinceIgnited >= 30)
		{
			if (getPowered())
				worldObj->createExplosion(this, posX, posY, posZ, 6.0f);
			else
				worldObj->createExplosion(this, posX, posY, posZ, 3.0f);
			setEntityDead();
		}
		hasAttacked = true;
	}
	else
	{
		setCreeperState(-1);
		timeSinceIgnited--;
		if (timeSinceIgnited < 0)
			timeSinceIgnited = 0;
	}
}

bool EntityCreeper::getPowered()
{
	return dataWatcher->getWatchableObjectByte(17) == 1;
}

float EntityCreeper::setCreeperFlashTime(float f)
{
	return ((float)lastActiveTime + (float)(timeSinceIgnited - lastActiveTime) * f) / 28.0f;
}

int_t EntityCreeper::getDropItemId()
{
	return Item::gunpowder->shiftedIndex;
}

int_t EntityCreeper::getCreeperState()
{
	return dataWatcher->getWatchableObjectByte(16);
}

void EntityCreeper::setCreeperState(int_t i)
{
	dataWatcher->updateObject(16, (byte_t)i);
}

void EntityCreeper::onStruckByLightning(EntityLightningBolt *entitylightningbolt)
{
	EntityMob::onStruckByLightning(entitylightningbolt);
	dataWatcher->updateObject(17, (byte_t)1);
}
