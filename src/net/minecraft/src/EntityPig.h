#pragma once

#include "EntityAnimal.h"

class World;
class NBTTagCompound;
class EntityPlayer;
class EntityLightningBolt;

// net.minecraft.src.EntityPig
class EntityPig : public EntityAnimal
{
public:
	EntityPig(World *world);

protected:
	void entityInit() override;

public:
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;

protected:
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;

public:
	bool interact(EntityPlayer *entityplayer) override;

protected:
	int_t getDropItemId() override;

public:
	bool getSaddled();
	void setSaddled(bool flag);
	void onStruckByLightning(EntityLightningBolt *entitylightningbolt) override;

protected:
	void fall(float f) override;
};
