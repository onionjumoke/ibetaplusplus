#include "EntitySnowball.h"

#include <cmath>

#include "Block.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "MovingObjectPosition.h"
#include "NBTTagCompound.h"
#include "Vec3D.h"
#include "World.h"
#include "AxisAlignedBB.h"

EntitySnowball::EntitySnowball(World *world)
	: Entity(world)
{
	xTileSnowball = -1;
	yTileSnowball = -1;
	zTileSnowball = -1;
	inTileSnowball = 0;
	inGroundSnowball = false;
	shakeSnowball = 0;
	ticksInAirSnowball = 0;
	setSize(0.25f, 0.25f);
}

EntitySnowball::EntitySnowball(World *world, EntityLiving *entityliving)
	: Entity(world)
{
	xTileSnowball = -1;
	yTileSnowball = -1;
	zTileSnowball = -1;
	inTileSnowball = 0;
	inGroundSnowball = false;
	shakeSnowball = 0;
	ticksInAirSnowball = 0;
	thrower = entityliving;
	setSize(0.25f, 0.25f);
	setLocationAndAngles(entityliving->posX, entityliving->posY + (double)entityliving->getEyeHeight(), entityliving->posZ, entityliving->rotationYaw, entityliving->rotationPitch);
	posX -= MathHelper::cos((rotationYaw / 180.0f) * 3.141593f) * 0.16f;
	posY -= 0.10000000149011612;
	posZ -= MathHelper::sin((rotationYaw / 180.0f) * 3.141593f) * 0.16f;
	setPosition(posX, posY, posZ);
	yOffset = 0.0f;
	float f = 0.4f;
	motionX = -MathHelper::sin((rotationYaw / 180.0f) * 3.141593f) * MathHelper::cos((rotationPitch / 180.0f) * 3.141593f) * f;
	motionZ = MathHelper::cos((rotationYaw / 180.0f) * 3.141593f) * MathHelper::cos((rotationPitch / 180.0f) * 3.141593f) * f;
	motionY = -MathHelper::sin((rotationPitch / 180.0f) * 3.141593f) * f;
	setSnowballHeading(motionX, motionY, motionZ, 1.5f, 1.0f);
}

EntitySnowball::EntitySnowball(World *world, double d, double d1, double d2)
	: Entity(world)
{
	xTileSnowball = -1;
	yTileSnowball = -1;
	zTileSnowball = -1;
	inTileSnowball = 0;
	inGroundSnowball = false;
	shakeSnowball = 0;
	ticksInAirSnowball = 0;
	ticksInGroundSnowball = 0;
	setSize(0.25f, 0.25f);
	setPosition(d, d1, d2);
	yOffset = 0.0f;
}

void EntitySnowball::entityInit()
{
}

bool EntitySnowball::isInRangeToRenderDist(double d)
{
	double d1 = boundingBox->getAverageEdgeLength() * 4.0;
	d1 *= 64.0;
	return d < d1 * d1;
}

void EntitySnowball::setSnowballHeading(double d, double d1, double d2, float f, float f1)
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
	ticksInGroundSnowball = 0;
}

void EntitySnowball::setVelocity(double d, double d1, double d2)
{
	motionX = d;
	motionY = d1;
	motionZ = d2;
	if (prevRotationPitch == 0.0f && prevRotationYaw == 0.0f)
	{
		float f = MathHelper::sqrt_double(d * d + d2 * d2);
		prevRotationYaw = rotationYaw = (float)((std::atan2(d, d2) * 180.0) / 3.1415927410125732);
		prevRotationPitch = rotationPitch = (float)((std::atan2(d1, (double)f) * 180.0) / 3.1415927410125732);
	}
}

void EntitySnowball::onUpdate()
{
	lastTickPosX = posX;
	lastTickPosY = posY;
	lastTickPosZ = posZ;
	Entity::onUpdate();
	if (shakeSnowball > 0)
	{
		shakeSnowball--;
	}
	if (inGroundSnowball)
	{
		int i = worldObj->getBlockId(xTileSnowball, yTileSnowball, zTileSnowball);
		if (i != inTileSnowball)
		{
			inGroundSnowball = false;
			motionX *= (double)(rand.nextFloat() * 0.2f);
			motionY *= (double)(rand.nextFloat() * 0.2f);
			motionZ *= (double)(rand.nextFloat() * 0.2f);
			ticksInGroundSnowball = 0;
			ticksInAirSnowball = 0;
		}
		else
		{
			ticksInGroundSnowball++;
			if (ticksInGroundSnowball == 1200)
			{
				setEntityDead();
			}
			return;
		}
	}
	else
	{
		ticksInAirSnowball++;
	}
	Vec3D *vec3d = Vec3D::createVector(posX, posY, posZ);
	Vec3D *vec3d1 = Vec3D::createVector(posX + motionX, posY + motionY, posZ + motionZ);
	MovingObjectPosition *movingobjectposition = worldObj->rayTraceBlocks(vec3d, vec3d1);
	vec3d = Vec3D::createVector(posX, posY, posZ);
	vec3d1 = Vec3D::createVector(posX + motionX, posY + motionY, posZ + motionZ);
	if (movingobjectposition != nullptr)
	{
		vec3d1 = Vec3D::createVector(movingobjectposition->hitVec->xCoord, movingobjectposition->hitVec->yCoord, movingobjectposition->hitVec->zCoord);
	}
	if (!worldObj->multiplayerWorld)
	{
		Entity *entity = nullptr;
		std::vector<Entity *> list = worldObj->getEntitiesWithinAABBExcludingEntity(this, boundingBox->addCoord(motionX, motionY, motionZ)->expand(1.0, 1.0, 1.0));
		double d = 0.0;
		for (size_t l = 0; l < list.size(); l++)
		{
			Entity *entity1 = list[l];
			if (!entity1->canBeCollidedWith() || (entity1 == thrower && ticksInAirSnowball < 5))
			{
				continue;
			}
			float f4 = 0.3f;
			AxisAlignedBB *axisalignedbb = entity1->boundingBox->expand(f4, f4, f4);
			MovingObjectPosition *movingobjectposition1 = axisalignedbb->calculateIntercept(vec3d, vec3d1);
			if (movingobjectposition1 == nullptr)
			{
				continue;
			}
			double d1 = vec3d->distanceTo(movingobjectposition1->hitVec);
			if (d1 < d || d == 0.0)
			{
				entity = entity1;
				d = d1;
			}
		}
		if (entity != nullptr)
		{
			movingobjectposition = new MovingObjectPosition(entity);
		}
	}
	if (movingobjectposition != nullptr)
	{
		if (movingobjectposition->entityHit != nullptr)
		{
			if (!movingobjectposition->entityHit->attackEntityFrom(thrower, 0));
		}
		for (int j = 0; j < 8; j++)
		{
			worldObj->spawnParticle(u"snowballpoof", posX, posY, posZ, 0.0, 0.0, 0.0);
		}
		setEntityDead();
	}
	posX += motionX;
	posY += motionY;
	posZ += motionZ;
	float f = MathHelper::sqrt_double(motionX * motionX + motionZ * motionZ);
	rotationYaw = (float)((std::atan2(motionX, motionZ) * 180.0) / 3.1415927410125732);
	for (rotationPitch = (float)((std::atan2(motionY, (double)f) * 180.0) / 3.1415927410125732); rotationPitch - prevRotationPitch < -180.0f; prevRotationPitch -= 360.0f) {}
	for (; rotationPitch - prevRotationPitch >= 180.0f; prevRotationPitch += 360.0f) {}
	for (; rotationYaw - prevRotationYaw < -180.0f; prevRotationYaw -= 360.0f) {}
	for (; rotationYaw - prevRotationYaw >= 180.0f; prevRotationYaw += 360.0f) {}
	rotationPitch = prevRotationPitch + (rotationPitch - prevRotationPitch) * 0.2f;
	rotationYaw = prevRotationYaw + (rotationYaw - prevRotationYaw) * 0.2f;
	float f1 = 0.99f;
	float f2 = 0.03f;
	if (isInWater())
	{
		for (int k = 0; k < 4; k++)
		{
			float f3 = 0.25f;
			worldObj->spawnParticle(u"bubble", posX - motionX * (double)f3, posY - motionY * (double)f3, posZ - motionZ * (double)f3, motionX, motionY, motionZ);
		}
		f1 = 0.8f;
	}
	motionX *= (double)f1;
	motionY *= (double)f1;
	motionZ *= (double)f1;
	motionY -= (double)f2;
	setPosition(posX, posY, posZ);
}

void EntitySnowball::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	nbttagcompound->setShort(u"xTile", (short)xTileSnowball);
	nbttagcompound->setShort(u"yTile", (short)yTileSnowball);
	nbttagcompound->setShort(u"zTile", (short)zTileSnowball);
	nbttagcompound->setByte(u"inTile", (byte_t)inTileSnowball);
	nbttagcompound->setByte(u"shake", (byte_t)shakeSnowball);
	nbttagcompound->setByte(u"inGround", (byte_t)(inGroundSnowball ? 1 : 0));
}

void EntitySnowball::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	xTileSnowball = nbttagcompound->getShort(u"xTile");
	yTileSnowball = nbttagcompound->getShort(u"yTile");
	zTileSnowball = nbttagcompound->getShort(u"zTile");
	inTileSnowball = nbttagcompound->getByte(u"inTile") & 0xFF;
	shakeSnowball = nbttagcompound->getByte(u"shake") & 0xFF;
	inGroundSnowball = nbttagcompound->getByte(u"inGround") == 1;
}

void EntitySnowball::onCollideWithPlayer(EntityPlayer *entityplayer)
{
	if (inGroundSnowball && thrower == entityplayer && ticksInGroundSnowball > 0 && entityplayer->inventory->addItemStackToInventory(new ItemStack(Item::snowball, 1)))
	{
		worldObj->playSoundAtEntity(this, u"random.pop", 0.2f, ((rand.nextFloat() - rand.nextFloat()) * 0.7f + 1.0f) * 2.0f);
		entityplayer->onItemPickup(this, 1);
		setEntityDead();
	}
}
