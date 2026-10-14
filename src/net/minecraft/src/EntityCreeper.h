#pragma once

#include "EntityMob.h"

class World;
class NBTTagCompound;
class EntityLightningBolt;
class Entity;

// net.minecraft.src.EntityCreeper
class EntityCreeper : public EntityMob
{
public:
	EntityCreeper(World *world);

	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;
	void onUpdate() override;
	void onDeath(Entity *entity) override;
	bool getPowered();
	float setCreeperFlashTime(float f);
	void onStruckByLightning(EntityLightningBolt *entitylightningbolt) override;

	int_t timeSinceIgnited;
	int_t lastActiveTime;

protected:
	void entityInit() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	void attackBlockedEntity(Entity *entity, float f) override;
	void attackEntity(Entity *entity, float f) override;
	int_t getDropItemId() override;

private:
	int_t getCreeperState();
	void setCreeperState(int_t i);
};
