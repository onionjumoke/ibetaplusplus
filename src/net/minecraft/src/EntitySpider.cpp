#include "EntitySpider.h"

#include <cmath>

#include "EntityPlayer.h"
#include "Item.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "World.h"

EntitySpider::EntitySpider(World *world)
	: EntityMob(world)
{
	texture = String::fromUTF8("/mob/spider.png");
	setSize(1.4f, 0.9f);
	moveSpeed = 0.8f;
}

double EntitySpider::getMountedYOffset()
{
	return (double)height * 0.75 - 0.5;
}

bool EntitySpider::canTriggerWalking()
{
	return false;
}

Entity *EntitySpider::findPlayerToAttack()
{
	float f = getEntityBrightness(1.0f);
	if (f < 0.5f)
	{
		double d = 16.0;
		return worldObj->getClosestPlayerToEntity(this, d);
	}
	return nullptr;
}

jstring EntitySpider::getLivingSound() { return u"mob.spider"; }
jstring EntitySpider::getHurtSound()   { return u"mob.spider"; }
jstring EntitySpider::getDeathSound()  { return u"mob.spiderdeath"; }

void EntitySpider::attackEntity(Entity *entity, float f)
{
	float f1 = getEntityBrightness(1.0f);
	if (f1 > 0.5f && rand.nextInt(100) == 0)
	{
		playerToAttack = nullptr;
		return;
	}
	if (f > 2.0f && f < 6.0f && rand.nextInt(10) == 0)
	{
		if (onGround)
		{
			double d = entity->posX - posX;
			double d1 = entity->posZ - posZ;
			float f2 = MathHelper::sqrt_double(d * d + d1 * d1);
			motionX = (d / (double)f2) * 0.5 * 0.80000001192092896 + motionX * 0.20000000298023224;
			motionZ = (d1 / (double)f2) * 0.5 * 0.80000001192092896 + motionZ * 0.20000000298023224;
			motionY = 0.40000000596046448;
		}
	}
	else
	{
		EntityMob::attackEntity(entity, f);
	}
}

void EntitySpider::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	EntityMob::writeEntityToNBT(nbttagcompound);
}

void EntitySpider::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	EntityMob::readEntityFromNBT(nbttagcompound);
}

int_t EntitySpider::getDropItemId()
{
	return Item::silk->shiftedIndex;
}

bool EntitySpider::isOnLadder()
{
	return isCollidedHorizontally;
}
