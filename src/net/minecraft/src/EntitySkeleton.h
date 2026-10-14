#pragma once

#include "EntityMob.h"

class World;
class Entity;
class NBTTagCompound;
class ItemStack;

// net.minecraft.src.EntitySkeleton
class EntitySkeleton : public EntityMob
{
public:
	EntitySkeleton(World *world);

	void onLivingUpdate() override;

protected:
	void attackEntity(Entity *entity, float f) override;
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;

public:
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;

protected:
	int_t getDropItemId() override;
	void dropFewItems() override;

public:
	ItemStack *getHeldItem() override;

private:
	static ItemStack *defaultHeldItem;
};
