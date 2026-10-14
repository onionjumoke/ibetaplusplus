#pragma once

#include <vector>
#include "java/Type.h"
#include "WorldChunkManager.h"

class BiomeGenBase;
class ChunkCoordIntPair;

// net.minecraft.src.WorldChunkManagerHell
class WorldChunkManagerHell : public WorldChunkManager
{
public:
	WorldChunkManagerHell(BiomeGenBase *biomegenbase, double d, double d1);

	BiomeGenBase *getBiomeGenAtChunkCoord(ChunkCoordIntPair *chunkcoordintpair) override;
	BiomeGenBase *getBiomeGenAt(int_t i, int_t j) override;
	double getTemperature(int_t i, int_t j) override;
	std::vector<double> &getTemperatures(std::vector<double> &ad, int_t i, int_t j, int_t k, int_t l) override;
	std::vector<BiomeGenBase *> loadBlockGeneratorData(std::vector<BiomeGenBase *> abiomegenbase, int_t i, int_t j, int_t k, int_t l) override;

private:
	BiomeGenBase *biome;             // field_4201_e
	double hellTemperature;          // field_4200_f
	double hellHumidity;             // field_4199_g
};
