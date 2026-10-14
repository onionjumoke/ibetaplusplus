#include "EntityFish.h"

#include <cmath>

#include "Block.h"
#include "EntityItem.h"
#include "EntityPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "MovingObjectPosition.h"
#include "NBTTagCompound.h"
#include "Vec3D.h"
#include "World.h"
#include "AxisAlignedBB.h"
#include "StatList.h"

EntityFish::EntityFish(World *world)
	: Entity(world)
{
	xTile = -1;
	yTile = -1;
	zTile = -1;
	inTile = 0;
	inGround = false;
	angler = nullptr;
	bobber = nullptr;
	ticksInGround = 0;
	ticksInAir = 0;
	ticksCatchable = 0;
	fishPosRotationIncrements = 0;
	setSize(0.25f, 0.25f);
}

EntityFish::EntityFish(World *world, EntityPlayer *entityplayer)
	: Entity(world)
{
	xTile = -1;
	yTile = -1;
	zTile = -1;
	inTile = 0;
	inGround = false;
	angler = entityplayer;
	bobber = nullptr;
	angler->fishEntity = this;
	ticksInGround = 0;
	ticksInAir = 0;
	ticksCatchable = 0;
	fishPosRotationIncrements = 0;
	setSize(0.25f, 0.25f);
	setLocationAndAngles(entityplayer->posX, entityplayer->posY + 1.62 - (double)entityplayer->yOffset, entityplayer->posZ, entityplayer->rotationYaw, entityplayer->rotationPitch);
	posX -= MathHelper::cos((rotationYaw / 180.0f) * 3.141593f) * 0.16f;
	posY -= 0.10000000149011612;
	posZ -= MathHelper::sin((rotationYaw / 180.0f) * 3.141593f) * 0.16f;
	setPosition(posX, posY, posZ);
	yOffset = 0.0f;
	float f = 0.4f;
	motionX = -MathHelper::sin((rotationYaw / 180.0f) * 3.141593f) * MathHelper::cos((rotationPitch / 180.0f) * 3.141593f) * f;
	motionZ = MathHelper::cos((rotationYaw / 180.0f) * 3.141593f) * MathHelper::cos((rotationPitch / 180.0f) * 3.141593f) * f;
	motionY = -MathHelper::sin((rotationPitch / 180.0f) * 3.141593f) * f;
	setFishHeading(motionX, motionY, motionZ, 1.5f, 1.0f);
}

void EntityFish::entityInit()
{
}

void EntityFish::setFishHeading(double d, double d1, double d2, float f, float f1)
{
	float f2 = MathHelper::sqrt_double(d * d + d1 * d1 + d2 * d2);
	d /= (double)f2;
	d1 /= (double)f2;
	d2 /= (double)f2;
	d += rand.nextGaussian() * 0.0074999998323619366 * (double)f1;
	d1 += rand.nextGaussian() * 0.0074999998323619366 * (double)f1;
	d2 += rand.nextGaussian() * 0.0074999998323619366 * (double)f1;
	d *= (double)f;
	d1 *= (double)f;
	d2 *= (double)f;
	motionX = d;
	motionY = d1;
	motionZ = d2;
	float f3 = MathHelper::sqrt_double(d * d + d2 * d2);
	prevRotationYaw = rotationYaw = (float)((std::atan2(d, d2) * 180.0) / 3.1415927410125732);
	prevRotationPitch = rotationPitch = (float)((std::atan2(d1, (double)f3) * 180.0) / 3.1415927410125732);
}

void EntityFish::onUpdate()
{
	lastTickPosX = posX;
	lastTickPosY = posY;
	lastTickPosZ = posZ;
	Entity::onUpdate();
	if (fishPosRotationIncrements > 0)
	{
		double d = posX + (fishX - posX) / (double)fishPosRotationIncrements;
		double d1 = posY + (fishY - posY) / (double)fishPosRotationIncrements;
		double d2 = posZ + (fishZ - posZ) / (double)fishPosRotationIncrements;
		double d3;
		for (d3 = fishYaw - (double)rotationYaw; d3 < -180.0; d3 += 360.0) {}
		for (; d3 >= 180.0; d3 -= 360.0) {}
		rotationYaw = (float)((double)rotationYaw + d3 / (double)fishPosRotationIncrements);
		rotationPitch = (float)((double)rotationPitch + (fishPitch - (double)rotationPitch) / (double)fishPosRotationIncrements);
		fishPosRotationIncrements--;
		setPosition(d, d1, d2);
		setRotation(rotationYaw, rotationPitch);
		return;
	}
	if (angler == nullptr)
	{
		setEntityDead();
		return;
	}
	if (bobber != nullptr)
	{
		if (bobber->isDead)
		{
			bobber = nullptr;
		}
		else
		{
			posX = bobber->posX;
			posY = bobber->boundingBox->minY + (double)bobber->height * 0.8;
			posZ = bobber->posZ;
			return;
		}
	}
	if (ticksInGround > 0)
	{
		ticksInGround--;
		if (ticksInGround == 0)
		{
			setEntityDead();
		}
		return;
	}
	ticksInAir++;
	Vec3D *vec3d = Vec3D::createVector(posX, posY, posZ);
	Vec3D *vec3d1 = Vec3D::createVector(posX + motionX, posY + motionY, posZ + motionZ);
	MovingObjectPosition *movingobjectposition = worldObj->rayTraceBlocks(vec3d, vec3d1);
	vec3d = Vec3D::createVector(posX, posY, posZ);
	vec3d1 = Vec3D::createVector(posX + motionX, posY + motionY, posZ + motionZ);
	if (movingobjectposition != nullptr)
	{
		vec3d1 = Vec3D::createVector(movingobjectposition->hitVec->xCoord, movingobjectposition->hitVec->yCoord, movingobjectposition->hitVec->zCoord);
	}
	Entity *entity = nullptr;
	std::vector<Entity *> list = worldObj->getEntitiesWithinAABBExcludingEntity(this, boundingBox->addCoord(motionX, motionY, motionZ)->expand(1.0, 1.0, 1.0));
	double d = 0.0;
	for (size_t i = 0; i < list.size(); i++)
	{
		Entity *entity1 = list[i];
		if (entity1->canBeCollidedWith() && entity1 != angler)
		{
			float f = 0.3f;
			AxisAlignedBB *axisalignedbb = entity1->boundingBox->expand(f, f, f);
			MovingObjectPosition *movingobjectposition1 = axisalignedbb->calculateIntercept(vec3d, vec3d1);
			if (movingobjectposition1 != nullptr)
			{
				double d1 = vec3d->distanceTo(movingobjectposition1->hitVec);
				if (d1 < d || d == 0.0)
				{
					entity = entity1;
					d = d1;
				}
			}
		}
	}
	if (entity != nullptr)
	{
		movingobjectposition = new MovingObjectPosition(entity);
	}
	if (movingobjectposition != nullptr)
	{
		if (movingobjectposition->entityHit != nullptr)
		{
			if (movingobjectposition->entityHit->attackEntityFrom(angler, 0))
			{
				bobber = movingobjectposition->entityHit;
			}
		}
		else
		{
			xTile = movingobjectposition->blockX;
			yTile = movingobjectposition->blockY;
			zTile = movingobjectposition->blockZ;
			inTile = worldObj->getBlockId(xTile, yTile, zTile);
			motionX = (float)(movingobjectposition->hitVec->xCoord - posX);
			motionY = (float)(movingobjectposition->hitVec->yCoord - posY);
			motionZ = (float)(movingobjectposition->hitVec->zCoord - posZ);
			float f1 = MathHelper::sqrt_double(motionX * motionX + motionY * motionY + motionZ * motionZ);
			posX -= (motionX / (double)f1) * 0.05000000074505806;
			posY -= (motionY / (double)f1) * 0.05000000074505806;
			posZ -= (motionZ / (double)f1) * 0.05000000074505806;
			inGround = true;
		}
	}
	if (!inGround)
	{
		posX += motionX;
		posY += motionY;
		posZ += motionZ;
		float f2 = MathHelper::sqrt_double(motionX * motionX + motionZ * motionZ);
		rotationYaw = (float)((std::atan2(motionX, motionZ) * 180.0) / 3.1415927410125732);
		for (rotationPitch = (float)((std::atan2(motionY, (double)f2) * 180.0) / 3.1415927410125732); rotationPitch - prevRotationPitch < -180.0f; prevRotationPitch -= 360.0f) {}
		for (; rotationPitch - prevRotationPitch >= 180.0f; prevRotationPitch += 360.0f) {}
		for (; rotationYaw - prevRotationYaw < -180.0f; prevRotationYaw -= 360.0f) {}
		for (; rotationYaw - prevRotationYaw >= 180.0f; prevRotationYaw += 360.0f) {}
		rotationPitch = prevRotationPitch + (rotationPitch - prevRotationPitch) * 0.2f;
		rotationYaw = prevRotationYaw + (rotationYaw - prevRotationYaw) * 0.2f;
		float f3 = 0.92f;
		if (onGround || isCollidedHorizontally)
			f3 = 0.5f;
		float f4 = 0.0f;
		if (ticksCatchable > 0)
		{
			f4 = (float)(ticksCatchable - 1) / 20.0f;
			f3 = f3 * (1.0f - f4) + f4 * 0.7f;
		}
		motionX *= (double)f3;
		motionY *= (double)f3;
		motionZ *= (double)f3;
		motionY -= 0.03;
		if (ticksCatchable > 0)
		{
			motionY *= 0.2;
			motionX *= 0.2;
			motionZ *= 0.2;
		}
		setPosition(posX, posY, posZ);
	}
}

void EntityFish::handleHealthUpdate(byte_t byte0)
{
	if (byte0 == 31)
		catchFish();
}

int_t EntityFish::catchFish()
{
	int_t rodDamage = 0;
	if (bobber != nullptr)
	{
		double dx = angler->posX - posX;
		double dy = angler->posY - posY;
		double dz = angler->posZ - posZ;
		double distance = MathHelper::sqrt_double(dx * dx + dy * dy + dz * dz);
		bobber->motionX += dx * 0.1;
		bobber->motionY += dy * 0.1 + (double)MathHelper::sqrt_double(distance) * 0.08;
		bobber->motionZ += dz * 0.1;
		rodDamage = 3;
	}
	else if (ticksCatchable > 0)
	{
		EntityItem *caughtFish = new EntityItem(worldObj, posX, posY, posZ, new ItemStack(Item::fishRaw));
		double dx = angler->posX - posX;
		double dy = angler->posY - posY;
		double dz = angler->posZ - posZ;
		double distance = MathHelper::sqrt_double(dx * dx + dy * dy + dz * dz);
		caughtFish->motionX = dx * 0.1;
		caughtFish->motionY = dy * 0.1 + (double)MathHelper::sqrt_double(distance) * 0.08;
		caughtFish->motionZ = dz * 0.1;
		worldObj->entityJoinedWorld(caughtFish);
		angler->addStat(StatList::fishCaughtStat, 1);
		rodDamage = 1;
	}
	if (inGround)
	{
		rodDamage = 2;
	}
	setEntityDead();
	angler->fishEntity = nullptr;
	return rodDamage;
}

void EntityFish::setPositionAndRotation2(double d, double d1, double d2, float f, float f1, int_t i)
{
	fishX = d;
	fishY = d1;
	fishZ = d2;
	fishYaw = f;
	fishPitch = f1;
	fishPosRotationIncrements = i;
	motionX = velocityX;
	motionY = velocityY;
	motionZ = velocityZ;
}

void EntityFish::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
}

void EntityFish::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
}

bool EntityFish::canBeCollidedWith()
{
	return false;
}

float EntityFish::getShadowSize()
{
	return 0.0f;
}
