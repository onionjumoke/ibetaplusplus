#pragma once

#include "EntityMob.h"

class World;

// net.minecraft.src.EntityGiantZombie
class EntityGiantZombie : public EntityMob
{
public:
	EntityGiantZombie(World *world);

protected:
	float getBlockPathWeight(int_t i, int_t j, int_t k) override;
};
