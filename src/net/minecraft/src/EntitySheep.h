#pragma once

#include "EntityAnimal.h"
#include "java/Random.h"

class World;
class NBTTagCompound;
class Entity;
class EntityPlayer;

// net.minecraft.src.EntitySheep
class EntitySheep : public EntityAnimal
{
public:
	EntitySheep(World *world);

protected:
	void entityInit() override;

public:
	bool attackEntityFrom(Entity *entity, int_t i) override { return EntityAnimal::attackEntityFrom(entity, i); }

protected:
	void dropFewItems() override;
	int_t getDropItemId() override;

public:
	bool interact(EntityPlayer *entityplayer) override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;

protected:
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;

public:
	int_t getFleeceColor();
	void setFleeceColor(int_t i);
	bool getSheared();
	void setSheared(bool flag);

	static int_t getRandomFleeceColor(Random &random);

	static const float fleeceColorTable[16][3];
};
