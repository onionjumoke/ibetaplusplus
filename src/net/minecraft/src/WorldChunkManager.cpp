#include "WorldChunkManager.h"

#include "BiomeGenBase.h"
#include "ChunkCoordIntPair.h"
#include "NoiseGeneratorOctaves2.h"
#include "World.h"
#include "java/Random.h"

WorldChunkManager::WorldChunkManager()
	: temperatureNoise(nullptr), humidityNoise(nullptr), noiseModifier(nullptr)
{
}

WorldChunkManager::WorldChunkManager(World *world)
{
	Random r1(world->getRandomSeed() * 9871LL);
	Random r2(world->getRandomSeed() * 39811LL);
	Random r3(world->getRandomSeed() * 0x84a59L);
	temperatureNoise = new NoiseGeneratorOctaves2(r1, 4);
	humidityNoise    = new NoiseGeneratorOctaves2(r2, 4);
	noiseModifier    = new NoiseGeneratorOctaves2(r3, 2);
}

WorldChunkManager::~WorldChunkManager()
{
	delete temperatureNoise;
	delete humidityNoise;
	delete noiseModifier;
}

BiomeGenBase *WorldChunkManager::getBiomeGenAtChunkCoord(ChunkCoordIntPair *chunkcoordintpair)
{
	return getBiomeGenAt(chunkcoordintpair->chunkXPos << 4, chunkcoordintpair->chunkZPos << 4);
}

BiomeGenBase *WorldChunkManager::getBiomeGenAt(int_t i, int_t j)
{
	return getBiomeBlock(i, j, 1, 1)[0];
}

double WorldChunkManager::getTemperature(int_t i, int_t j)
{
	temperature = temperatureNoise->getBlockMaterial(temperature, i, j, 1, 1, 0.02500000037252903, 0.02500000037252903, 0.5);
	return temperature[0];
}

std::vector<BiomeGenBase *> &WorldChunkManager::getBiomeBlock(int_t i, int_t j, int_t k, int_t l)
{
	biomeCache = loadBlockGeneratorData(biomeCache, i, j, k, l);
	return biomeCache;
}

std::vector<double> &WorldChunkManager::getTemperatures(std::vector<double> &ad, int_t i, int_t j, int_t k, int_t l)
{
	if (ad.empty() || (int_t)ad.size() < k * l)
		ad.resize(k * l);
	ad = temperatureNoise->getBlockMaterial(ad, i, j, k, l, 0.02500000037252903, 0.02500000037252903, 0.25);
	modifierValues = noiseModifier->getBlockMaterial(modifierValues, i, j, k, l, 0.25, 0.25, 0.58823529411764708);
	int_t i1 = 0;
	for (int_t j1 = 0; j1 < k; j1++)
	{
		for (int_t k1 = 0; k1 < l; k1++)
		{
			double d  = modifierValues[i1] * 1.1000000000000001 + 0.5;
			double d1 = 0.01;
			double d2 = 1.0 - d1;
			double d3 = (ad[i1] * 0.14999999999999999 + 0.69999999999999996) * d2 + d * d1;
			d3 = 1.0 - (1.0 - d3) * (1.0 - d3);
			if (d3 < 0.0) d3 = 0.0;
			if (d3 > 1.0) d3 = 1.0;
			ad[i1] = d3;
			i1++;
		}
	}
	return ad;
}

std::vector<BiomeGenBase *> WorldChunkManager::loadBlockGeneratorData(std::vector<BiomeGenBase *> abiomegenbase, int_t i, int_t j, int_t k, int_t l)
{
	if (forcedBiome != nullptr)
	{
		size_t n = (size_t)(k * l);
		if (n == 0) n = 1;
		abiomegenbase.assign(n, forcedBiome);
		// BlockGrass/BlockLeaves read these caches right after the call, so they
		// MUST be populated too (forest-like temp/humidity => normal green).
		temperature.assign(n, 0.7);
		humidity.assign(n, 0.8);
		return abiomegenbase;
	}
	if (abiomegenbase.empty() || (int_t)abiomegenbase.size() < k * l)
		abiomegenbase.resize(k * l);
	temperature    = temperatureNoise->getBlockMaterial(temperature, i, j, k, k, 0.02500000037252903, 0.02500000037252903, 0.25);
	humidity       = humidityNoise->getBlockMaterial(humidity, i, j, k, k, 0.05000000074505806, 0.05000000074505806, 0.33333333333333331);
	modifierValues = noiseModifier->getBlockMaterial(modifierValues, i, j, k, k, 0.25, 0.25, 0.58823529411764708);
	int_t i1 = 0;
	for (int_t j1 = 0; j1 < k; j1++)
	{
		for (int_t k1 = 0; k1 < l; k1++)
		{
			double d  = modifierValues[i1] * 1.1000000000000001 + 0.5;
			double d1 = 0.01;
			double d2 = 1.0 - d1;
			double d3 = (temperature[i1] * 0.14999999999999999 + 0.69999999999999996) * d2 + d * d1;
			d1 = 0.002;
			d2 = 1.0 - d1;
			double d4 = (humidity[i1] * 0.14999999999999999 + 0.5) * d2 + d * d1;
			d3 = 1.0 - (1.0 - d3) * (1.0 - d3);
			if (d3 < 0.0) d3 = 0.0;
			if (d4 < 0.0) d4 = 0.0;
			if (d3 > 1.0) d3 = 1.0;
			if (d4 > 1.0) d4 = 1.0;
			temperature[i1] = d3;
			humidity[i1]    = d4;
			abiomegenbase[i1++] = BiomeGenBase::getBiomeFromLookup(d3, d4);
		}
	}
	return abiomegenbase;
}
