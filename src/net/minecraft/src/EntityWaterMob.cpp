#include "EntityWaterMob.h"

#include "World.h"

bool EntityWaterMob::getCanSpawnHere()
{
	return worldObj->checkIfAABBIsClear(boundingBox);
}
