#pragma once

#include <functional>
#include <set>
#include <typeindex>
#include <vector>
#include "java/Type.h"

class World;
class EntityLiving;
class EnumCreatureType;

// Register a factory so SpawnerAnimals can instantiate that entity type by type_index.
// Must be called before performSpawning / performSleepSpawning.
void SpawnerAnimals_registerFactory(std::type_index idx, std::function<EntityLiving *(World *)> factory);

// net.minecraft.src.SpawnerAnimals
class SpawnerAnimals
{
public:
    SpawnerAnimals() = default;

    static int_t performSpawning(World *world, bool spawnHostiles, bool spawnPeaceful);
    static bool performSleepSpawning(World *world, std::vector<class EntityPlayer*> &list);

protected:
    static class ChunkPosition getRandomSpawningPointInChunk(World *world, int_t chunkX, int_t chunkZ);

private:
    static bool canCreatureTypeSpawnAtLocation(const EnumCreatureType &type, World *world, int_t i, int_t j, int_t k);
    static void creatureSpecificInit(EntityLiving *entity, World *world, float x, float y, float z);

    static std::set<class ChunkCoordIntPair> eligibleChunksForSpawning;
    static const int_t nightSpawnEntityCount = 3;
};
