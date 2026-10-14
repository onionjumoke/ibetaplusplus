#pragma once

#include "EntityMob.h"

class World;
class NBTTagCompound;

// net.minecraft.src.EntityZombie
class EntityZombie : public EntityMob
{
public:
	EntityZombie(World *world);

	void onLivingUpdate() override;

protected:
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	int_t getDropItemId() override;
};
