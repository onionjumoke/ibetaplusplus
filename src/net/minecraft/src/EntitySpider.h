#pragma once

#include "EntityMob.h"

class World;
class Entity;
class NBTTagCompound;

// net.minecraft.src.EntitySpider
class EntitySpider : public EntityMob
{
public:
	EntitySpider(World *world);

	double getMountedYOffset() override;

protected:
	bool canTriggerWalking() override;
	Entity *findPlayerToAttack() override;
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	void attackEntity(Entity *entity, float f) override;

public:
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;

protected:
	int_t getDropItemId() override;

public:
	bool isOnLadder() override;
};
