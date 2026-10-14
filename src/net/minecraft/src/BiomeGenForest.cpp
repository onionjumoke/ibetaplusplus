#include "BiomeGenForest.h"
#include "WorldGenForest.h"
#include "WorldGenBigTree.h"
#include "WorldGenTrees.h"
#include <typeindex>
#include "EntityWolf.h"

class EntityWolf;

BiomeGenForest::BiomeGenForest()
{
    spawnableCreatureList.push_back(SpawnListEntry(std::type_index(typeid(EntityWolf)), 2));
}

WorldGenerator *BiomeGenForest::getRandomWorldGenForTrees(Random &random)
{
    if (random.nextInt(5) == 0) return new WorldGenForest();
    if (random.nextInt(3) == 0) return new WorldGenBigTree();
    return new WorldGenTrees();
}
