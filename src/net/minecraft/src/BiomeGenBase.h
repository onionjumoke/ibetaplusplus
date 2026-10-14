#pragma once

#include <vector>
#include "java/Type.h"
#include "java/String.h"
#include "java/Random.h"
#include "SpawnListEntry.h"
#include "EnumCreatureType.h"

class WorldGenerator;

// net.minecraft.src.BiomeGenBase
class BiomeGenBase
{
public:
	static BiomeGenBase *rainforest;
	static BiomeGenBase *swampland;
	static BiomeGenBase *seasonalForest;
	static BiomeGenBase *forest;
	static BiomeGenBase *savanna;
	static BiomeGenBase *shrubland;
	static BiomeGenBase *taiga;
	static BiomeGenBase *desert;
	static BiomeGenBase *plains;
	static BiomeGenBase *iceDesert;
	static BiomeGenBase *tundra;
	static BiomeGenBase *hell;
	static BiomeGenBase *sky;

	static BiomeGenBase *biomeLookupTable[4096];

	static void initialize();
	static void generateBiomeLookup();
	static BiomeGenBase *getBiomeFromLookup(double d, double d1);
	static BiomeGenBase *getBiome(float f, float f1);

	virtual WorldGenerator *getRandomWorldGenForTrees(Random &random);
	virtual int_t getSkyColorByTemp(float f);
	virtual std::vector<SpawnListEntry> *getSpawnableList(const EnumCreatureType &enumcreaturetype);
	bool getEnableSnow() const { return enableSnow; }
	bool canSpawnLightningBolt() const { return !enableSnow && enableRain; }

	jstring biomeName;
	int_t color;
	byte_t topBlock;
	byte_t fillerBlock;
	int_t texturePackSelected;

protected:
	BiomeGenBase();
	virtual ~BiomeGenBase() = default;

	BiomeGenBase *setDisableRain();
	BiomeGenBase *setEnableSnow();
	BiomeGenBase *setBiomeName(const jstring &s);
	BiomeGenBase *getBiomeAtCoords(int_t i);
	BiomeGenBase *setColor(int_t i);

	std::vector<SpawnListEntry> spawnableMonsterList;
	std::vector<SpawnListEntry> spawnableCreatureList;
	std::vector<SpawnListEntry> spawnableWaterCreatureList;

private:
	bool enableSnow = false;
	bool enableRain = true;
};
