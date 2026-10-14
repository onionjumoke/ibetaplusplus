#include "EntityTNTPrimed.h"

#include <cmath>

#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "World.h"

EntityTNTPrimed::EntityTNTPrimed(World *world)
	: Entity(world)
{
	fuse = 0;
	preventEntitySpawning = true;
	setSize(0.98f, 0.98f);
	yOffset = height / 2.0f;
}

EntityTNTPrimed::EntityTNTPrimed(World *world, double d, double d1, double d2)
	: EntityTNTPrimed(world)
{
	setPosition(d, d1, d2);
	float f = (float)(rand.nextDouble() * 3.1415927410125732 * 2.0);
	motionX = -MathHelper::sin((f * 3.141593f) / 180.0f) * 0.02f;
	motionY = 0.20000000298023224;
	motionZ = -MathHelper::cos((f * 3.141593f) / 180.0f) * 0.02f;
	fuse = 80;
	prevPosX = d;
	prevPosY = d1;
	prevPosZ = d2;
}

void EntityTNTPrimed::entityInit()
{
}

bool EntityTNTPrimed::canTriggerWalking()
{
	return false;
}

bool EntityTNTPrimed::canBeCollidedWith()
{
	return !isDead;
}

void EntityTNTPrimed::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	motionY -= 0.039999999105930328;
	moveEntity(motionX, motionY, motionZ);
	motionX *= 0.98000001907348633;
	motionY *= 0.98000001907348633;
	motionZ *= 0.98000001907348633;
	if (onGround)
	{
		motionX *= 0.69999998807907104;
		motionZ *= 0.69999998807907104;
		motionY *= -0.5;
	}
	if (fuse-- <= 0)
	{
		if (!worldObj->multiplayerWorld)
		{
			setEntityDead();
			explode();
		}
		else
		{
			setEntityDead();
		}
	}
	else
	{
		worldObj->spawnParticle(u"smoke", posX, posY + 0.5, posZ, 0.0, 0.0, 0.0);
	}
}

void EntityTNTPrimed::explode()
{
	float f = 4.0f;
	worldObj->createExplosion(nullptr, posX, posY, posZ, f);
}

void EntityTNTPrimed::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	nbttagcompound->setByte(u"Fuse", (byte_t)fuse);
}

void EntityTNTPrimed::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	fuse = nbttagcompound->getByte(u"Fuse");
}

float EntityTNTPrimed::getShadowSize()
{
	return 0.0f;
}
