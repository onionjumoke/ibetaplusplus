#pragma once

#include "EntityZombie.h"

class World;
class Entity;
class NBTTagCompound;

// net.minecraft.src.EntityPigZombie
class EntityPigZombie : public EntityZombie
{
public:
	EntityPigZombie(World *world);

	void onUpdate() override;
	bool getCanSpawnHere() override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;

protected:
	Entity *findPlayerToAttack() override;

public:
	void onLivingUpdate() override;
	bool attackEntityFrom(Entity *entity, int_t i) override;

protected:
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDropItemId() override;

public:
	ItemStack *getHeldItem() override;

private:
	void becomeAngryAt(Entity *entity);

	int angerLevel;
	int randomSoundDelay;
	static ItemStack *defaultHeldItem;
};
