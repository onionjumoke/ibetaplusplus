#pragma once

#include "Entity.h"
#include "java/Type.h"

class World;
class NBTTagCompound;

// net.minecraft.src.EntityFallingSand
class EntityFallingSand : public Entity
{
public:
	EntityFallingSand(World *world);
	EntityFallingSand(World *world, double d, double d1, double d2, int_t i);

protected:
	void entityInit() override;

public:
	bool canBeCollidedWith() override { return false; }
	void onUpdate() override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;
	float getShadowSize() override;

public:
	int_t blockID;
	int_t fallTime;
};
