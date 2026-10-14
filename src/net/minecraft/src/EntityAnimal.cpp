#include "EntityAnimal.h"

#include "AxisAlignedBB.h"
#include "Block.h"
#include "MathHelper.h"
#include "World.h"

float EntityAnimal::getBlockPathWeight(int_t i, int_t j, int_t k)
{
	if (worldObj->getBlockId(i, j - 1, k) == ((Block *)Block::grass)->blockID)
	{
		return 10.0f;
	}
	return worldObj->getLightBrightness(i, j, k) - 0.5f;
}

bool EntityAnimal::getCanSpawnHere()
{
	int_t i = MathHelper::floor_double(posX);
	int_t j = MathHelper::floor_double(boundingBox->minY);
	int_t k = MathHelper::floor_double(posZ);
	return worldObj->getBlockId(i, j - 1, k) == ((Block *)Block::grass)->blockID && worldObj->getFullBlockLightValue(i, j, k) > 8 && EntityCreature::getCanSpawnHere();
}
