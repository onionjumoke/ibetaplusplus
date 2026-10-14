#pragma once

#include "EntityFlying.h"
#include "IMob.h"

class World;
class Entity;

// net.minecraft.src.EntityGhast
class EntityGhast : public EntityFlying, public IMob
{
public:
	EntityGhast(World *world);

protected:
	void entityInit() override;

public:
	void onUpdate() override;
	void updatePlayerActionState() override;

private:
	bool isCourseTraversable(double d, double d1, double d2, double d3);

protected:
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDropItemId() override;
	float getSoundVolume() override;

public:
	bool getCanSpawnHere() override;
	int_t getMaxSpawnedInChunk() override;

public:
	int courseChangeCooldown;
	double waypointX;
	double waypointY;
	double waypointZ;
	int prevAttackCounter;
	int attackCounter;

private:
	Entity *targetedEntity;
	int aggroCooldown;
};
