#include "BiomeGenHell.h"
#include <typeindex>
#include "EntityGhast.h"
#include "EntityPigZombie.h"

class EntityGhast;
class EntityPigZombie;

BiomeGenHell::BiomeGenHell()
{
    spawnableMonsterList.clear();
    spawnableCreatureList.clear();
    spawnableWaterCreatureList.clear();
    // Only zombie pigmen spawn in the nether (per request): no ghasts.
    spawnableMonsterList.push_back(SpawnListEntry(std::type_index(typeid(EntityPigZombie)), 10));
}
