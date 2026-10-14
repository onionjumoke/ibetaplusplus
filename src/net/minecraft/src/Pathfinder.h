#pragma once

#include <memory>
#include <vector>

#include "java/Type.h"

class Path;
class MCHash;
class PathPoint;
class PathEntity;
class IBlockAccess;
class Entity;

// net.minecraft.src.Pathfinder
class Pathfinder
{
public:
	Pathfinder(IBlockAccess *iblockaccess);
	~Pathfinder();

	PathEntity *createEntityPathTo(Entity *entity, Entity *entity1, float f);
	PathEntity *createEntityPathTo(Entity *entity, int_t i, int_t j, int_t k, float f);

private:
	PathEntity *createEntityPathTo(Entity *entity, double d, double d1, double d2, float f);
	PathEntity *addToPath(Entity *entity, PathPoint *pathpoint, PathPoint *pathpoint1, PathPoint *pathpoint2, float f);
	int_t       findPathOptions(Entity *entity, PathPoint *pathpoint, PathPoint *pathpoint1, PathPoint *pathpoint2, float f);
	PathPoint  *getSafePoint(Entity *entity, int_t i, int_t j, int_t k, PathPoint *pathpoint, int_t l);
	PathPoint  *openPoint(int_t i, int_t j, int_t k);
	int_t       getVerticalOffset(Entity *entity, int_t i, int_t j, int_t k, PathPoint *pathpoint);
	PathEntity *createEntityPath(PathPoint *pathpoint, PathPoint *pathpoint1);

	IBlockAccess *worldMap;
	Path *path;
	MCHash *pointMap;
	std::vector<PathPoint *> pathOptions;
	std::vector<PathPoint *> allocatedPoints;
};
