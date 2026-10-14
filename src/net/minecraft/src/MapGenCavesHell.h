#pragma once

#include "MapGenBase.h"

// net.minecraft.src.MapGenCavesHell
class MapGenCavesHell : public MapGenBase
{
public:
	MapGenCavesHell();

protected:
	void generateChunk(World *world, int_t i, int_t j, int_t k, int_t l, byte_t abyte0[]) override;

private:
	void generateCaveNode(int_t i, int_t j, byte_t abyte0[], double d, double d1, double d2);
	void generateCaveRoom(int_t i, int_t j, byte_t abyte0[], double d, double d1, double d2,
		float f, float f1, float f2, int_t k, int_t l, double d3);
};
