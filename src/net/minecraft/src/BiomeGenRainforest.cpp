#include "BiomeGenRainforest.h"
#include "WorldGenBigTree.h"
#include "WorldGenTrees.h"

WorldGenerator *BiomeGenRainforest::getRandomWorldGenForTrees(Random &random)
{
    if (random.nextInt(3) == 0) return new WorldGenBigTree();
    return new WorldGenTrees();
}
