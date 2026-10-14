#pragma once

#include "Entity.h"
#include "java/Type.h"

class World;
class EntityLiving;
class NBTTagCompound;
class MovingObjectPosition;
class Vec3D;

// net.minecraft.src.EntityFireball
class EntityFireball : public Entity
{
public:
	EntityFireball(World *world);
	EntityFireball(World *world, EntityLiving *entityliving, double d, double d1, double d2);
	EntityFireball(World *world, double x, double y, double z, double ax, double ay, double az);

protected:
	void entityInit() override;

public:
	void onUpdate() override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;

private:
	int xTile;
	int yTile;
	int zTile;
	int inTile;
	bool inGround;
	EntityLiving *shootingEntity;
	int ticksInGround;
	int ticksInAir;
	double accelerationX;
	double accelerationY;
	double accelerationZ;
};
