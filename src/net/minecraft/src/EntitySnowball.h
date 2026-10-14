#pragma once

#include "Entity.h"
#include "java/Type.h"

class World;
class EntityLiving;
class EntityPlayer;
class NBTTagCompound;

// net.minecraft.src.EntitySnowball
class EntitySnowball : public Entity
{
public:
	EntitySnowball(World *world);
	EntitySnowball(World *world, EntityLiving *entityliving);
	EntitySnowball(World *world, double d, double d1, double d2);

protected:
	void entityInit() override;

public:
	bool isInRangeToRenderDist(double d) override;
	void setSnowballHeading(double d, double d1, double d2, float f, float f1);
	void setVelocity(double d, double d1, double d2) override;
	void onUpdate() override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;
	void onCollideWithPlayer(EntityPlayer *entityplayer) override;

private:
	int xTileSnowball;
	int yTileSnowball;
	int zTileSnowball;
	int inTileSnowball;
	bool inGroundSnowball;
	int shakeSnowball;
	int ticksInGroundSnowball;
	int ticksInAirSnowball;
	EntityLiving *thrower;
};
