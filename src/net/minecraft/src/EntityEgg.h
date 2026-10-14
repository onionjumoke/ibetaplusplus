#pragma once

#include "Entity.h"
#include "java/Type.h"

class World;
class EntityLiving;
class NBTTagCompound;

// net.minecraft.src.EntityEgg
class EntityEgg : public Entity
{
public:
	EntityEgg(World *world);
	EntityEgg(World *world, EntityLiving *entityliving);
	EntityEgg(World *world, double d, double d1, double d2);

protected:
	void entityInit() override;

public:
	void setEggHeading(double d, double d1, double d2, float f, float f1);
	void onUpdate() override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;

private:
	int xTile;
	int yTile;
	int zTile;
	int inTile;
	bool inGround;
	EntityLiving *thrower;
	int ticksInGround;
	int ticksInAir;
};
