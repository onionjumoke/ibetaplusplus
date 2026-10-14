#include "EntityPig.h"

#include "AchievementList.h"
#include "DataWatcher.h"
#include "EntityPigZombie.h"
#include "EntityPlayer.h"
#include "Item.h"
#include "NBTTagCompound.h"
#include "World.h"

EntityPig::EntityPig(World *world)
	: EntityAnimal(world)
{
	ensureEntityInit();
	texture = String::fromUTF8("/mob/pig.png");
	setSize(0.9f, 0.9f);
}

void EntityPig::entityInit()
{
	dataWatcher->addObject(16, (byte_t)0);
}

void EntityPig::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	EntityAnimal::writeEntityToNBT(nbttagcompound);
	nbttagcompound->setBoolean(u"Saddle", getSaddled());
}

void EntityPig::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	EntityAnimal::readEntityFromNBT(nbttagcompound);
	setSaddled(nbttagcompound->getBoolean(u"Saddle"));
}

jstring EntityPig::getLivingSound() { return u"mob.pig"; }
jstring EntityPig::getHurtSound()   { return u"mob.pig"; }
jstring EntityPig::getDeathSound()  { return u"mob.pigdeath"; }

bool EntityPig::interact(EntityPlayer *entityplayer)
{
	if (getSaddled() && !worldObj->multiplayerWorld && (riddenByEntity == nullptr || riddenByEntity == entityplayer))
	{
		entityplayer->mountEntity(this);
		return true;
	}
	return false;
}

int_t EntityPig::getDropItemId()
{
	if (fire > 0)
	{
		return Item::porkCooked->shiftedIndex;
	}
	return Item::porkRaw->shiftedIndex;
}

bool EntityPig::getSaddled()
{
	return (dataWatcher->getWatchableObjectByte(16) & 1) != 0;
}

void EntityPig::setSaddled(bool flag)
{
	dataWatcher->updateObject(16, (byte_t)(flag ? 1 : 0));
}

void EntityPig::onStruckByLightning(EntityLightningBolt *entitylightningbolt)
{
	if (worldObj->multiplayerWorld)
	{
		return;
	}
	EntityPigZombie *entitypigzombie = new EntityPigZombie(worldObj);
	entitypigzombie->setLocationAndAngles(posX, posY, posZ, rotationYaw, rotationPitch);
	worldObj->entityJoinedWorld(entitypigzombie);
	setEntityDead();
}

void EntityPig::fall(float f)
{
	EntityAnimal::fall(f);
	if (f > 5.0f && dynamic_cast<EntityPlayer *>(riddenByEntity) != nullptr)
	{
		((EntityPlayer *)riddenByEntity)->triggerAchievement(AchievementList::flyPig);
	}
}
