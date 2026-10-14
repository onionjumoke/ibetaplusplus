#pragma once

#include <vector>
#include "java/Type.h"

class World;
class BiomeGenBase;
class ChunkCoordIntPair;
class NoiseGeneratorOctaves2;

// net.minecraft.src.WorldChunkManager
class WorldChunkManager
{
public:
	WorldChunkManager();
	explicit WorldChunkManager(World *world);
	virtual ~WorldChunkManager();

	virtual BiomeGenBase *getBiomeGenAtChunkCoord(ChunkCoordIntPair *chunkcoordintpair);
	virtual BiomeGenBase *getBiomeGenAt(int_t i, int_t j);
	virtual double getTemperature(int_t i, int_t j);
	virtual std::vector<BiomeGenBase *> &getBiomeBlock(int_t i, int_t j, int_t k, int_t l);
	virtual std::vector<double> &getTemperatures(std::vector<double> &ad, int_t i, int_t j, int_t k, int_t l);
	virtual std::vector<BiomeGenBase *> loadBlockGeneratorData(std::vector<BiomeGenBase *> abiomegenbase, int_t i, int_t j, int_t k, int_t l);

	std::vector<double> temperature;
	std::vector<double> humidity;
	std::vector<double> modifierValues;  // field_4196_c
	std::vector<BiomeGenBase *> biomeCache;  // field_4195_d

	// When set, every biome lookup returns this biome (used by Superflat so the
	// whole world is Forest). nullptr = normal noise-driven biomes.
	BiomeGenBase *forcedBiome = nullptr;

private:
	NoiseGeneratorOctaves2 *temperatureNoise;  // field_4194_e
	NoiseGeneratorOctaves2 *humidityNoise;     // field_4193_f
	NoiseGeneratorOctaves2 *noiseModifier;     // field_4192_g
};
