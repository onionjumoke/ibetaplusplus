#include "EntityMob.h"

#include "AxisAlignedBB.h"
#include "EntityPlayer.h"
#include "EnumSkyBlock.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "World.h"

EntityMob::EntityMob(World *world)
	: EntityCreature(world)
{
	attackStrength = 2;
	health = 20;
}

void EntityMob::onLivingUpdate()
{
	float f = getEntityBrightness(1.0f);
	if (f > 0.5f)
	{
		entityAge += 2;
	}
	EntityCreature::onLivingUpdate();
}

void EntityMob::onUpdate()
{
	EntityCreature::onUpdate();
	if (!worldObj->multiplayerWorld && worldObj->difficultySetting == 0)
	{
		setEntityDead();
	}
}

Entity *EntityMob::findPlayerToAttack()
{
	EntityPlayer *entityplayer = worldObj->getClosestPlayerToEntity(this, 16.0);
	if (entityplayer != nullptr && canEntityBeSeen(entityplayer))
	{
		return entityplayer;
	}
	return nullptr;
}

bool EntityMob::attackEntityFrom(Entity *entity, int_t i)
{
	if (EntityCreature::attackEntityFrom(entity, i))
	{
		if (riddenByEntity == entity || ridingEntity == entity)
		{
			return true;
		}
		if (entity != this)
		{
			playerToAttack = entity;
		}
		return true;
	}
	return false;
}

void EntityMob::attackEntity(Entity *entity, float f)
{
	if (attackTime <= 0 && f < 2.0f && entity->boundingBox->maxY > boundingBox->minY && entity->boundingBox->minY < boundingBox->maxY)
	{
		attackTime = 20;
		entity->attackEntityFrom(this, attackStrength);
	}
}

float EntityMob::getBlockPathWeight(int_t i, int_t j, int_t k)
{
	return 0.5f - worldObj->getLightBrightness(i, j, k);
}

bool EntityMob::getCanSpawnHere()
{
	int_t i = MathHelper::floor_double(posX);
	int_t j = MathHelper::floor_double(boundingBox->minY);
	int_t k = MathHelper::floor_double(posZ);
	if (worldObj->getSavedLightValue(EnumSkyBlock::Sky, i, j, k) > rand.nextInt(32))
	{
		return false;
	}
	int_t l = worldObj->getBlockLightValue(i, j, k);
	if (worldObj->func_27160_B())
	{
		int_t i1 = worldObj->skylightSubtracted;
		worldObj->skylightSubtracted = 10;
		l = worldObj->getBlockLightValue(i, j, k);
		worldObj->skylightSubtracted = i1;
	}
	return l <= rand.nextInt(8) && EntityCreature::getCanSpawnHere();
}
