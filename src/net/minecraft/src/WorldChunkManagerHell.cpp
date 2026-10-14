#include "WorldChunkManagerHell.h"

#include <algorithm>

#include "BiomeGenBase.h"
#include "ChunkCoordIntPair.h"

WorldChunkManagerHell::WorldChunkManagerHell(BiomeGenBase *biomegenbase, double d, double d1)
	: WorldChunkManager()
	, biome(biomegenbase)
	, hellTemperature(d)
	, hellHumidity(d1)
{
}

BiomeGenBase *WorldChunkManagerHell::getBiomeGenAtChunkCoord(ChunkCoordIntPair *)
{
	return biome;
}

BiomeGenBase *WorldChunkManagerHell::getBiomeGenAt(int_t, int_t)
{
	return biome;
}

double WorldChunkManagerHell::getTemperature(int_t, int_t)
{
	return hellTemperature;
}

std::vector<double> &WorldChunkManagerHell::getTemperatures(std::vector<double> &ad, int_t, int_t, int_t k, int_t l)
{
	if (ad.empty() || (int_t)ad.size() < k * l)
		ad.assign(k * l, 0.0);
	std::fill(ad.begin(), ad.begin() + k * l, hellTemperature);
	return ad;
}

std::vector<BiomeGenBase *> WorldChunkManagerHell::loadBlockGeneratorData(std::vector<BiomeGenBase *> abiomegenbase, int_t, int_t, int_t k, int_t l)
{
	if (abiomegenbase.empty() || (int_t)abiomegenbase.size() < k * l)
		abiomegenbase.assign(k * l, nullptr);
	if (temperature.empty() || (int_t)temperature.size() < k * l)
	{
		temperature.assign(k * l, 0.0);
		humidity.assign(k * l, 0.0);
	}
	std::fill(abiomegenbase.begin(), abiomegenbase.begin() + k * l, biome);
	std::fill(humidity.begin(), humidity.begin() + k * l, hellHumidity);
	std::fill(temperature.begin(), temperature.begin() + k * l, hellTemperature);
	return abiomegenbase;
}
