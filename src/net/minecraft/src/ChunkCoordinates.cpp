#include "ChunkCoordinates.h"

#include <cmath>

ChunkCoordinates::ChunkCoordinates()
{
}

ChunkCoordinates::ChunkCoordinates(int_t i, int_t j, int_t k)
{
	x = i;
	y = j;
	z = k;
}

ChunkCoordinates::ChunkCoordinates(const ChunkCoordinates *chunkcoordinates)
{
	x = chunkcoordinates->x;
	y = chunkcoordinates->y;
	z = chunkcoordinates->z;
}

bool ChunkCoordinates::equals(const ChunkCoordinates *obj) const
{
	if (obj == nullptr)
		return false;
	else
		return x == obj->x && y == obj->y && z == obj->z;
}

int_t ChunkCoordinates::hashCode() const
{
	return x + z << 8 + y << 16;
}

int_t ChunkCoordinates::compareChunkCoordinate(const ChunkCoordinates *chunkcoordinates) const
{
	if (y == chunkcoordinates->y)
	{
		if (z == chunkcoordinates->z)
			return x - chunkcoordinates->x;
		else
			return z - chunkcoordinates->z;
	}
	else
	{
		return y - chunkcoordinates->y;
	}
}

double ChunkCoordinates::getSqDistanceTo(int_t i, int_t j, int_t k) const
{
	int_t l = x - i;
	int_t i1 = y - j;
	int_t j1 = z - k;
	return std::sqrt((double)(l * l + i1 * i1 + j1 * j1));
}

int_t ChunkCoordinates::compareTo(const ChunkCoordinates *obj) const
{
	return compareChunkCoordinate(obj);
}
