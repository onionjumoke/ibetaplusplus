#include "EntityFireball.h"

#include <cmath>

#include "Block.h"
#include "EntityLiving.h"
#include "MathHelper.h"
#include "MovingObjectPosition.h"
#include "NBTTagCompound.h"
#include "Vec3D.h"
#include "World.h"
#include "AxisAlignedBB.h"

EntityFireball::EntityFireball(World *world)
	: Entity(world)
{
	xTile = -1;
	yTile = -1;
	zTile = -1;
	inTile = 0;
	inGround = false;
	shootingEntity = nullptr;
	ticksInGround = 0;
	ticksInAir = 0;
	setSize(1.0f, 1.0f);
}

EntityFireball::EntityFireball(World *world, EntityLiving *entityliving, double d, double d1, double d2)
	: Entity(world)
{
	xTile = -1;
	yTile = -1;
	zTile = -1;
	inTile = 0;
	inGround = false;
	ticksInAir = 0;
	shootingEntity = entityliving;
	setSize(1.0f, 1.0f);
	setLocationAndAngles(entityliving->posX, entityliving->posY + (double)entityliving->getEyeHeight(), entityliving->posZ, entityliving->rotationYaw, entityliving->rotationPitch);
	setPosition(posX, posY, posZ);
	motionX = motionY = motionZ = 0.0;
	double d3 = MathHelper::sqrt_double(d * d + d1 * d1 + d2 * d2);
	accelerationX = (d / d3) * 0.1;
	accelerationY = (d1 / d3) * 0.1;
	accelerationZ = (d2 / d3) * 0.1;
}

EntityFireball::EntityFireball(World *world, double x, double y, double z, double ax, double ay, double az)
	: Entity(world)
{
	xTile = -1;
	yTile = -1;
	zTile = -1;
	inTile = 0;
	inGround = false;
	shootingEntity = nullptr;
	ticksInGround = 0;
	ticksInAir = 0;
	setSize(1.0f, 1.0f);
	setPosition(x, y, z);
	motionX = motionY = motionZ = 0.0;
	double len = MathHelper::sqrt_double(ax * ax + ay * ay + az * az);
	if (len > 0.0)
	{
		accelerationX = (ax / len) * 0.1;
		accelerationY = (ay / len) * 0.1;
		accelerationZ = (az / len) * 0.1;
	}
	else
	{
		accelerationX = accelerationY = accelerationZ = 0.0;
	}
}

void EntityFireball::entityInit()
{
}

void EntityFireball::onUpdate()
{
	lastTickPosX = posX;
	lastTickPosY = posY;
	lastTickPosZ = posZ;
	Entity::onUpdate();
	if (shootingEntity == nullptr)
	{
		setEntityDead();
		return;
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
		if (entity1->canBeCollidedWith() && entity1 != shootingEntity)
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
		if (!worldObj->multiplayerWorld)
		{
			if (movingobjectposition->entityHit != nullptr)
			{
				movingobjectposition->entityHit->attackEntityFrom(shootingEntity, 5);
			}
			worldObj->createExplosion(nullptr, posX, posY, posZ, 1.0f);
		}
		setEntityDead();
	}
	posX += motionX;
	posY += motionY;
	posZ += motionZ;
	float f1 = MathHelper::sqrt_double(motionX * motionX + motionZ * motionZ);
	rotationYaw = (float)((std::atan2(motionX, motionZ) * 180.0) / 3.1415927410125732);
	for (rotationPitch = (float)((std::atan2(motionY, (double)f1) * 180.0) / 3.1415927410125732); rotationPitch - prevRotationPitch < -180.0f; prevRotationPitch -= 360.0f) {}
	for (; rotationPitch - prevRotationPitch >= 180.0f; prevRotationPitch += 360.0f) {}
	for (; rotationYaw - prevRotationYaw < -180.0f; prevRotationYaw -= 360.0f) {}
	for (; rotationYaw - prevRotationYaw >= 180.0f; prevRotationYaw += 360.0f) {}
	rotationPitch = prevRotationPitch + (rotationPitch - prevRotationPitch) * 0.2f;
	rotationYaw = prevRotationYaw + (rotationYaw - prevRotationYaw) * 0.2f;
	float f2 = 0.95f;
	if (isInWater())
	{
		for (int j = 0; j < 4; j++)
		{
			float f3 = 0.25f;
			worldObj->spawnParticle(u"bubble", posX - motionX * (double)f3, posY - motionY * (double)f3, posZ - motionZ * (double)f3, motionX, motionY, motionZ);
		}
		f2 = 0.8f;
	}
	motionX += accelerationX;
	motionY += accelerationY;
	motionZ += accelerationZ;
	motionX *= (double)f2;
	motionY *= (double)f2;
	motionZ *= (double)f2;
	setPosition(posX, posY, posZ);
}

void EntityFireball::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	nbttagcompound->setShort(u"xTile", (short)xTile);
	nbttagcompound->setShort(u"yTile", (short)yTile);
	nbttagcompound->setShort(u"zTile", (short)zTile);
	nbttagcompound->setByte(u"inTile", (byte_t)inTile);
	nbttagcompound->setByte(u"shake", (byte_t)0);
	nbttagcompound->setByte(u"inGround", (byte_t)(inGround ? 1 : 0));
}

void EntityFireball::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	xTile = nbttagcompound->getShort(u"xTile");
	yTile = nbttagcompound->getShort(u"yTile");
	zTile = nbttagcompound->getShort(u"zTile");
	inTile = nbttagcompound->getByte(u"inTile") & 0xFF;
	inGround = nbttagcompound->getByte(u"inGround") == 1;
}
