#include "EntityGiantZombie.h"

#include "World.h"

EntityGiantZombie::EntityGiantZombie(World *world)
	: EntityMob(world)
{
	texture = String::fromUTF8("/mob/zombie.png");
	moveSpeed = 0.5f;
	attackStrength = 50;
	health *= 10;
	yOffset *= 6.0f;
	setSize(width * 6.0f, height * 6.0f);
}

float EntityGiantZombie::getBlockPathWeight(int_t i, int_t j, int_t k)
{
	return worldObj->getLightBrightness(i, j, k) - 0.5f;
}
