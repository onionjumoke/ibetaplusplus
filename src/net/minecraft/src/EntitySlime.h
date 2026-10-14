#pragma once

#include "EntityLiving.h"
#include "IMob.h"

class World;
class NBTTagCompound;
class EntityPlayer;

// net.minecraft.src.EntitySlime
class EntitySlime : public EntityLiving, public IMob
{
public:
	EntitySlime(World *world);

protected:
	void entityInit() override;

public:
	void setSlimeSize(int_t i);
	int_t getSlimeSize();
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;
	void onUpdate() override;

protected:
	void updatePlayerActionState() override;

public:
	void onEntityDeath() override;
	bool getCanSpawnHere() override;

protected:
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	float getSoundVolume() override;
	int_t getDropItemId() override;

public:
	float field_768_a;
	float field_767_b;
	float field_21087_c;
	float field_21086_f;

private:
	int_t slimeJumpDelay;
};
