#include "Pathfinder.h"

#include "AxisAlignedBB.h"
#include "Block.h"
#include "BlockDoor.h"
#include "Entity.h"
#include "IBlockAccess.h"
#include "MCHash.h"
#include "MathHelper.h"
#include "Material.h"
#include "Path.h"
#include "PathEntity.h"
#include "PathPoint.h"

Pathfinder::Pathfinder(IBlockAccess *iblockaccess)
	: pathOptions(32, nullptr)
{
	path = new Path();
	pointMap = new MCHash();
	worldMap = iblockaccess;
}

Pathfinder::~Pathfinder()
{
	delete path;
	delete pointMap;
	for (PathPoint *point : allocatedPoints)
		delete point;
}

PathEntity *Pathfinder::createEntityPathTo(Entity *entity, Entity *entity1, float f)
{
	return createEntityPathTo(entity, entity1->posX, entity1->boundingBox->minY, entity1->posZ, f);
}

PathEntity *Pathfinder::createEntityPathTo(Entity *entity, int_t i, int_t j, int_t k, float f)
{
	return createEntityPathTo(entity, (double)((float)i + 0.5f), (double)((float)j + 0.5f), (double)((float)k + 0.5f), f);
}

PathEntity *Pathfinder::createEntityPathTo(Entity *entity, double d, double d1, double d2, float f)
{
	pointMap->clearMap();
	for (PathPoint *point : allocatedPoints)
		delete point;
	allocatedPoints.clear();

	path->clearPath();
	PathPoint *pathpoint  = openPoint(MathHelper::floor_double(entity->boundingBox->minX), MathHelper::floor_double(entity->boundingBox->minY), MathHelper::floor_double(entity->boundingBox->minZ));
	PathPoint *pathpoint1 = openPoint(MathHelper::floor_double(d - (double)(entity->width / 2.0f)), MathHelper::floor_double(d1), MathHelper::floor_double(d2 - (double)(entity->width / 2.0f)));
	PathPoint *pathpoint2 = new PathPoint(MathHelper::floor_float(entity->width + 1.0f), MathHelper::floor_float(entity->height + 1.0f), MathHelper::floor_float(entity->width + 1.0f));
	allocatedPoints.push_back(pathpoint2);
	PathEntity *pathentity = addToPath(entity, pathpoint, pathpoint1, pathpoint2, f);
	return pathentity;
}

PathEntity *Pathfinder::addToPath(Entity *entity, PathPoint *pathpoint, PathPoint *pathpoint1, PathPoint *pathpoint2, float f)
{
	pathpoint->totalPathDistance = 0.0f;
	pathpoint->distanceToNext    = pathpoint->distanceTo(pathpoint1);
	pathpoint->distanceToTarget  = pathpoint->distanceToNext;
	path->clearPath();
	path->addPoint(pathpoint);
	PathPoint *pathpoint3 = pathpoint;
	while (!path->isPathEmpty())
	{
		PathPoint *pathpoint4 = path->dequeue();
		if (pathpoint4->equals(*pathpoint1))
		{
			return createEntityPath(pathpoint, pathpoint1);
		}
		if (pathpoint4->distanceTo(pathpoint1) < pathpoint3->distanceTo(pathpoint1))
		{
			pathpoint3 = pathpoint4;
		}
		pathpoint4->isFirst = true;
		int_t i = findPathOptions(entity, pathpoint4, pathpoint2, pathpoint1, f);
		int_t j = 0;
		while (j < i)
		{
			PathPoint *pathpoint5 = pathOptions[j];
			float f1 = pathpoint4->totalPathDistance + pathpoint4->distanceTo(pathpoint5);
			if (!pathpoint5->isAssigned() || f1 < pathpoint5->totalPathDistance)
			{
				pathpoint5->previous = pathpoint4;
				pathpoint5->totalPathDistance = f1;
				pathpoint5->distanceToNext = pathpoint5->distanceTo(pathpoint1);
				if (pathpoint5->isAssigned())
				{
					path->changeDistance(pathpoint5, pathpoint5->totalPathDistance + pathpoint5->distanceToNext);
				}
				else
				{
					pathpoint5->distanceToTarget = pathpoint5->totalPathDistance + pathpoint5->distanceToNext;
					path->addPoint(pathpoint5);
				}
			}
			j++;
		}
	}
	if (pathpoint3 == pathpoint)
	{
		return nullptr;
	}
	return createEntityPath(pathpoint, pathpoint3);
}

int_t Pathfinder::findPathOptions(Entity *entity, PathPoint *pathpoint, PathPoint *pathpoint1, PathPoint *pathpoint2, float f)
{
	int_t i = 0;
	int_t j = 0;
	if (getVerticalOffset(entity, pathpoint->xCoord, pathpoint->yCoord + 1, pathpoint->zCoord, pathpoint1) == 1)
	{
		j = 1;
	}
	PathPoint *pathpoint3 = getSafePoint(entity, pathpoint->xCoord,     pathpoint->yCoord, pathpoint->zCoord + 1, pathpoint1, j);
	PathPoint *pathpoint4 = getSafePoint(entity, pathpoint->xCoord - 1, pathpoint->yCoord, pathpoint->zCoord,     pathpoint1, j);
	PathPoint *pathpoint5 = getSafePoint(entity, pathpoint->xCoord + 1, pathpoint->yCoord, pathpoint->zCoord,     pathpoint1, j);
	PathPoint *pathpoint6 = getSafePoint(entity, pathpoint->xCoord,     pathpoint->yCoord, pathpoint->zCoord - 1, pathpoint1, j);
	if (pathpoint3 != nullptr && !pathpoint3->isFirst && pathpoint3->distanceTo(pathpoint2) < f) { pathOptions[i++] = pathpoint3; }
	if (pathpoint4 != nullptr && !pathpoint4->isFirst && pathpoint4->distanceTo(pathpoint2) < f) { pathOptions[i++] = pathpoint4; }
	if (pathpoint5 != nullptr && !pathpoint5->isFirst && pathpoint5->distanceTo(pathpoint2) < f) { pathOptions[i++] = pathpoint5; }
	if (pathpoint6 != nullptr && !pathpoint6->isFirst && pathpoint6->distanceTo(pathpoint2) < f) { pathOptions[i++] = pathpoint6; }
	return i;
}

PathPoint *Pathfinder::getSafePoint(Entity *entity, int_t i, int_t j, int_t k, PathPoint *pathpoint, int_t l)
{
	PathPoint *pathpoint1 = nullptr;
	if (getVerticalOffset(entity, i, j, k, pathpoint) == 1)
	{
		pathpoint1 = openPoint(i, j, k);
	}
	if (pathpoint1 == nullptr && l > 0 && getVerticalOffset(entity, i, j + l, k, pathpoint) == 1)
	{
		pathpoint1 = openPoint(i, j + l, k);
		j += l;
	}
	if (pathpoint1 != nullptr)
	{
		int_t i1 = 0;
		int_t j1 = 0;
		while (true)
		{
			if (j <= 0 || (j1 = getVerticalOffset(entity, i, j - 1, k, pathpoint)) != 1)
			{
				break;
			}
			if (++i1 >= 4)
			{
				return nullptr;
			}
			if (--j > 0)
			{
				pathpoint1 = openPoint(i, j, k);
			}
		}
		if (j1 == -2)
		{
			return nullptr;
		}
	}
	return pathpoint1;
}

PathPoint *Pathfinder::openPoint(int_t i, int_t j, int_t k)
{
	int_t l = PathPoint::hashOf(i, j, k);
	PathPoint *pathpoint = (PathPoint *)pointMap->lookup(l);
	if (pathpoint == nullptr)
	{
		pathpoint = new PathPoint(i, j, k);
		allocatedPoints.push_back(pathpoint);
		pointMap->addKey(l, pathpoint);
	}
	return pathpoint;
}

int_t Pathfinder::getVerticalOffset(Entity *entity, int_t i, int_t j, int_t k, PathPoint *pathpoint)
{
	for (int_t l = i; l < i + pathpoint->xCoord; l++)
	{
		for (int_t i1 = j; i1 < j + pathpoint->yCoord; i1++)
		{
			for (int_t j1 = k; j1 < k + pathpoint->zCoord; j1++)
			{
				int_t k1 = worldMap->getBlockId(l, i1, j1);
				if (k1 <= 0)
				{
					continue;
				}
				if (k1 == Block::doorSteel->blockID || k1 == Block::doorWood->blockID)
				{
					int_t l1 = worldMap->getBlockMetadata(l, i1, j1);
					if (!BlockDoor::isOpen(l1))
					{
						return 0;
					}
					continue;
				}
				Material *material = Block::blocksList[k1]->blockMaterial;
				if (material->getIsSolid())
				{
					return 0;
				}
				if (material == Material::water)
				{
					return -1;
				}
				if (material == Material::lava)
				{
					return -2;
				}
			}
		}
	}
	return 1;
}

PathEntity *Pathfinder::createEntityPath(PathPoint *pathpoint, PathPoint *pathpoint1)
{
	int_t i = 1;
	for (PathPoint *pathpoint2 = pathpoint1; pathpoint2->previous != nullptr; pathpoint2 = pathpoint2->previous)
	{
		i++;
	}
	std::vector<PathPoint *> apathpoint(i, nullptr);
	PathPoint *pathpoint3 = pathpoint1;
	apathpoint[--i] = pathpoint3;
	while (pathpoint3->previous != nullptr)
	{
		pathpoint3 = pathpoint3->previous;
		apathpoint[--i] = pathpoint3;
	}
	return new PathEntity(apathpoint);
}
