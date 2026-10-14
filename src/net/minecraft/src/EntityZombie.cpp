#include "EntityZombie.h"

#include "Item.h"
#include "MathHelper.h"
#include "World.h"

EntityZombie::EntityZombie(World *world)
	: EntityMob(world)
{
	texture = String::fromUTF8("/mob/zombie.png");
	moveSpeed = 0.5f;
	attackStrength = 5;
}

void EntityZombie::onLivingUpdate()
{
	if (worldObj->isDaytime())
	{
		float f = getEntityBrightness(1.0f);
		if (f > 0.5f && worldObj->canBlockSeeTheSky(MathHelper::floor_double(posX), MathHelper::floor_double(posY), MathHelper::floor_double(posZ)) && rand.nextFloat() * 30.0f < (f - 0.4f) * 2.0f)
		{
			fire = 300;
		}
	}
	EntityMob::onLivingUpdate();
}

jstring EntityZombie::getLivingSound() { return u"mob.zombie"; }
jstring EntityZombie::getHurtSound()   { return u"mob.zombiehurt"; }
jstring EntityZombie::getDeathSound()  { return u"mob.zombiedeath"; }

int_t EntityZombie::getDropItemId()
{
	return Item::feather->shiftedIndex;
}
