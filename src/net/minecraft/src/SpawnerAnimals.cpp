#include "SpawnerAnimals.h"

#include <unordered_set>
#include <functional>
#include <map>
#include <typeindex>
#include <cmath>
#include <cstdio>

#include "World.h"
#include "Entity.h"
#include "EntityPlayer.h"
#include "EntityLiving.h"
#include "ChunkPosition.h"
#include "ChunkCoordIntPair.h"
#include "ChunkCoordinates.h"
#include "EnumCreatureType.h"
#include "SpawnListEntry.h"
#include "BiomeGenBase.h"
#include "MathHelper.h"
#include "Material.h"
#include "BlockBed.h"
#include "Pathfinder.h"
#include "PathEntity.h"
#include "PathPoint.h"

// Entity-specific headers for creatureSpecificInit
#include "EntitySpider.h"
#include "EntitySkeleton.h"
#include "EntitySheep.h"
#include "EntityZombie.h"
#include "WorldChunkManager.h"

// Construct-on-first-use: avoids static init order fiasco when EntityList registers
// factories before this TU's statics are initialized.
static std::map<std::type_index, std::function<EntityLiving*(World*)>>& getEntityFactory()
{
    static std::map<std::type_index, std::function<EntityLiving*(World*)>> instance;
    return instance;
}

void SpawnerAnimals_registerFactory(std::type_index idx, std::function<EntityLiving*(World*)> factory)
{
    getEntityFactory()[idx] = factory;
}

std::set<ChunkCoordIntPair> SpawnerAnimals::eligibleChunksForSpawning;

ChunkPosition SpawnerAnimals::getRandomSpawningPointInChunk(World *world, int_t i, int_t j)
{
    int_t k  = i + world->rand.nextInt(16);
    int_t l  = world->rand.nextInt(128);
    int_t i1 = j + world->rand.nextInt(16);
    return ChunkPosition(k, l, i1);
}

int_t SpawnerAnimals::performSpawning(World *world, bool spawnHostiles, bool spawnPeaceful)
{
    if (!spawnHostiles && !spawnPeaceful)
        return 0;

    eligibleChunksForSpawning.clear();

    // Collect chunks within range of every player
    for (int_t pi = 0; pi < (int_t)world->playerEntities.size(); pi++)
    {
        EntityPlayer *player = world->playerEntities[pi];
        int_t cx = MathHelper::floor_double(player->posX / 16.0);
        int_t cz = MathHelper::floor_double(player->posZ / 16.0);
        const int_t radius = 8;

        for (int_t dx = -radius; dx <= radius; dx++)
            for (int_t dz = -radius; dz <= radius; dz++)
                eligibleChunksForSpawning.insert(ChunkCoordIntPair(dx + cx, dz + cz));
    }

    int_t totalSpawned = 0;
    ChunkCoordinates *spawnPoint = world->getSpawnPoint();

    // Keep the canonical instances. Biomes distinguish creature categories by
    // their tag (and historically Java passed the enum singletons themselves),
    // so copying these values can make identity-based lookups fail.
    const EnumCreatureType *creatureTypes[] = {
        &EnumCreatureType::monster,
        &EnumCreatureType::creature,
        &EnumCreatureType::waterCreature
    };

    for (const EnumCreatureType *creatureTypePtr : creatureTypes)
    {
        const EnumCreatureType &creatureType = *creatureTypePtr;
        if (creatureType.getPeacefulCreature() && !spawnPeaceful) continue;
        if (!creatureType.getPeacefulCreature() && !spawnHostiles) continue;

        if (world->countEntities(creatureType.getCreatureTag()) >
            creatureType.getMaxNumberOfCreature() * (int_t)eligibleChunksForSpawning.size() / 256)
            continue;

        for (const ChunkCoordIntPair &chunkPair : eligibleChunksForSpawning)
        {
            BiomeGenBase *biome = world->getWorldChunkManager()->getBiomeGenAtChunkCoord(
                new ChunkCoordIntPair(chunkPair.chunkXPos, chunkPair.chunkZPos));

            std::vector<SpawnListEntry> *spawnList = biome->getSpawnableList(creatureType);
            if (spawnList == nullptr || spawnList->empty())
                continue;

            // Pick a random entry weighted by rarity
            int_t totalRarity = 0;
            for (SpawnListEntry &entry : *spawnList)
                totalRarity += entry.spawnRarityRate;

            int_t rnd = world->rand.nextInt(totalRarity);
            SpawnListEntry *chosen = &(*spawnList)[0];
            for (SpawnListEntry &entry : *spawnList)
            {
                rnd -= entry.spawnRarityRate;
                if (rnd < 0) { chosen = &entry; break; }
            }

            // Get random spawn point in chunk
            ChunkPosition spawnPos = getRandomSpawningPointInChunk(
                world, chunkPair.chunkXPos * 16, chunkPair.chunkZPos * 16);

            int_t bx = spawnPos.x;
            int_t by = spawnPos.y;
            int_t bz = spawnPos.z;

            if (world->isBlockNormalCube(bx, by, bz)) continue;
            if (world->getBlockMaterial(bx, by, bz) != creatureType.getCreatureMaterial()) continue;

            // Try to spawn a cluster of up to 4 entities in 3 attempts
            int_t spawnedInGroup = 0;

            for (int_t attempt = 0; attempt < 3; attempt++)
            {
                int_t ex = bx, ey = by, ez = bz;
                const int_t spread = 6;

                for (int_t sub = 0; sub < 4; sub++)
                {
                    ex += world->rand.nextInt(spread) - world->rand.nextInt(spread);
                    ey += world->rand.nextInt(1)      - world->rand.nextInt(1);
                    ez += world->rand.nextInt(spread) - world->rand.nextInt(spread);

                    if (!canCreatureTypeSpawnAtLocation(creatureType, world, ex, ey, ez))
                        continue;

                    float fx = (float)ex + 0.5f;
                    float fy = (float)ey;
                    float fz = (float)ez + 0.5f;

                    if (world->getClosestPlayer((double)fx, (double)fy, (double)fz, 24.0) != nullptr)
                        continue;

                    float dsx = fx - (float)spawnPoint->x;
                    float dsy = fy - (float)spawnPoint->y;
                    float dsz = fz - (float)spawnPoint->z;
                    if (dsx * dsx + dsy * dsy + dsz * dsz < 576.0f)
                        continue;

                    // Create entity via factory
                    auto factIt = getEntityFactory().find(chosen->entityClass);
                    if (factIt == getEntityFactory().end())
                        continue;

                    EntityLiving *entity = factIt->second(world);
                    if (entity == nullptr)
                        continue;

                    entity->setLocationAndAngles((double)fx, (double)fy, (double)fz,
                        world->rand.nextFloat() * 360.0f, 0.0f);

                    if (entity->getCanSpawnHere())
                    {
                        spawnedInGroup++;
                        world->entityJoinedWorld(entity);
                        creatureSpecificInit(entity, world, fx, fy, fz);

                        if (spawnedInGroup >= entity->getMaxSpawnedInChunk())
                            goto next_chunk;
                    }

                    totalSpawned += spawnedInGroup;
                }
            }

            next_chunk:;
        }
    }

    return totalSpawned;
}

bool SpawnerAnimals::canCreatureTypeSpawnAtLocation(const EnumCreatureType &type, World *world, int_t i, int_t j, int_t k)
{
    if (type.getCreatureMaterial() == Material::water)
        return world->getBlockMaterial(i, j, k)->getIsLiquid() && !world->isBlockNormalCube(i, j + 1, k);

    return world->isBlockNormalCube(i, j - 1, k)
        && !world->isBlockNormalCube(i, j, k)
        && !world->getBlockMaterial(i, j, k)->getIsLiquid()
        && !world->isBlockNormalCube(i, j + 1, k);
}

void SpawnerAnimals::creatureSpecificInit(EntityLiving *entity, World *world, float x, float y, float z)
{
    if (dynamic_cast<EntitySpider*>(entity) && world->rand.nextInt(100) == 0)
    {
        EntitySkeleton *skeleton = new EntitySkeleton(world);
        skeleton->setLocationAndAngles((double)x, (double)y, (double)z, entity->rotationYaw, 0.0f);
        world->entityJoinedWorld(skeleton);
        skeleton->mountEntity(entity);
    }
    else if (EntitySheep *sheep = dynamic_cast<EntitySheep*>(entity))
    {
        sheep->setFleeceColor(EntitySheep::getRandomFleeceColor(world->rand));
    }
}

bool SpawnerAnimals::performSleepSpawning(World *world, std::vector<EntityPlayer*> &playerList)
{
    bool spawned = false;
    Pathfinder pathfinder(world);

    static const std::type_index nightSpawnEntities[] = {
        typeid(EntitySpider),
        typeid(EntityZombie),
        typeid(EntitySkeleton)
    };
    static const int_t nightSpawnCount = 3;

    for (EntityPlayer *player : playerList)
    {
        bool foundSpawn = false;
        int_t tries = 0;

        while (tries < 20 && !foundSpawn)
        {
            int_t bx = (MathHelper::floor_double(player->posX) + world->rand.nextInt(32)) - world->rand.nextInt(32);
            int_t bz = (MathHelper::floor_double(player->posZ) + world->rand.nextInt(32)) - world->rand.nextInt(32);
            int_t by = (MathHelper::floor_double(player->posY) + world->rand.nextInt(16)) - world->rand.nextInt(16);

            if (by < 1)   by = 1;
            if (by > 128) by = 128;

            int_t entityIdx = world->rand.nextInt(nightSpawnCount);
            int_t sy = by;

            while (sy > 2 && !world->isBlockNormalCube(bx, sy - 1, bz))
                sy--;
            while (!canCreatureTypeSpawnAtLocation(EnumCreatureType::monster, world, bx, sy, bz) && sy < by + 16 && sy < 128)
                sy++;

            if (sy >= by + 16 || sy >= 128)
            {
                tries++;
                continue;
            }

            float fx = (float)bx + 0.5f;
            float fy = (float)sy;
            float fz = (float)bz + 0.5f;

            auto factIt = getEntityFactory().find(nightSpawnEntities[entityIdx]);
            if (factIt == getEntityFactory().end()) { tries++; continue; }

            EntityLiving *entity = factIt->second(world);
            if (entity == nullptr) { tries++; continue; }

            entity->setLocationAndAngles((double)fx, (double)fy, (double)fz, world->rand.nextFloat() * 360.0f, 0.0f);

            if (entity->getCanSpawnHere())
            {
                PathEntity *path = pathfinder.createEntityPathTo(entity, player, 32.0f);
                if (path != nullptr && path->pathLength > 1)
                {
                    PathPoint *tip = path->getFinalPathPoint();
                    if (std::abs((double)tip->xCoord - player->posX) < 1.5 &&
                        std::abs((double)tip->zCoord - player->posZ) < 1.5 &&
                        std::abs((double)tip->yCoord - player->posY) < 1.5)
                    {
                        ChunkCoordinates *bedPos = BlockBed::getNearestEmptyChunkCoordinates(
                            world,
                            MathHelper::floor_double(player->posX),
                            MathHelper::floor_double(player->posY),
                            MathHelper::floor_double(player->posZ), 1);

                        if (bedPos == nullptr)
                            bedPos = new ChunkCoordinates(bx, sy + 1, bz);

                        entity->setLocationAndAngles(
                            (float)bedPos->x + 0.5f, (float)bedPos->y, (float)bedPos->z + 0.5f, 0.0f, 0.0f);

                        world->entityJoinedWorld(entity);
                        creatureSpecificInit(entity, world,
                            (float)bedPos->x + 0.5f, (float)bedPos->y, (float)bedPos->z + 0.5f);

                        player->wakeUpPlayer(true, false, false);
                        entity->playLivingSound();

                        spawned = true;
                        foundSpawn = true;
                    }
                }
            }

            tries++;
        }
    }

    return spawned;
}
