#include "BiomeGenSky.h"
#include <typeindex>
#include "EntityChicken.h"

class EntityChicken;

BiomeGenSky::BiomeGenSky()
{
    spawnableMonsterList.clear();
    spawnableCreatureList.clear();
    spawnableWaterCreatureList.clear();
    spawnableCreatureList.push_back(SpawnListEntry(std::type_index(typeid(EntityChicken)), 10));
}

int_t BiomeGenSky::getSkyColorByTemp(float f)
{
    return 0xc0c0ff;
}
