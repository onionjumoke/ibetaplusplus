#include "EntityEgg.h"

#include <cmath>

#include "Block.h"
#include "EntityChicken.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "MathHelper.h"
#include "MovingObjectPosition.h"
#include "NBTTagCompound.h"
#include "Vec3D.h"
#include "World.h"
#include "AxisAlignedBB.h"

EntityEgg::EntityEgg(World *world)
	: Entity(world)
{
	xTile = -1;
	yTile = -1;
	zTile = -1;
	inTile = 0;
	inGround = false;
	thrower = nullptr;
	ticksInGround = 0;
	ticksInAir = 0;
	setSize(0.25f, 0.25f);
}

EntityEgg::EntityEgg(World *world, EntityLiving *entityliving)
	: Entity(world)
{
	xTile = -1;
	yTile = -1;
	zTile = -1;
	inTile = 0;
	inGround = false;
	thrower = entityliving;
	ticksInGround = 0;
	ticksInAir = 0;
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
	setEggHeading(motionX, motionY, motionZ, 1.5f, 1.0f);
}

EntityEgg::EntityEgg(World *world, double d, double d1, double d2)
	: Entity(world)
{
	xTile = -1;
	yTile = -1;
	zTile = -1;
	inTile = 0;
	inGround = false;
	thrower = nullptr;
	ticksInGround = 0;
	ticksInAir = 0;
	setSize(0.25f, 0.25f);
	setPosition(d, d1, d2);
	yOffset = 0.0f;
}

void EntityEgg::entityInit()
{
}

void EntityEgg::setEggHeading(double d, double d1, double d2, float f, float f1)
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

void EntityEgg::onUpdate()
{
	lastTickPosX = posX;
	lastTickPosY = posY;
	lastTickPosZ = posZ;
	Entity::onUpdate();
	if (thrower == nullptr)
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
		if (!entity1->canBeCollidedWith() || (entity1 == thrower && ticksInAir < 5))
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
	if (movingobjectposition != nullptr)
	{
		if (movingobjectposition->entityHit != nullptr && movingobjectposition->entityHit->attackEntityFrom(thrower, 0))
		{
			;
		}
		if (!worldObj->multiplayerWorld && rand.nextInt(8) == 0)
		{
			byte_t byte0 = 1;
			if (rand.nextInt(32) == 0)
			{
				byte0 = 4;
			}
			for (int j = 0; j < byte0; j++)
			{
				EntityChicken *entitychicken = new EntityChicken(worldObj);
				entitychicken->setLocationAndAngles(posX, posY, posZ, rotationYaw, 0.0f);
				worldObj->entityJoinedWorld(entitychicken);
			}
		}
		for (int k = 0; k < 8; k++)
		{
			worldObj->spawnParticle(u"snowballpoof", posX, posY, posZ, 0.0, 0.0, 0.0);
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
	float f2 = 0.99f;
	float f3 = 0.03f;
	if (isInWater())
	{
		for (int i1 = 0; i1 < 4; i1++)
		{
			float f5 = 0.25f;
			worldObj->spawnParticle(u"bubble", posX - motionX * (double)f5, posY - motionY * (double)f5, posZ - motionZ * (double)f5, motionX, motionY, motionZ);
		}
		f2 = 0.8f;
	}
	motionX *= (double)f2;
	motionY *= (double)f2;
	motionZ *= (double)f2;
	motionY -= (double)f3;
	setPosition(posX, posY, posZ);
}

void EntityEgg::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	nbttagcompound->setShort(u"xTile", (short)xTile);
	nbttagcompound->setShort(u"yTile", (short)yTile);
	nbttagcompound->setShort(u"zTile", (short)zTile);
	nbttagcompound->setByte(u"inTile", (byte_t)inTile);
	nbttagcompound->setByte(u"shake", (byte_t)0);
	nbttagcompound->setByte(u"inGround", (byte_t)(inGround ? 1 : 0));
}

void EntityEgg::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	xTile = nbttagcompound->getShort(u"xTile");
	yTile = nbttagcompound->getShort(u"yTile");
	zTile = nbttagcompound->getShort(u"zTile");
	inTile = nbttagcompound->getByte(u"inTile") & 0xFF;
	inGround = nbttagcompound->getByte(u"inGround") == 1;
}
