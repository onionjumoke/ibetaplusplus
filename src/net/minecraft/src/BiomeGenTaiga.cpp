#include "BiomeGenTaiga.h"
#include "WorldGenTaiga1.h"
#include "WorldGenTaiga2.h"
#include <typeindex>
#include "EntityWolf.h"

class EntityWolf;

BiomeGenTaiga::BiomeGenTaiga()
{
    spawnableCreatureList.push_back(SpawnListEntry(std::type_index(typeid(EntityWolf)), 2));
}

WorldGenerator *BiomeGenTaiga::getRandomWorldGenForTrees(Random &random)
{
    if (random.nextInt(3) == 0) return new WorldGenTaiga1();
    return new WorldGenTaiga2();
}
