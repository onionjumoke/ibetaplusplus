#include "EntityCreature.h"

#include <cmath>

#include "AxisAlignedBB.h"
#include "Entity.h"
#include "MathHelper.h"
#include "PathEntity.h"
#include "Vec3D.h"
#include "World.h"

EntityCreature::EntityCreature(World *world)
	: EntityLiving(world)
{
	pathToEntity = nullptr;
	playerToAttack = nullptr;
	hasAttacked = false;
}

EntityCreature::~EntityCreature() = default;

bool EntityCreature::isMovementCeased()
{
	return false;
}

void EntityCreature::updatePlayerActionState()
{
	hasAttacked = isMovementCeased();
	float f = 16.0f;
#if defined(__ARM_ARCH)
	// Touch build: the expensive part of mob "AI" is target scanning, A*
	// pathfinding and line-of-sight ray traces. Run it every other tick per
	// entity (staggered by id); movement keeps using the cached path.
	if (((ticksExisted + entityId) & 1) != 0)
		return;
#endif
	if (playerToAttack == nullptr)
	{
		playerToAttack = findPlayerToAttack();
		if (playerToAttack != nullptr)
		{
			pathToEntity.reset(worldObj->getPathToEntity(this, playerToAttack, f));
		}
	}
	else if (!playerToAttack->isEntityAlive())
	{
		playerToAttack = nullptr;
	}
	else
	{
		float f1 = playerToAttack->getDistanceToEntity(this);
		if (canEntityBeSeen(playerToAttack))
		{
			attackEntity(playerToAttack, f1);
		}
		else
		{
			attackBlockedEntity(playerToAttack, f1);
		}
	}
	if (!hasAttacked && playerToAttack != nullptr && (pathToEntity == nullptr || rand.nextInt(20) == 0))
	{
		pathToEntity.reset(worldObj->getPathToEntity(this, playerToAttack, f));
	}
	else if (!hasAttacked && ((pathToEntity == nullptr && rand.nextInt(80) == 0) || rand.nextInt(80) == 0))
	{
		func_31026_E();
	}
	int_t i = MathHelper::floor_double(boundingBox->minY + 0.5);
	bool flag  = isInWater();
	bool flag1 = handleLavaMovement();
	rotationPitch = 0.0f;
	if (pathToEntity == nullptr || rand.nextInt(100) == 0)
	{
		EntityLiving::updatePlayerActionState();
		pathToEntity = nullptr;
		return;
	}
	Vec3D *vec3d = pathToEntity->getPosition(this);
	for (double d = (double)width * 2.0; vec3d != nullptr && vec3d->squareDistanceTo(posX, vec3d->yCoord, posZ) < d * d;)
	{
		pathToEntity->incrementPathIndex();
		if (pathToEntity->isFinished())
		{
			vec3d = nullptr;
			pathToEntity = nullptr;
		}
		else
		{
			vec3d = pathToEntity->getPosition(this);
		}
	}
	isJumping = false;
	if (vec3d != nullptr)
	{
		double d1 = vec3d->xCoord - posX;
		double d2 = vec3d->zCoord - posZ;
		double d3 = vec3d->yCoord - (double)i;
		float f2 = (float)((std::atan2(d2, d1) * 180.0) / 3.1415927410125732) - 90.0f;
		float f3 = f2 - rotationYaw;
		moveForward = moveSpeed;
		for (; f3 < -180.0f; f3 += 360.0f) {}
		for (; f3 >= 180.0f; f3 -= 360.0f) {}
		if (f3 >  30.0f) { f3 =  30.0f; }
		if (f3 < -30.0f) { f3 = -30.0f; }
		rotationYaw += f3;
		if (hasAttacked && playerToAttack != nullptr)
		{
			double d4 = playerToAttack->posX - posX;
			double d5 = playerToAttack->posZ - posZ;
			float f5 = rotationYaw;
			rotationYaw = (float)((std::atan2(d5, d4) * 180.0) / 3.1415927410125732) - 90.0f;
			float f4 = (((f5 - rotationYaw) + 90.0f) * 3.141593f) / 180.0f;
			moveStrafing = -MathHelper::sin(f4) * moveForward * 1.0f;
			moveForward  =  MathHelper::cos(f4) * moveForward * 1.0f;
		}
		if (d3 > 0.0)
		{
			isJumping = true;
		}
	}
	if (playerToAttack != nullptr)
	{
		faceEntity(playerToAttack, 30.0f, 30.0f);
	}
	if (isCollidedHorizontally && !hasPath())
	{
		isJumping = true;
	}
	if (rand.nextFloat() < 0.8f && (flag || flag1))
	{
		isJumping = true;
	}
}

void EntityCreature::func_31026_E()
{
	bool flag = false;
	int_t i = -1;
	int_t j = -1;
	int_t k = -1;
	float f = -99999.0f;
	for (int_t l = 0; l < 10; l++)
	{
		int_t i1 = MathHelper::floor_double((posX + (double)rand.nextInt(13)) - 6.0);
		int_t j1 = MathHelper::floor_double((posY + (double)rand.nextInt(7))  - 3.0);
		int_t k1 = MathHelper::floor_double((posZ + (double)rand.nextInt(13)) - 6.0);
		float f1 = getBlockPathWeight(i1, j1, k1);
		if (f1 > f)
		{
			f = f1;
			i = i1;
			j = j1;
			k = k1;
			flag = true;
		}
	}
	if (flag)
	{
		pathToEntity.reset(worldObj->getEntityPathToXYZ(this, i, j, k, 10.0f));
	}
}

void EntityCreature::attackEntity(Entity *entity, float f)
{
}

void EntityCreature::attackBlockedEntity(Entity *entity, float f)
{
}

float EntityCreature::getBlockPathWeight(int_t i, int_t j, int_t k)
{
	return 0.0f;
}

Entity *EntityCreature::findPlayerToAttack()
{
	return nullptr;
}

bool EntityCreature::getCanSpawnHere()
{
	int_t i = MathHelper::floor_double(posX);
	int_t j = MathHelper::floor_double(boundingBox->minY);
	int_t k = MathHelper::floor_double(posZ);
	return EntityLiving::getCanSpawnHere() && getBlockPathWeight(i, j, k) >= 0.0f;
}

bool EntityCreature::hasPath()
{
	return pathToEntity != nullptr;
}

void EntityCreature::setPathToEntity(PathEntity *pathentity)
{
	pathToEntity.reset(pathentity);
}

Entity *EntityCreature::getTarget()
{
	return playerToAttack;
}

void EntityCreature::setTarget(Entity *entity)
{
	playerToAttack = entity;
}
