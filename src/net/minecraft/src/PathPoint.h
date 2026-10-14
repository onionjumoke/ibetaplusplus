#pragma once

#include "java/Type.h"
#include "MathHelper.h"

// net.minecraft.src.PathPoint
class PathPoint
{
public:
	PathPoint(int_t i, int_t j, int_t k)
		: xCoord(i), yCoord(j), zCoord(k),
		  hash(hashOf(i, j, k)), index(-1), isFirst(false),
		  totalPathDistance(0), distanceToNext(0), distanceToTarget(0),
		  previous(nullptr) {}

	static int_t hashOf(int_t i, int_t j, int_t k)
	{
		return j & 0xff | (i & 0x7fff) << 8 | (k & 0x7fff) << 24
		       | (i >= 0 ? 0 : 0x80000000) | (k >= 0 ? 0 : 0x8000);
	}

	float distanceTo(PathPoint *pathpoint) const
	{
		float f  = (float)(pathpoint->xCoord - xCoord);
		float f1 = (float)(pathpoint->yCoord - yCoord);
		float f2 = (float)(pathpoint->zCoord - zCoord);
		return MathHelper::sqrt_float(f * f + f1 * f1 + f2 * f2);
	}

	bool equals(const PathPoint &other) const
	{
		return hash == other.hash && xCoord == other.xCoord
		       && yCoord == other.yCoord && zCoord == other.zCoord;
	}

	int_t hashCode() const { return hash; }
	bool isAssigned() const { return index >= 0; }

	const int_t xCoord;
	const int_t yCoord;
	const int_t zCoord;

	int_t index;
	float totalPathDistance;
	float distanceToNext;
	float distanceToTarget;
	PathPoint *previous;
	bool isFirst;

private:
	const int_t hash;
};
