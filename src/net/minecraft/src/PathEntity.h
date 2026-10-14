#pragma once

#include <vector>
#include "java/Type.h"
#include "PathPoint.h"

class Entity;
class Vec3D;

// net.minecraft.src.PathEntity
class PathEntity
{
public:
	PathEntity(const std::vector<PathPoint *> &apathpoint);

	void incrementPathIndex();
	bool isFinished();
	PathPoint *func_22328_c();
	PathPoint *getFinalPathPoint() { return func_22328_c(); } // last point in path
	Vec3D *getPosition(Entity *entity);

	const int_t pathLength;

private:
	std::vector<PathPoint> points;
	int_t pathIndex;
};
