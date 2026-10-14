#pragma once

#include "Entity.h"
#include "java/Type.h"

class World;
class EntityLiving;
class EntityPlayer;
class NBTTagCompound;
class Vec3D;
class MovingObjectPosition;

// net.minecraft.src.EntityArrow
class EntityArrow : public Entity
{
public:
	EntityArrow(World *world);
	EntityArrow(World *world, double d, double d1, double d2);
	EntityArrow(World *world, EntityLiving *entityliving);

protected:
	void entityInit() override;

public:
	void setArrowHeading(double d, double d1, double d2, float f, float f1);
	void setVelocity(double d, double d1, double d2) override;
	void onUpdate() override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;
	void onCollideWithPlayer(EntityPlayer *entityplayer) override;
	float getShadowSize() override;

public:
	int_t xTile;
	int_t yTile;
	int_t zTile;
	int_t inTile;
	int_t inData;
	bool  inGround;
	// Java: doesArrowBelongToPlayer — true if player-fired (used for pick-up).
	bool  doesArrowBelongToPlayer;
	int_t arrowShake;
	int_t ticksInGround;
	int_t ticksInAir;
	EntityLiving *owner;
};
