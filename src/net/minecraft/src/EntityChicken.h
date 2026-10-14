#pragma once

#include "EntityAnimal.h"

class World;
class NBTTagCompound;

// net.minecraft.src.EntityChicken
class EntityChicken : public EntityAnimal
{
public:
	EntityChicken(World *world);

	void onLivingUpdate() override;

protected:
	void fall(float f) override {}

public:
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override { EntityAnimal::writeEntityToNBT(nbttagcompound); }
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override { EntityAnimal::readEntityFromNBT(nbttagcompound); }

protected:
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDropItemId() override;

public:
	bool   field_753_a;
	float  field_752_b;
	float  destPos;
	float  field_757_d;
	float  field_756_e;
	float  field_755_h;
	int_t  timeUntilNextEgg;
};
