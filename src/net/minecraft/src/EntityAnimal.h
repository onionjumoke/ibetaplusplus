#pragma once

#include "EntityCreature.h"

class World;
class NBTTagCompound;

// net.minecraft.src.EntityAnimal
class EntityAnimal : public EntityCreature
{
public:
	EntityAnimal(World *world) : EntityCreature(world) {}

protected:
	float getBlockPathWeight(int_t i, int_t j, int_t k) override;

public:
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override { EntityCreature::writeEntityToNBT(nbttagcompound); }
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override { EntityCreature::readEntityFromNBT(nbttagcompound); }
	bool getCanSpawnHere() override;
	int_t getTalkInterval() override { return 120; }
};
