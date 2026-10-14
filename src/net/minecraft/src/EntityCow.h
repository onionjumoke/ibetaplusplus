#pragma once

#include "EntityAnimal.h"

class World;
class NBTTagCompound;
class EntityPlayer;

// net.minecraft.src.EntityCow
class EntityCow : public EntityAnimal
{
public:
	EntityCow(World *world);

	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override { EntityAnimal::writeEntityToNBT(nbttagcompound); }
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override { EntityAnimal::readEntityFromNBT(nbttagcompound); }

protected:
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	float getSoundVolume() override;
	int_t getDropItemId() override;

public:
	bool interact(EntityPlayer *entityplayer) override;
};
