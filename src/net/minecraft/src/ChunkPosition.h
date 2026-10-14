#pragma once

#include "java/Type.h"

// net.minecraft.src.ChunkPosition
class ChunkPosition
{
public:
	ChunkPosition(int_t i, int_t j, int_t k) : x(i), y(j), z(k) {}

	bool equals(const ChunkPosition &other) const
	{
		return other.x == x && other.y == y && other.z == z;
	}

	int_t hashCode() const
	{
		return x * 0x88f9fa + y * 0xef88b + z;
	}

	int_t x;
	int_t y;
	int_t z;
};
