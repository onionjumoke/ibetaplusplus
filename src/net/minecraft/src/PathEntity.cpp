#include "PathEntity.h"

#include "PathPoint.h"
#include "Entity.h"
#include "Vec3D.h"

PathEntity::PathEntity(const std::vector<PathPoint *> &apathpoint)
	: pathLength((int_t)apathpoint.size()), pathIndex(0)
{
	points.reserve(apathpoint.size());
	for (PathPoint *point : apathpoint)
		points.emplace_back(point->xCoord, point->yCoord, point->zCoord);
}

void PathEntity::incrementPathIndex()
{
	pathIndex++;
}

bool PathEntity::isFinished()
{
	return (size_t)pathIndex >= points.size();
}

PathPoint *PathEntity::func_22328_c()
{
	if (pathLength > 0)
	{
		return &points[pathLength - 1];
	}
	return nullptr;
}

Vec3D *PathEntity::getPosition(Entity *entity)
{
	double d  = (double)points[pathIndex].xCoord + (double)(int_t)(entity->width + 1.0f) * 0.5;
	double d1 = (double)points[pathIndex].yCoord;
	double d2 = (double)points[pathIndex].zCoord + (double)(int_t)(entity->width + 1.0f) * 0.5;
	return Vec3D::createVector(d, d1, d2);
}
