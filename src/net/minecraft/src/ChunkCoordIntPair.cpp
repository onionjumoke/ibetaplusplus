#include "ChunkCoordIntPair.h"

ChunkCoordIntPair::ChunkCoordIntPair(int_t i, int_t j) :
	chunkXPos(i),
	chunkZPos(j)
{
}

int_t ChunkCoordIntPair::chunkXZ2Int(int_t i, int_t j)
{
	return (i >= 0 ? 0 : (int_t)0x80000000) | (i & 0x7fff) << 16 | (j >= 0 ? 0 : 0x8000) | j & 0x7fff;
}

int_t ChunkCoordIntPair::hashCode() const
{
	return chunkXZ2Int(chunkXPos, chunkZPos);
}

bool ChunkCoordIntPair::equals(const ChunkCoordIntPair *obj) const
{
	return obj->chunkXPos == chunkXPos && obj->chunkZPos == chunkZPos;
}
