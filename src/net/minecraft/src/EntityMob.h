#pragma once

#include "EntityCreature.h"
#include "IMob.h"

class World;
class Entity;
class NBTTagCompound;

// net.minecraft.src.EntityMob
class EntityMob : public EntityCreature, public IMob
{
public:
	EntityMob(World *world);

	void onLivingUpdate() override;
	void onUpdate() override;

protected:
	Entity *findPlayerToAttack() override;

public:
	bool attackEntityFrom(Entity *entity, int_t i) override;

protected:
	void attackEntity(Entity *entity, float f) override;
	float getBlockPathWeight(int_t i, int_t j, int_t k) override;

public:
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override { EntityCreature::writeEntityToNBT(nbttagcompound); }
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override { EntityCreature::readEntityFromNBT(nbttagcompound); }
	bool getCanSpawnHere() override;

protected:
	int_t attackStrength;
};
