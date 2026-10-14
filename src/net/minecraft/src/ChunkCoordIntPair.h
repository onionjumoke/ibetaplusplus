#pragma once

#include <cstddef>

#include "java/Type.h"

// net.minecraft.src.ChunkCoordIntPair
class ChunkCoordIntPair
{
public:
	ChunkCoordIntPair(int_t i, int_t j);

	static int_t chunkXZ2Int(int_t i, int_t j);
	int_t hashCode() const;
	bool equals(const ChunkCoordIntPair *obj) const;

	const int_t chunkXPos;
	const int_t chunkZPos;

	bool operator<(const ChunkCoordIntPair &o) const
	{
		if (chunkXPos != o.chunkXPos) return chunkXPos < o.chunkXPos;
		return chunkZPos < o.chunkZPos;
	}
	bool operator==(const ChunkCoordIntPair &o) const
	{
		return chunkXPos == o.chunkXPos && chunkZPos == o.chunkZPos;
	}
};

struct ChunkCoordIntPairHash
{
	std::size_t operator()(const ChunkCoordIntPair *a) const
	{
		return (std::size_t)(uint_t)a->hashCode();
	}
};

struct ChunkCoordIntPairEqual
{
	bool operator()(const ChunkCoordIntPair *a, const ChunkCoordIntPair *b) const
	{
		return a->equals(b);
	}
};
