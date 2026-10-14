#include "EntitySkeleton.h"

#include <cmath>

#include "EntityArrow.h"
#include "Item.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "World.h"

ItemStack *EntitySkeleton::defaultHeldItem = nullptr;

EntitySkeleton::EntitySkeleton(World *world)
	: EntityMob(world)
{
	texture = String::fromUTF8("/mob/skeleton.png");
	if (defaultHeldItem == nullptr)
	{
		defaultHeldItem = new ItemStack(Item::bow, 1);
	}
}

void EntitySkeleton::onLivingUpdate()
{
	if (worldObj->isDaytime())
	{
		float f = getEntityBrightness(1.0f);
		if (f > 0.5f && worldObj->canBlockSeeTheSky(MathHelper::floor_double(posX), MathHelper::floor_double(posY), MathHelper::floor_double(posZ)) && rand.nextFloat() * 30.0f < (f - 0.4f) * 2.0f)
		{
			fire = 300;
		}
	}
	EntityMob::onLivingUpdate();
}

void EntitySkeleton::attackEntity(Entity *entity, float f)
{
	if (f < 10.0f)
	{
		double d = entity->posX - posX;
		double d1 = entity->posZ - posZ;
		if (attackTime == 0)
		{
			EntityArrow *entityarrow = new EntityArrow(worldObj, this);
			entityarrow->posY += 1.3999999761581421;
			double d2 = (entity->posY + (double)entity->getEyeHeight()) - 0.20000000298023224 - entityarrow->posY;
			float f1 = MathHelper::sqrt_double(d * d + d1 * d1) * 0.2f;
			worldObj->playSoundAtEntity(this, u"random.bow", 1.0f, 1.0f / (rand.nextFloat() * 0.4f + 0.8f));
			worldObj->entityJoinedWorld(entityarrow);
			entityarrow->setArrowHeading(d, d2 + (double)f1, d1, 0.6f, 12.0f);
			attackTime = 30;
		}
		rotationYaw = (float)((std::atan2(d1, d) * 180.0) / 3.1415927410125732) - 90.0f;
		hasAttacked = true;
	}
}

jstring EntitySkeleton::getLivingSound() { return u"mob.skeleton"; }
jstring EntitySkeleton::getHurtSound()   { return u"mob.skeletonhurt"; }
jstring EntitySkeleton::getDeathSound()  { return u"mob.skeletonhurt"; }

void EntitySkeleton::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	EntityMob::writeEntityToNBT(nbttagcompound);
}

void EntitySkeleton::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	EntityMob::readEntityFromNBT(nbttagcompound);
}

int_t EntitySkeleton::getDropItemId()
{
	return Item::arrow->shiftedIndex;
}

void EntitySkeleton::dropFewItems()
{
	int i = rand.nextInt(3);
	for (int j = 0; j < i; j++)
	{
		dropItem(Item::arrow->shiftedIndex, 1);
	}
	i = rand.nextInt(3);
	for (int k = 0; k < i; k++)
	{
		dropItem(Item::bone->shiftedIndex, 1);
	}
}

ItemStack *EntitySkeleton::getHeldItem()
{
	return defaultHeldItem;
}
