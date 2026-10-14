#include "EntityGhast.h"

#include <cmath>

#include "AxisAlignedBB.h"
#include "EntityFireball.h"
#include "EntityPlayer.h"
#include "Item.h"
#include "MathHelper.h"
#include "Vec3D.h"
#include "World.h"
#include "DataWatcher.h"

EntityGhast::EntityGhast(World *world)
	: EntityFlying(world)
{
	ensureEntityInit();
	courseChangeCooldown = 0;
	targetedEntity = nullptr;
	aggroCooldown = 0;
	prevAttackCounter = 0;
	attackCounter = 0;
	texture = String::fromUTF8("/mob/ghast.png");
	setSize(4.0f, 4.0f);
	isImmuneToFire = true;
}

void EntityGhast::entityInit()
{
	EntityFlying::entityInit();
	dataWatcher->addObject(16, (byte_t)0);
}

void EntityGhast::onUpdate()
{
	EntityFlying::onUpdate();
	byte_t byte0 = dataWatcher->getWatchableObjectByte(16);
	texture = byte0 != 1 ? String::fromUTF8("/mob/ghast.png") : String::fromUTF8("/mob/ghast_fire.png");
}

void EntityGhast::updatePlayerActionState()
{
	if (!worldObj->multiplayerWorld && worldObj->difficultySetting == 0)
	{
		setEntityDead();
	}
	func_27021_X();
	prevAttackCounter = attackCounter;
	double d = waypointX - posX;
	double d1 = waypointY - posY;
	double d2 = waypointZ - posZ;
	double d3 = MathHelper::sqrt_double(d * d + d1 * d1 + d2 * d2);
	if (d3 < 1.0 || d3 > 60.0)
	{
		waypointX = posX + (double)((rand.nextFloat() * 2.0f - 1.0f) * 16.0f);
		waypointY = posY + (double)((rand.nextFloat() * 2.0f - 1.0f) * 16.0f);
		waypointZ = posZ + (double)((rand.nextFloat() * 2.0f - 1.0f) * 16.0f);
	}
	if (courseChangeCooldown-- <= 0)
	{
		courseChangeCooldown += rand.nextInt(5) + 2;
		if (isCourseTraversable(waypointX, waypointY, waypointZ, d3))
		{
			motionX += (d / d3) * 0.10000000000000001;
			motionY += (d1 / d3) * 0.10000000000000001;
			motionZ += (d2 / d3) * 0.10000000000000001;
		}
		else
		{
			waypointX = posX;
			waypointY = posY;
			waypointZ = posZ;
		}
	}
	if (targetedEntity != nullptr && targetedEntity->isDead)
	{
		targetedEntity = nullptr;
	}
	if (targetedEntity == nullptr || aggroCooldown-- <= 0)
	{
		targetedEntity = worldObj->getClosestPlayerToEntity(this, 100.0);
		if (targetedEntity != nullptr)
		{
			aggroCooldown = 20;
		}
	}
	double d4 = 64.0;
	if (targetedEntity != nullptr && targetedEntity->getDistanceSqToEntity(this) < d4 * d4)
	{
		double d5 = targetedEntity->posX - posX;
		double d6 = (targetedEntity->boundingBox->minY + (double)(targetedEntity->height / 2.0f)) - (posY + (double)(height / 2.0f));
		double d7 = targetedEntity->posZ - posZ;
		renderYawOffset = rotationYaw = (float)((-(std::atan2(d5, d7) * 180.0)) / 3.141593);
		if (canEntityBeSeen(targetedEntity))
		{
			if (attackCounter == 10)
			{
				worldObj->playSoundAtEntity(this, u"mob.ghast.charge", getSoundVolume(), (rand.nextFloat() - rand.nextFloat()) * 0.2f + 1.0f);
			}
			attackCounter++;
			if (attackCounter == 20)
			{
				worldObj->playSoundAtEntity(this, u"mob.ghast.fireball", getSoundVolume(), (rand.nextFloat() - rand.nextFloat()) * 0.2f + 1.0f);
				EntityFireball *entityfireball = new EntityFireball(worldObj, this, d5, d6, d7);
				double d8 = 4.0;
				Vec3D *vec3d = getLook(1.0f);
				entityfireball->posX = posX + vec3d->xCoord * d8;
				entityfireball->posY = posY + (double)(height / 2.0f) + 0.5;
				entityfireball->posZ = posZ + vec3d->zCoord * d8;
				worldObj->entityJoinedWorld(entityfireball);
				attackCounter = -40;
			}
		}
		else if (attackCounter > 0)
		{
			attackCounter--;
		}
	}
	else
	{
		renderYawOffset = rotationYaw = (float)((-(std::atan2(motionX, motionZ) * 180.0)) / 3.141593);
		if (attackCounter > 0)
		{
			attackCounter--;
		}
	}
	if (!worldObj->multiplayerWorld)
	{
		byte_t byte0 = dataWatcher->getWatchableObjectByte(16);
		byte_t byte1 = (byte_t)(attackCounter <= 10 ? 0 : 1);
		if (byte0 != byte1)
		{
			dataWatcher->updateObject(16, byte1);
		}
	}
}

bool EntityGhast::isCourseTraversable(double d, double d1, double d2, double d3)
{
	double d4 = (waypointX - posX) / d3;
	double d5 = (waypointY - posY) / d3;
	double d6 = (waypointZ - posZ) / d3;
	AxisAlignedBB *axisalignedbb = boundingBox->copy();
	for (int i = 1; (double)i < d3; i++)
	{
		axisalignedbb->offset(d4, d5, d6);
		if (worldObj->getCollidingBoundingBoxes(this, axisalignedbb).size() > 0)
		{
			return false;
		}
	}
	return true;
}

jstring EntityGhast::getLivingSound() { return u"mob.ghast.moan"; }
jstring EntityGhast::getHurtSound()   { return u"mob.ghast.scream"; }
jstring EntityGhast::getDeathSound()  { return u"mob.ghast.death"; }

int_t EntityGhast::getDropItemId()
{
	return Item::gunpowder->shiftedIndex;
}

float EntityGhast::getSoundVolume()
{
	return 10.0f;
}

bool EntityGhast::getCanSpawnHere()
{
	return rand.nextInt(20) == 0 && EntityFlying::getCanSpawnHere() && worldObj->difficultySetting > 0;
}

int_t EntityGhast::getMaxSpawnedInChunk()
{
	return 1;
}
