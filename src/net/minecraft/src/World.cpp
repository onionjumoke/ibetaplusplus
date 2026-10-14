#include "World.h"

#include <algorithm>
#include <cstdio>
#include <unordered_set>

#include "java/System.h"
#include "IBlockAccess.h"
#include "WorldProvider.h"
#include "WorldInfo.h"
#include "MapStorage.h"
#include "ISaveHandler.h"
#include "ChunkProvider.h"
#include "EntityPlayer.h"
#include "ChunkProviderLoadOrGenerate.h"
#include "MathHelper.h"
#include "IChunkProvider.h"
#include "IProgressUpdate.h"
#include "Chunk.h"
#include "Material.h"
#include "Block.h"
#include "IWorldAccess.h"
#include "EnumSkyBlock.h"
#include "Vec3D.h"
#include "Entity.h"
#include "AxisAlignedBB.h"
#include "WorldChunkManager.h"
#include "BiomeGenBase.h"
#include "NextTickListEntry.h"
#include "TileEntity.h"
#include "BlockFire.h"
#include "BlockFluid.h"
#include "Explosion.h"
#include "MetadataChunkBlock.h"
#include "SpawnerAnimals.h"
#include "EntityMob.h"
#include "EntityAnimal.h"
#include "EntityWaterMob.h"
#include "ChunkCoordIntPair.h"
#include "EntityLightningBolt.h"
#include "ChunkCache.h"
#include "Pathfinder.h"
#include "ChunkCoordinates.h"
#include "MovingObjectPosition.h"
#include "PathEntity.h"
#include "MapDataBase.h"
#include "NBTTagCompound.h"
#include "EnumCreatureType.h"
#include "Container.h"
#include "ItemStack.h"
#include "EnumStatus.h"
#include "IChunkLoader.h"
#include "BlockLeaves.h"
#include "client/Minecraft.h"
#include "java/String.h"
#include "GameSettings.h"

// Cave ambience is toggleable in World Options. Read it through Minecraft's
// settings so World needs no back-pointer of its own.
static bool caveSoundsEnabled()
{
    Minecraft *mc = Minecraft::getMinecraft();
    return mc == nullptr || mc->gameSettings == nullptr || mc->gameSettings->ofCaveSounds;
}

static bool mobSpawningEnabled()
{
    Minecraft *mc = Minecraft::getMinecraft();
    return mc == nullptr || mc->gameSettings == nullptr || mc->gameSettings->ofMobs;
}

static bool particlesEnabled()
{
    Minecraft *mc = Minecraft::getMinecraft();
    return mc == nullptr || mc->gameSettings == nullptr || mc->gameSettings->ofParticles;
}

// Autosave period, in world ticks.
//
// Vanilla Beta 1.7.3 used 40 ticks (2 s): every two seconds saveWorld() rewrites
// level.dat and pushes dirty chunks through the zlib/region-file path. That is
// cheap against a desktop page cache, but on slower storage it is a long
// synchronous stall: measured 0.55-0.85 s of I/O per autosave on an iPhone 4S,
// i.e. a visible freeze roughly every two seconds of play.
//
// OptiCraft Heritage hit the same wall and made this a platform knob (it uses
// 40 on desktop, 1200 on Wii, and disables runtime autosave on PS2). We keep
// autosave on for iOS but fire it far less often; explicit saves (pausing,
// leaving the world) are unaffected.
#ifdef MC_IOS
static const int_t kAutosavePeriodTicks = 1200; // 60 s
#else
static const int_t kAutosavePeriodTicks = 40;
#endif

// ── Tick-spike bounds (ported from OptiCraft Heritage's console profile) ─────
// NOTE: the lighting-drain knobs (job cap / wall-clock budget / merge scan /
// queue cap) were tried and REVERTED -- see the comment in updatingLighting().
// Only the two below are kept; both were wins with no render-side cost.
static const int_t kRandomTickChunksPerTick     = 5;     // 0 = visit all (vanilla)
static const int_t kRandomTicksPerChunk         = 80;    // vanilla probes/chunk
static const int_t kRandomDisplayProbes         = 250;   // vanilla 1000

static int_t g_nextWorldType = 0;
void World::setNextWorldType(int_t t) { g_nextWorldType = t; }

// Static initialization
int World::lightingUpdatesScheduled = 0;
static constexpr int_t UPDATE_LCG_INCREMENT = 1013904223;

// Java's World only drops references to removed entities and lets the GC decide when
// to destroy them.  Minecraft keeps independent strong references to the local player
// and camera, so the C++ World must not delete either object while those references are
// still active.
static bool isActiveClientEntity(Entity *entity)
{
    Minecraft *mc = Minecraft::getMinecraft();
    return mc != nullptr &&
           (static_cast<void *>(mc->thePlayer) == static_cast<void *>(entity) ||
            static_cast<void *>(mc->renderViewEntity) == static_cast<void *>(entity));
}

static void deleteWorldOwnedEntity(Entity *entity)
{
    if (!isActiveClientEntity(entity))
        delete entity;
}

WorldChunkManager* World::getWorldChunkManager()
{
    return worldProvider->worldChunkMgr;
}

World::~World()
{
    // The same object can appear in the world list, a chunk list and an unload
    // queue.  Destroy each allocation once, after providers no longer use it.
    std::unordered_set<Entity *> entities;
    entities.insert(loadedEntityList.begin(), loadedEntityList.end());
    entities.insert(unloadedEntityList.begin(), unloadedEntityList.end());
    entities.insert(weatherEffects.begin(), weatherEffects.end());
    entities.insert(playerEntities.begin(), playerEntities.end());

    std::unordered_set<TileEntity *> tileEntities;
    tileEntities.insert(loadedTileEntityList.begin(), loadedTileEntityList.end());
    tileEntities.insert(tileEntitiesToAdd.begin(), tileEntitiesToAdd.end());

    delete chunkProvider;
    chunkProvider = nullptr;

    for (Entity *entity : entities)
        deleteWorldOwnedEntity(entity);
    for (TileEntity *tileEntity : tileEntities)
        delete tileEntity;
    for (MetadataChunkBlock *update : lightingToUpdate)
        delete update;
    for (NextTickListEntry *entry : scheduledTickTreeSet)
        delete entry;
    for (ChunkCoordIntPair *position : positionsToUpdate)
        delete position;

    delete worldProvider;
    delete worldInfo;
    if (ownsMapStorage)
        delete mapStorage;
    if (ownsSaveHandler)
        delete saveHandler;
}

World::World(ISaveHandler* saveHandler, const jstring& name, WorldProvider* worldProvider, long_t seed)
{
    scheduledUpdatesAreImmediate = false;
    lightingToUpdate.clear();
    loadedEntityList.clear();
    unloadedEntityList.clear();
    scheduledTickTreeSet.clear();
    scheduledTickSet.clear();
    loadedTileEntityList.clear();
    tileEntitiesToAdd.clear();
    playerEntities.clear();
    weatherEffects.clear();
    field_1019_F = 0xffffffL;
    skylightSubtracted = 0;
    updateLCG = rand.nextInt();
    thunderTime = 0;
    field_27172_i = 0;
    editingBlocks = false;
    lockTimestamp = System::currentTimeMillis();
    autosavePeriod = kAutosavePeriodTicks;
    isNewWorld = false;
    worldAccesses.clear();
    collidingBoundingBoxes.clear();
    lightingUpdatesCounter = 0;
    spawnHostileMobs = true;
    spawnPeacefulMobs = true;
    positionsToUpdate.clear();
    soundCounter = rand.nextInt(12000);
    entitiesWithinAABBExcludingEntity.clear();
    multiplayerWorld = false;
    
    this->saveHandler = saveHandler;
    this->worldInfo = new WorldInfo(seed, name);
    this->worldProvider = worldProvider;
    this->mapStorage = new MapStorage(saveHandler);
    
    worldProvider->registerWorld(this);
    chunkProvider = getChunkProvider();
    calculateInitialSkylight();
    func_27163_E();
}

World::World(World* world, WorldProvider* worldProvider)
{
    scheduledUpdatesAreImmediate = false;
    lightingToUpdate.clear();
    loadedEntityList.clear();
    unloadedEntityList.clear();
    scheduledTickTreeSet.clear();
    scheduledTickSet.clear();
    loadedTileEntityList.clear();
    tileEntitiesToAdd.clear();
    playerEntities.clear();
    weatherEffects.clear();
    field_1019_F = 0xffffffL;
    skylightSubtracted = 0;
    updateLCG = rand.nextInt();
    thunderTime = 0;
    field_27172_i = 0;
    editingBlocks = false;
    lockTimestamp = /*System.currentTimeMillis()*/ 0;
    autosavePeriod = kAutosavePeriodTicks;
    isNewWorld = false;
    worldAccesses.clear();
    collidingBoundingBoxes.clear();
    lightingUpdatesCounter = 0;
    spawnHostileMobs = true;
    spawnPeacefulMobs = true;
    positionsToUpdate.clear();
    soundCounter = rand.nextInt(12000);
    entitiesWithinAABBExcludingEntity.clear();
    multiplayerWorld = false;
    
    this->lockTimestamp = world->lockTimestamp;
    this->saveHandler = world->saveHandler;
    this->ownsSaveHandler = world->ownsSaveHandler;
    world->ownsSaveHandler = false;
    this->worldInfo = new WorldInfo(world->worldInfo);
    this->mapStorage = new MapStorage(saveHandler);
    this->worldProvider = worldProvider;
    
    worldProvider->registerWorld(this);
    chunkProvider = getChunkProvider();
    calculateInitialSkylight();
    func_27163_E();
}

void World::setMapStorage(MapStorage *storage)
{
    if (ownsMapStorage)
        delete mapStorage;
    mapStorage = storage;
    ownsMapStorage = false;
}

World::World(ISaveHandler* saveHandler, const jstring& name, long_t seed)
    : World(saveHandler, name, seed, nullptr)
{
}

World::World(ISaveHandler* saveHandler, const jstring& name, long_t seed, WorldProvider* worldProvider)
{
    scheduledUpdatesAreImmediate = false;
    lightingToUpdate.clear();
    loadedEntityList.clear();
    unloadedEntityList.clear();
    scheduledTickTreeSet.clear();
    scheduledTickSet.clear();
    loadedTileEntityList.clear();
    tileEntitiesToAdd.clear();
    playerEntities.clear();
    weatherEffects.clear();
    field_1019_F = 0xffffffL;
    skylightSubtracted = 0;
    updateLCG = rand.nextInt();
    thunderTime = 0;
    field_27172_i = 0;
    editingBlocks = false;
    lockTimestamp = 0; // TODO: SDL_GetTicks()
    autosavePeriod = kAutosavePeriodTicks;
    isNewWorld = false;
    worldAccesses.clear();
    collidingBoundingBoxes.clear();
    lightingUpdatesCounter = 0;
    spawnHostileMobs = true;
    spawnPeacefulMobs = true;
    positionsToUpdate.clear();
    soundCounter = rand.nextInt(12000);
    entitiesWithinAABBExcludingEntity.clear();
    multiplayerWorld = false;
    
    this->saveHandler = saveHandler;
    this->mapStorage = new MapStorage(saveHandler);
    this->worldInfo = saveHandler->loadWorldInfo();
    this->isNewWorld = (this->worldInfo == nullptr);
    
    if (worldProvider != nullptr)
    {
        this->worldProvider = worldProvider;
    }
    else if (this->worldInfo != nullptr && this->worldInfo->getDimension() == -1)
    {
        this->worldProvider = WorldProvider::getProviderForDimension(-1);
    }
    else
    {
        this->worldProvider = WorldProvider::getProviderForDimension(0);
    }
    
    bool flag = false;
    if (this->worldInfo == nullptr)
    {
        this->worldInfo = new WorldInfo(seed, name);
        this->worldInfo->generatorType = g_nextWorldType;
        flag = true;
    }
    else
    {
        this->worldInfo->setWorldName(name);
    }
    
    this->worldProvider->registerWorld(this);
    this->chunkProvider = getChunkProvider();

    (void)0;
    
    if (flag)
    {
        getInitialSpawnLocation();
    }
    
    calculateInitialSkylight();
    func_27163_E();
}

IChunkProvider* World::getChunkProvider()
{
    IChunkLoader* chunkLoader = saveHandler->getChunkLoader(worldProvider);
    return new ChunkProvider(this, chunkLoader, worldProvider->getChunkProvider());
}

void World::getInitialSpawnLocation()
{
    findingSpawnPoint = true;
    int x = 0;
    int y = 64;
    int z = 0;
    
    while (!worldProvider->canCoordinateBeSpawn(x, z))
    {
        x += rand.nextInt(64) - rand.nextInt(64);
        z += rand.nextInt(64) - rand.nextInt(64);
    }
    
    worldInfo->setSpawn(x, y, z);
    findingSpawnPoint = false;
}

void World::setSpawnLocation()
{
    if (worldInfo->getSpawnY() <= 0)
    {
        worldInfo->setSpawnY(64);
    }
    
    int x = worldInfo->getSpawnX();
    int z = worldInfo->getSpawnZ();
    
    while (getFirstUncoveredBlock(x, z) == 0)
    {
        x += rand.nextInt(8) - rand.nextInt(8);
        z += rand.nextInt(8) - rand.nextInt(8);
    }
    
    worldInfo->setSpawnX(x);
    worldInfo->setSpawnZ(z);
}

int World::getFirstUncoveredBlock(int x, int z)
{
    int y = 63;
    while (!isAirBlock(x, y + 1, z))
    {
        y++;
    }
    return getBlockId(x, y, z);
}

void World::emptyMethod1()
{
}

void World::spawnPlayerWithLoadedChunks(EntityPlayer* entityPlayer)
{
    try
    {
        NBTTagCompound* nbt = worldInfo->getPlayerNBTTagCompound();
        if (nbt != nullptr)
        {
            entityPlayer->readFromNBT(nbt);
            worldInfo->setPlayerNBTTagCompound(nullptr);
        }
        
        if (dynamic_cast<ChunkProviderLoadOrGenerate*>(chunkProvider) != nullptr)
        {
            ChunkProviderLoadOrGenerate* chunkProviderLoadOrGenerate = static_cast<ChunkProviderLoadOrGenerate*>(chunkProvider);
            int chunkX = MathHelper::floor_float((int)entityPlayer->posX) >> 4;
            int chunkZ = MathHelper::floor_float((int)entityPlayer->posZ) >> 4;
            chunkProviderLoadOrGenerate->setCurrentChunkOver(chunkX, chunkZ);
        }
        
        entityJoinedWorld(entityPlayer);
    }
    catch (...)
    {
        // exception.printStackTrace();
    }
}

void World::saveWorld(bool flag, IProgressUpdate* progressUpdate)
{
    if (!chunkProvider->canSave())
    {
        return;
    }
    
    if (progressUpdate != nullptr)
    {
        progressUpdate->resetProgressAndMessage("Saving level");
    }
    
    long_t swT0 = System::currentTimeMillis();
    saveLevel();
    long_t swT1 = System::currentTimeMillis();
    
    if (progressUpdate != nullptr)
    {
        progressUpdate->displayLoadingString("Saving chunks");
    }
    
    chunkProvider->saveChunks(flag, progressUpdate);
    long_t swT2 = System::currentTimeMillis();
    if (swT2 - swT0 >= 15)
        (void)0;
}

void World::saveLevel()
{
    checkSessionLock();
    long_t sl0 = System::currentTimeMillis();
    saveHandler->saveWorldInfoAndPlayer(worldInfo, playerEntities);
    long_t sl1 = System::currentTimeMillis();
    mapStorage->saveAllData();
    long_t sl2 = System::currentTimeMillis();
    if (sl2 - sl0 >= 15)
        (void)0;
}

bool World::saveAllChunks(int i)
{
    if (!chunkProvider->canSave())
    {
        return true;
    }
    
    if (i == 0)
    {
        saveLevel();
    }
    
    return chunkProvider->saveChunks(false, nullptr);
}

void World::unstickPlayer(EntityPlayer *player)
{
    if (player == nullptr)
        return;

    const double x = player->posX;
    const double z = player->posZ;

    auto boxFree = [this](double px, double py, double pz) -> bool
    {
        const double half = 0.31;
        const double height = 1.8;
        const int minBX = MathHelper::floor_double(px - half);
        const int maxBX = MathHelper::floor_double(px + half);
        const int minBY = MathHelper::floor_double(py);
        const int maxBY = MathHelper::floor_double(py + height);
        const int minBZ = MathHelper::floor_double(pz - half);
        const int maxBZ = MathHelper::floor_double(pz + half);
        for (int bx = minBX; bx <= maxBX; bx++)
            for (int by = minBY; by <= maxBY; by++)
                for (int bz = minBZ; bz <= maxBZ; bz++)
                {
                    Block *block = Block::blocksList[getBlockId(bx, by, bz)];
                    if (block != nullptr && block->blockMaterial != nullptr && block->blockMaterial->getIsSolid())
                        return false;
                }
        return true;
    };

    for (int dy = 0; dy <= 8; dy++)
    {
        if (boxFree(x, player->posY + dy, z))
        {
            player->setPosition(x, player->posY + dy + 0.01, z);
            player->motionX = player->motionY = player->motionZ = 0.0;
            return;
        }
    }

    // Last resort: drop in from above and let gravity settle them.
    player->setPosition(x, 127.0, z);
    player->motionX = player->motionY = player->motionZ = 0.0;
}

int World::getBlockId(int x, int y, int z)
{
    if (x < -0x01e84800 || z < -0x01e84800 || x >= 0x01e84800 || z >= 0x01e84800)
    {
        return 0;
    }
    
    if (y < 0)
    {
        return 0;
    }
    
    if (y >= 128)
    {
        return 0;
    }
    
    Chunk *c = getChunkFromChunkCoords(x >> 4, z >> 4);
    return c != nullptr ? c->getBlockID(x & 0xf, y, z & 0xf) : 0;
}

bool World::isAirBlock(int x, int y, int z)
{
    return getBlockId(x, y, z) == 0;
}

bool World::blockExists(int x, int y, int z)
{
    if (y < 0 || y >= 128)
    {
        return false;
    }
    
    return chunkExists(x >> 4, z >> 4);
}

bool World::doChunksNearChunkExist(int x, int y, int z, int range)
{
    return checkChunksExist(x - range, y - range, z - range, x + range, y + range, z + range);
}

bool World::checkChunksExist(int minX, int minY, int minZ, int maxX, int maxY, int maxZ)
{
    if (maxY < 0 || minY >= 128)
    {
        return false;
    }
    
    minX >>= 4;
    minY >>= 4;
    minZ >>= 4;
    maxX >>= 4;
    maxY >>= 4;
    maxZ >>= 4;
    
    for (int cx = minX; cx <= maxX; cx++)
    {
        for (int cz = minZ; cz <= maxZ; cz++)
        {
            if (!chunkExists(cx, cz))
            {
                return false;
            }
        }
    }
    
    return true;
}

bool World::chunkExists(int chunkX, int chunkZ)
{
    return chunkProvider->chunkExists(chunkX, chunkZ);
}

Chunk* World::getChunkFromBlockCoords(int x, int z)
{
    return getChunkFromChunkCoords(x >> 4, z >> 4);
}

Chunk* World::getChunkFromChunkCoords(int chunkX, int chunkZ)
{
    return chunkProvider->provideChunk(chunkX, chunkZ);
}

bool World::setBlockAndMetadata(int x, int y, int z, int blockId, int metadata)
{
    if (x < -0x01e84800 || z < -0x01e84800 || x >= 0x01e84800 || z >= 0x01e84800)
    {
        return false;
    }
    
    if (y < 0)
    {
        return false;
    }
    
    if (y >= 128)
    {
        return false;
    }
    
    Chunk* chunk = getChunkFromChunkCoords(x >> 4, z >> 4);
    return chunk->setBlockIDWithMetadata(x & 0xf, y, z & 0xf, blockId, metadata);
}

bool World::setBlock(int x, int y, int z, int blockId)
{
    if (x < -0x01e84800 || z < -0x01e84800 || x >= 0x01e84800 || z >= 0x01e84800)
    {
        return false;
    }
    
    if (y < 0)
    {
        return false;
    }
    
    if (y >= 128)
    {
        return false;
    }
    
    Chunk* chunk = getChunkFromChunkCoords(x >> 4, z >> 4);
    if (chunk == nullptr)
        return false; // decoration may touch not-yet-created neighbors
    return chunk->setBlockID(x & 0xf, y, z & 0xf, blockId);
}

Material* World::getBlockMaterial(int x, int y, int z)
{
    int blockId = getBlockId(x, y, z);
    if (blockId == 0)
    {
        return Material::air;
    }
    
    return Block::blocksList[blockId]->blockMaterial;
}

int World::getBlockMetadata(int x, int y, int z)
{
    if (x < -0x01e84800 || z < -0x01e84800 || x >= 0x01e84800 || z >= 0x01e84800)
    {
        return 0;
    }
    
    if (y < 0)
    {
        return 0;
    }
    
    if (y >= 128)
    {
        return 0;
    }
    
    Chunk* chunk = getChunkFromChunkCoords(x >> 4, z >> 4);
    x &= 0xf;
    z &= 0xf;
    return chunk->getBlockMetadata(x, y, z);
}

void World::setBlockMetadataWithNotify(int x, int y, int z, int metadata)
{
    if (setBlockMetadata(x, y, z, metadata))
    {
        int blockId = getBlockId(x, y, z);
        if (Block::disableNeighborNotifyOnMetadataChangeLookup[blockId & 0xff])
        {
            notifyBlockChange(x, y, z, blockId);
        }
        else
        {
            notifyBlocksOfNeighborChange(x, y, z, blockId);
        }
    }
}

bool World::setBlockMetadata(int x, int y, int z, int metadata)
{
    if (x < -0x01e84800 || z < -0x01e84800 || x >= 0x01e84800 || z >= 0x01e84800)
    {
        return false;
    }
    
    if (y < 0)
    {
        return false;
    }
    
    if (y >= 128)
    {
        return false;
    }
    
    Chunk* chunk = getChunkFromChunkCoords(x >> 4, z >> 4);
    x &= 0xf;
    z &= 0xf;
    chunk->setBlockMetadata(x, y, z, metadata);
    return true;
}

bool World::setBlockWithNotify(int x, int y, int z, int blockId)
{
    if (setBlock(x, y, z, blockId))
    {
        notifyBlockChange(x, y, z, blockId);
        return true;
    }
    
    return false;
}

bool World::setBlockAndMetadataWithNotify(int x, int y, int z, int blockId, int metadata)
{
    if (setBlockAndMetadata(x, y, z, blockId, metadata))
    {
        notifyBlockChange(x, y, z, blockId);
        return true;
    }
    
    return false;
}

void World::markBlockNeedsUpdate(int x, int y, int z)
{
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->markBlockAndNeighborsNeedsUpdate(x, y, z);
    }
}

void World::notifyBlockChange(int x, int y, int z, int blockId)
{
    markBlockNeedsUpdate(x, y, z);
    notifyBlocksOfNeighborChange(x, y, z, blockId);
}

void World::markBlocksDirtyVertical(int x, int z, int y1, int y2)
{
    if (y1 > y2)
    {
        int temp = y1;
        y1 = y2;
        y2 = temp;
    }
    
    markBlocksDirty(x, y1, z, x, y2, z);
}

void World::markBlockAsNeedsUpdate(int x, int y, int z)
{
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->markBlockRangeNeedsUpdate(x, y, z, x, y, z);
    }
}

void World::markBlocksDirty(int minX, int minY, int minZ, int maxX, int maxY, int maxZ)
{
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->markBlockRangeNeedsUpdate(minX, minY, minZ, maxX, maxY, maxZ);
    }
}

void World::notifyBlocksOfNeighborChange(int x, int y, int z, int blockId)
{
    notifyBlockOfNeighborChange(x - 1, y, z, blockId);
    notifyBlockOfNeighborChange(x + 1, y, z, blockId);
    notifyBlockOfNeighborChange(x, y - 1, z, blockId);
    notifyBlockOfNeighborChange(x, y + 1, z, blockId);
    notifyBlockOfNeighborChange(x, y, z - 1, blockId);
    notifyBlockOfNeighborChange(x, y, z + 1, blockId);
}

void World::notifyBlockOfNeighborChange(int x, int y, int z, int blockId)
{
    if (editingBlocks || multiplayerWorld)
    {
        return;
    }
    
    Block* block = Block::blocksList[getBlockId(x, y, z)];
    if (block != nullptr)
    {
        block->onNeighborBlockChange(this, x, y, z, blockId);
    }
}

bool World::canBlockSeeTheSky(int x, int y, int z)
{
    return getChunkFromChunkCoords(x >> 4, z >> 4)->canBlockSeeTheSky(x & 0xf, y, z & 0xf);
}

int World::getFullBlockLightValue(int x, int y, int z)
{
    if (y < 0)
    {
        return 0;
    }
    
    if (y >= 128)
    {
        y = 127;
    }
    
    return getChunkFromChunkCoords(x >> 4, z >> 4)->getBlockLightValue(x & 0xf, y, z & 0xf, 0);
}

int World::getBlockLightValue(int x, int y, int z)
{
    return getBlockLightValue_do(x, y, z, true);
}

int World::getBlockLightValue_do(int x, int y, int z, bool flag)
{
    if (x < -0x01e84800 || z < -0x01e84800 || x >= 0x01e84800 || z >= 0x01e84800)
    {
        return 15;
    }
    
    if (flag)
    {
        int blockId = getBlockId(x, y, z);
        // TODO: Check for stairs, farmland, etc. that need special light handling
        // Block::stairSingle, Block::tilledField, etc.
        if (blockId == Block::stairSingle->blockID || 
            blockId == Block::tilledField->blockID ||
            blockId == Block::stairCompactCobblestone->blockID ||
            blockId == Block::stairCompactPlanks->blockID)
        {
            int lightValue = getBlockLightValue_do(x, y + 1, z, false);
            lightValue = std::max(lightValue, getBlockLightValue_do(x + 1, y, z, false));
            lightValue = std::max(lightValue, getBlockLightValue_do(x - 1, y, z, false));
            lightValue = std::max(lightValue, getBlockLightValue_do(x, y, z + 1, false));
            lightValue = std::max(lightValue, getBlockLightValue_do(x, y, z - 1, false));
            return lightValue;
        }
    }
    
    if (y < 0)
    {
        return 0;
    }
    
    if (y >= 128)
    {
        y = 127;
    }
    
    Chunk* chunk = getChunkFromChunkCoords(x >> 4, z >> 4);
    x &= 0xf;
    z &= 0xf;
    return chunk->getBlockLightValue(x, y, z, skylightSubtracted);
}

bool World::canExistingBlockSeeTheSky(int x, int y, int z)
{
    if (x < -0x01e84800 || z < -0x01e84800 || x >= 0x01e84800 || z >= 0x01e84800)
    {
        return false;
    }
    
    if (y < 0)
    {
        return false;
    }
    
    if (y >= 128)
    {
        return true;
    }
    
    if (!chunkExists(x >> 4, z >> 4))
    {
        return false;
    }
    
    Chunk* chunk = getChunkFromChunkCoords(x >> 4, z >> 4);
    x &= 0xf;
    z &= 0xf;
    return chunk->canBlockSeeTheSky(x, y, z);
}

int World::getHeightValue(int x, int z)
{
    if (x < -0x01e84800 || z < -0x01e84800 || x >= 0x01e84800 || z >= 0x01e84800)
    {
        return 0;
    }
    
    if (!chunkExists(x >> 4, z >> 4))
    {
        return 0;
    }
    
    Chunk* chunk = getChunkFromChunkCoords(x >> 4, z >> 4);
    return chunk->getHeightValue(x & 0xf, z & 0xf);
}

void World::neighborLightPropagationChanged(EnumSkyBlock* enumSkyBlock, int x, int y, int z, int lightValue)
{
    if (worldProvider->hasNoSky && enumSkyBlock == EnumSkyBlock::Sky)
    {
        return;
    }
    
    if (!blockExists(x, y, z))
    {
        return;
    }
    
    if (enumSkyBlock == EnumSkyBlock::Sky)
    {
        if (canExistingBlockSeeTheSky(x, y, z))
        {
            lightValue = 15;
        }
    }
    else if (enumSkyBlock == EnumSkyBlock::Block)
    {
        int blockId = getBlockId(x, y, z);
        if (Block::lightValue[blockId] > lightValue)
        {
            lightValue = Block::lightValue[blockId];
        }
    }
    
    if (getSavedLightValue(enumSkyBlock, x, y, z) != lightValue)
    {
        scheduleLightingUpdate(enumSkyBlock, x, y, z, x, y, z);
    }
}

int World::getSavedLightValue(EnumSkyBlock* enumSkyBlock, int x, int y, int z)
{
    if (y < 0)
    {
        y = 0;
    }
    
    if (y >= 128)
    {
        y = 127;
    }
    
    if (y < 0 || y >= 128 || x < -0x01e84800 || z < -0x01e84800 || x >= 0x01e84800 || z >= 0x01e84800)
    {
        return enumSkyBlock->defaultLightValue;
    }
    
    int chunkX = x >> 4;
    int chunkZ = z >> 4;
    
    if (!chunkExists(chunkX, chunkZ))
    {
        return 0;
    }
    
    Chunk* chunk = getChunkFromChunkCoords(chunkX, chunkZ);
    return chunk->getSavedLightValue(enumSkyBlock, x & 0xf, y, z & 0xf);
}

void World::setLightValue(EnumSkyBlock* enumSkyBlock, int x, int y, int z, int lightValue)
{
    if (x < -0x01e84800 || z < -0x01e84800 || x >= 0x01e84800 || z >= 0x01e84800)
    {
        return;
    }
    
    if (y < 0)
    {
        return;
    }
    
    if (y >= 128)
    {
        return;
    }
    
    if (!chunkExists(x >> 4, z >> 4))
    {
        return;
    }
    
    Chunk* chunk = getChunkFromChunkCoords(x >> 4, z >> 4);
    chunk->setLightValue(enumSkyBlock, x & 0xf, y, z & 0xf, lightValue);
    
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->markBlockAndNeighborsNeedsUpdate(x, y, z);
    }
}

float World::getBrightness(int x, int y, int z, int minLight)
{
    int lightValue = getBlockLightValue(x, y, z);
    if (lightValue < minLight)
    {
        lightValue = minLight;
    }
    
    return worldProvider->lightBrightnessTable[lightValue];
}

float World::getLightBrightness(int x, int y, int z)
{
    return worldProvider->lightBrightnessTable[getBlockLightValue(x, y, z)];
}

bool World::isDaytime()
{
    return skylightSubtracted < 4;
}

MovingObjectPosition* World::rayTraceBlocks(Vec3D* vec1, Vec3D* vec2)
{
    return rayTraceBlocks(vec1, vec2, false, false);
}

MovingObjectPosition* World::rayTraceBlocks_do(Vec3D* vec1, Vec3D* vec2, bool flag)
{
    return rayTraceBlocks(vec1, vec2, flag, false);
}

MovingObjectPosition* World::rayTraceBlocks(Vec3D* vec1, Vec3D* vec2, bool flag, bool flag1)
{
    if (std::isnan(vec1->xCoord) || std::isnan(vec1->yCoord) || std::isnan(vec1->zCoord))
    {
        return nullptr;
    }
    
    if (std::isnan(vec2->xCoord) || std::isnan(vec2->yCoord) || std::isnan(vec2->zCoord))
    {
        return nullptr;
    }
    
    int endX = MathHelper::floor_double(vec2->xCoord);
    int endY = MathHelper::floor_double(vec2->yCoord);
    int endZ = MathHelper::floor_double(vec2->zCoord);
    int startX = MathHelper::floor_double(vec1->xCoord);
    int startY = MathHelper::floor_double(vec1->yCoord);
    int startZ = MathHelper::floor_double(vec1->zCoord);
    int blockId = getBlockId(startX, startY, startZ);
    int metadata = getBlockMetadata(startX, startY, startZ);
    Block* block = Block::blocksList[blockId];
    
    if ((!flag1 || block == nullptr || block->getCollisionBoundingBoxFromPool(this, startX, startY, startZ) != nullptr) 
        && blockId > 0 && block->canCollideCheck(metadata, flag))
    {
        MovingObjectPosition* movingObjectPosition = block->collisionRayTrace(this, startX, startY, startZ, vec1, vec2);
        if (movingObjectPosition != nullptr)
        {
            return movingObjectPosition;
        }
    }
    
    for (int l1 = 200; l1-- >= 0;)
    {
        if (std::isnan(vec1->xCoord) || std::isnan(vec1->yCoord) || std::isnan(vec1->zCoord))
        {
            return nullptr;
        }
        
        if (startX == endX && startY == endY && startZ == endZ)
        {
            return nullptr;
        }
        
        bool flag2 = true;
        bool flag3 = true;
        bool flag4 = true;
        double d = 999.0;
        double d1 = 999.0;
        double d2 = 999.0;
        
        if (endX > startX)
        {
            d = (double)startX + 1.0;
        }
        else if (endX < startX)
        {
            d = (double)startX + 0.0;
        }
        else
        {
            flag2 = false;
        }
        
        if (endY > startY)
        {
            d1 = (double)startY + 1.0;
        }
        else if (endY < startY)
        {
            d1 = (double)startY + 0.0;
        }
        else
        {
            flag3 = false;
        }
        
        if (endZ > startZ)
        {
            d2 = (double)startZ + 1.0;
        }
        else if (endZ < startZ)
        {
            d2 = (double)startZ + 0.0;
        }
        else
        {
            flag4 = false;
        }
        
        double d3 = 999.0;
        double d4 = 999.0;
        double d5 = 999.0;
        double d6 = vec2->xCoord - vec1->xCoord;
        double d7 = vec2->yCoord - vec1->yCoord;
        double d8 = vec2->zCoord - vec1->zCoord;
        
        if (flag2)
        {
            d3 = (d - vec1->xCoord) / d6;
        }
        
        if (flag3)
        {
            d4 = (d1 - vec1->yCoord) / d7;
        }
        
        if (flag4)
        {
            d5 = (d2 - vec1->zCoord) / d8;
        }
        
        unsigned char byte0 = 0;
        
        if (d3 < d4 && d3 < d5)
        {
            if (endX > startX)
            {
                byte0 = 4;
            }
            else
            {
                byte0 = 5;
            }
            
            vec1->xCoord = d;
            vec1->yCoord += d7 * d3;
            vec1->zCoord += d8 * d3;
        }
        else if (d4 < d5)
        {
            if (endY > startY)
            {
                byte0 = 0;
            }
            else
            {
                byte0 = 1;
            }
            
            vec1->xCoord += d6 * d4;
            vec1->yCoord = d1;
            vec1->zCoord += d8 * d4;
        }
        else
        {
            if (endZ > startZ)
            {
                byte0 = 2;
            }
            else
            {
                byte0 = 3;
            }
            
            vec1->xCoord += d6 * d5;
            vec1->yCoord += d7 * d5;
            vec1->zCoord = d2;
        }
        
        Vec3D* vec3d2 = Vec3D::createVector(vec1->xCoord, vec1->yCoord, vec1->zCoord);
        startX = (int)(vec3d2->xCoord = MathHelper::floor_double(vec1->xCoord));
        
        if (byte0 == 5)
        {
            startX--;
            vec3d2->xCoord++;
        }
        
        startY = (int)(vec3d2->yCoord = MathHelper::floor_double(vec1->yCoord));
        
        if (byte0 == 1)
        {
            startY--;
            vec3d2->yCoord++;
        }
        
        startZ = (int)(vec3d2->zCoord = MathHelper::floor_double(vec1->zCoord));
        
        if (byte0 == 3)
        {
            startZ--;
            vec3d2->zCoord++;
        }
        
        int j2 = getBlockId(startX, startY, startZ);
        int k2 = getBlockMetadata(startX, startY, startZ);
        Block* block1 = Block::blocksList[j2];
        
        if ((!flag1 || block1 == nullptr || block1->getCollisionBoundingBoxFromPool(this, startX, startY, startZ) != nullptr) 
            && j2 > 0 && block1->canCollideCheck(k2, flag))
        {
            MovingObjectPosition* movingObjectPosition1 = block1->collisionRayTrace(this, startX, startY, startZ, vec1, vec2);
            if (movingObjectPosition1 != nullptr)
            {
                return movingObjectPosition1;
            }
        }
    }
    
    return nullptr;
}

void World::playSoundAtEntity(Entity* entity, const jstring& soundName, float volume, float pitch)
{
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->playSound(soundName, entity->posX, entity->posY - (double)entity->yOffset, entity->posZ, volume, pitch);
    }
}

void World::playSoundEffect(double x, double y, double z, const jstring& soundName, float volume, float pitch)
{
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->playSound(soundName, x, y, z, volume, pitch);
    }
}

void World::playRecord(const jstring& recordName, int x, int y, int z)
{
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->playRecord(recordName, x, y, z);
    }
}

void World::spawnParticle(const jstring& particleName, double x, double y, double z, double velX, double velY, double velZ)
{
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->spawnParticle(particleName, x, y, z, velX, velY, velZ);
    }
}

bool World::addWeatherEffect(Entity* entity)
{
    weatherEffects.push_back(entity);
    return true;
}

bool World::entityJoinedWorld(Entity* entity)
{
    int chunkX = MathHelper::floor_double(entity->posX / 16.0);
    int chunkZ = MathHelper::floor_double(entity->posZ / 16.0);
    bool flag = false;
    
    if (dynamic_cast<EntityPlayer*>(entity) != nullptr)
    {
        flag = true;
    }
    
    if (flag || chunkExists(chunkX, chunkZ))
    {
        // Java can harmlessly keep the same object in an ArrayList more than once:
        // removing one reference does not destroy the object while another reference
        // still exists.  In C++ loadedEntityList owns its entries, so a duplicate turns
        // into a dangling pointer as soon as updateEntities() deletes the first copy.
        // changeWorld() exercises this path by joining the transferred player and then
        // calling spawnPlayerWithLoadedChunks() for that same player.
        if (std::find(loadedEntityList.begin(), loadedEntityList.end(), entity) != loadedEntityList.end())
            return true;

        if (dynamic_cast<EntityPlayer*>(entity) != nullptr)
        {
            EntityPlayer* entityPlayer = static_cast<EntityPlayer*>(entity);
            playerEntities.push_back(entityPlayer);
            updateAllPlayersSleepingFlag();
        }
        
        getChunkFromChunkCoords(chunkX, chunkZ)->addEntity(entity);
        loadedEntityList.push_back(entity);
        obtainEntitySkin(entity);
        return true;
    }
    
    return false;
}

void World::obtainEntitySkin(Entity* entity)
{
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->obtainEntitySkin(entity);
    }
}

void World::releaseEntitySkin(Entity* entity)
{
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->releaseEntitySkin(entity);
    }
}

void World::onEntityRemoved(Entity* entity)
{
    // No-op in base World; WorldClient overrides this to scrub its non-owning
    // entity indexes before the pointer is freed.
}

void World::setEntityDead(Entity* entity)
{
    if (entity->riddenByEntity != nullptr)
    {
        entity->riddenByEntity->mountEntity(nullptr);
    }
    
    if (entity->ridingEntity != nullptr)
    {
        entity->mountEntity(nullptr);
    }
    
    entity->setEntityDead();
    
    if (dynamic_cast<EntityPlayer*>(entity) != nullptr)
    {
        auto it = std::find(playerEntities.begin(), playerEntities.end(), static_cast<EntityPlayer*>(entity));
        if (it != playerEntities.end())
        {
            playerEntities.erase(it);
        }
        updateAllPlayersSleepingFlag();
    }
}

void World::detachEntityForWorldChange(Entity *entity)
{
    if (entity == nullptr)
        return;

    loadedEntityList.erase(std::remove(loadedEntityList.begin(), loadedEntityList.end(), entity), loadedEntityList.end());
    unloadedEntityList.erase(std::remove(unloadedEntityList.begin(), unloadedEntityList.end(), entity), unloadedEntityList.end());
    weatherEffects.erase(std::remove(weatherEffects.begin(), weatherEffects.end(), entity), weatherEffects.end());

    if (EntityPlayer *player = dynamic_cast<EntityPlayer *>(entity))
        playerEntities.erase(std::remove(playerEntities.begin(), playerEntities.end(), player), playerEntities.end());

    if (entity->addedToChunk && chunkExists(entity->chunkCoordX, entity->chunkCoordZ))
        getChunkFromChunkCoords(entity->chunkCoordX, entity->chunkCoordZ)->removeEntity(entity);
    entity->addedToChunk = false;
}

void World::addWorldAccess(IWorldAccess* worldAccess)
{
    worldAccesses.push_back(worldAccess);
}

void World::removeWorldAccess(IWorldAccess* worldAccess)
{
    auto it = std::find(worldAccesses.begin(), worldAccesses.end(), worldAccess);
    if (it != worldAccesses.end())
    {
        worldAccesses.erase(it);
    }
}

std::vector<AxisAlignedBB*> &World::getCollidingBoundingBoxes(Entity* entity, AxisAlignedBB* aabb)
{
    collidingBoundingBoxes.clear();
    
    int minX = MathHelper::floor_double(aabb->minX);
    int maxX = MathHelper::floor_double(aabb->maxX + 1.0);
    int minY = MathHelper::floor_double(aabb->minY);
    int maxY = MathHelper::floor_double(aabb->maxY + 1.0);
    int minZ = MathHelper::floor_double(aabb->minZ);
    int maxZ = MathHelper::floor_double(aabb->maxZ + 1.0);
    
    for (int x = minX; x < maxX; x++)
    {
        for (int z = minZ; z < maxZ; z++)
        {
            if (!blockExists(x, 64, z))
            {
                continue;
            }
            
            for (int y = minY - 1; y < maxY; y++)
            {
                Block* block = Block::blocksList[getBlockId(x, y, z)];
                if (block != nullptr)
                {
                    block->getCollidingBoundingBoxes(this, x, y, z, aabb, collidingBoundingBoxes);
                }
            }
        }
    }
    
    double d = 0.25;
    std::vector<Entity*> list = getEntitiesWithinAABBExcludingEntity(entity, aabb->expand(d, d, d));
    
    for (size_t j2 = 0; j2 < list.size(); j2++)
    {
        AxisAlignedBB* aabb1 = list[j2]->getBoundingBox();
        if (aabb1 != nullptr && aabb1->intersectsWith(aabb))
        {
            collidingBoundingBoxes.push_back(aabb1);
        }
        
        aabb1 = entity->getCollisionBox(list[j2]);
        if (aabb1 != nullptr && aabb1->intersectsWith(aabb))
        {
            collidingBoundingBoxes.push_back(aabb1);
        }
    }
    
    return collidingBoundingBoxes;
}

int World::calculateSkylightSubtracted(float partialTicks)
{
    float celestialAngle = getCelestialAngle(partialTicks);
    float f2 = 1.0f - (MathHelper::cos(celestialAngle * 3.141593f * 2.0f) * 2.0f + 0.5f);
    
    if (f2 < 0.0f)
    {
        f2 = 0.0f;
    }
    
    if (f2 > 1.0f)
    {
        f2 = 1.0f;
    }
    
    f2 = 1.0f - f2;
    f2 = (float)((double)f2 * (1.0 - (double)(func_27162_g(partialTicks) * 5.0f) / 16.0));
    f2 = (float)((double)f2 * (1.0 - (double)(func_27166_f(partialTicks) * 5.0f) / 16.0));
    f2 = 1.0f - f2;
    
    return (int)(f2 * 11.0f);
}

Vec3D* World::getSkyColor(Entity* entity, float partialTicks)
{
    float celestialAngle = getCelestialAngle(partialTicks);
    float f2 = MathHelper::cos(celestialAngle * 3.141593f * 2.0f) * 2.0f + 0.5f;
    
    if (f2 < 0.0f)
    {
        f2 = 0.0f;
    }
    
    if (f2 > 1.0f)
    {
        f2 = 1.0f;
    }
    
    int i = MathHelper::floor_double(entity->posX);
    int j = MathHelper::floor_double(entity->posZ);
    float f3 = (float)getWorldChunkManager()->getTemperature(i, j);
    int k = getWorldChunkManager()->getBiomeGenAt(i, j)->getSkyColorByTemp(f3);
    
    float f4 = (float)(k >> 16 & 0xff) / 255.0f;
    float f5 = (float)(k >> 8 & 0xff) / 255.0f;
    float f6 = (float)(k & 0xff) / 255.0f;
    
    f4 *= f2;
    f5 *= f2;
    f6 *= f2;
    
    float f7 = func_27162_g(partialTicks);
    if (f7 > 0.0f)
    {
        float f8 = (f4 * 0.3f + f5 * 0.59f + f6 * 0.11f) * 0.6f;
        float f10 = 1.0f - f7 * 0.75f;
        f4 = f4 * f10 + f8 * (1.0f - f10);
        f5 = f5 * f10 + f8 * (1.0f - f10);
        f6 = f6 * f10 + f8 * (1.0f - f10);
    }
    
    float f9 = func_27166_f(partialTicks);
    if (f9 > 0.0f)
    {
        float f11 = (f4 * 0.3f + f5 * 0.59f + f6 * 0.11f) * 0.2f;
        float f13 = 1.0f - f9 * 0.75f;
        f4 = f4 * f13 + f11 * (1.0f - f13);
        f5 = f5 * f13 + f11 * (1.0f - f13);
        f6 = f6 * f13 + f11 * (1.0f - f13);
    }
    
    if (field_27172_i > 0)
    {
        float f12 = (float)field_27172_i - partialTicks;
        if (f12 > 1.0f)
        {
            f12 = 1.0f;
        }
        f12 *= 0.45f;
        f4 = f4 * (1.0f - f12) + 0.8f * f12;
        f5 = f5 * (1.0f - f12) + 0.8f * f12;
        f6 = f6 * (1.0f - f12) + 1.0f * f12;
    }
    
    return Vec3D::createVector(f4, f5, f6);
}

float World::getCelestialAngle(float partialTicks)
{
    return worldProvider->calculateCelestialAngle(worldInfo->getWorldTime(), partialTicks);
}

Vec3D* World::getCloudFogColor(float partialTicks)
{
    float celestialAngle = getCelestialAngle(partialTicks);
    float f2 = MathHelper::cos(celestialAngle * 3.141593f * 2.0f) * 2.0f + 0.5f;
    
    if (f2 < 0.0f)
    {
        f2 = 0.0f;
    }
    
    if (f2 > 1.0f)
    {
        f2 = 1.0f;
    }
    
    float f3 = (float)(field_1019_F >> 16 & 255LL) / 255.0f;
    float f4 = (float)(field_1019_F >> 8 & 255LL) / 255.0f;
    float f5 = (float)(field_1019_F & 255LL) / 255.0f;
    
    float f6 = func_27162_g(partialTicks);
    if (f6 > 0.0f)
    {
        float f7 = (f3 * 0.3f + f4 * 0.59f + f5 * 0.11f) * 0.6f;
        float f9 = 1.0f - f6 * 0.95f;
        f3 = f3 * f9 + f7 * (1.0f - f9);
        f4 = f4 * f9 + f7 * (1.0f - f9);
        f5 = f5 * f9 + f7 * (1.0f - f9);
    }
    
    f3 *= f2 * 0.9f + 0.1f;
    f4 *= f2 * 0.9f + 0.1f;
    f5 *= f2 * 0.85f + 0.15f;
    
    float f8 = func_27166_f(partialTicks);
    if (f8 > 0.0f)
    {
        float f10 = (f3 * 0.3f + f4 * 0.59f + f5 * 0.11f) * 0.2f;
        float f11 = 1.0f - f8 * 0.95f;
        f3 = f3 * f11 + f10 * (1.0f - f11);
        f4 = f4 * f11 + f10 * (1.0f - f11);
        f5 = f5 * f11 + f10 * (1.0f - f11);
    }
    
    return Vec3D::createVector(f3, f4, f5);
}

Vec3D* World::getFogColor(float partialTicks)
{
    float celestialAngle = getCelestialAngle(partialTicks);
    return worldProvider->getFogColor(celestialAngle, partialTicks);
}

int World::findTopSolidBlock(int x, int z)
{
    Chunk* chunk = getChunkFromBlockCoords(x, z);
    int y = 127;
    x &= 0xf;
    z &= 0xf;
    
    while (y > 0)
    {
        int blockId = chunk->getBlockID(x, y, z);
        Material* material = (blockId != 0) ? Block::blocksList[blockId]->blockMaterial : Material::air;
        
        if (!material->getIsSolid() && !material->getIsLiquid())
        {
            y--;
        }
        else
        {
            return y + 1;
        }
    }
    
    return -1;
}

float World::getStarBrightness(float partialTicks)
{
    float celestialAngle = getCelestialAngle(partialTicks);
    float f2 = 1.0f - (MathHelper::cos(celestialAngle * 3.141593f * 2.0f) * 2.0f + 0.75f);
    
    if (f2 < 0.0f)
    {
        f2 = 0.0f;
    }
    
    if (f2 > 1.0f)
    {
        f2 = 1.0f;
    }
    
    return f2 * f2 * 0.5f;
}

void World::scheduleBlockUpdate(int x, int y, int z, int blockId, int delay)
{
    NextTickListEntry* entry = new NextTickListEntry(x, y, z, blockId);
    const int RANGE = 8;
    
    if (scheduledUpdatesAreImmediate)
    {
        if (checkChunksExist(entry->xCoord - RANGE, entry->yCoord - RANGE, entry->zCoord - RANGE,
                             entry->xCoord + RANGE, entry->yCoord + RANGE, entry->zCoord + RANGE))
        {
            int currentBlockId = getBlockId(entry->xCoord, entry->yCoord, entry->zCoord);
            if (currentBlockId == entry->blockID && currentBlockId > 0)
            {
                Block::blocksList[currentBlockId]->updateTick(this, entry->xCoord, entry->yCoord, entry->zCoord, rand);
            }
        }
        return;
    }
    
    if (checkChunksExist(x - RANGE, y - RANGE, z - RANGE, x + RANGE, y + RANGE, z + RANGE))
    {
        if (blockId > 0)
        {
            entry->setScheduledTime((long)delay + worldInfo->getWorldTime());
        }
        
        if (scheduledTickSet.find(entry) == scheduledTickSet.end())
        {
            scheduledTickSet.insert(entry);
            scheduledTickTreeSet.insert(entry);
        }
    }
}

void World::updateEntities()
{
    // Update weather effects
    for (size_t i = 0; i < weatherEffects.size(); i++)
    {
        Entity* entity = weatherEffects[i];
        entity->onUpdate();
        
        if (entity->isDead)
        {
            weatherEffects.erase(weatherEffects.begin() + i);
            onEntityRemoved(entity);
            deleteWorldOwnedEntity(entity);
            i--;
        }
    }
    
    // Java: loadedEntityList.removeAll(unloadedEntityList). Entities whose
    // chunks were unloaded must stop counting as active before the temporary
    // unload list is cleared, otherwise they accumulate as invisible ghosts
    // and permanently exhaust the creature spawn caps.
    if (!unloadedEntityList.empty())
    {
        const std::unordered_set<Entity*> unloadedEntities(
            unloadedEntityList.begin(), unloadedEntityList.end());
        loadedEntityList.erase(
            std::remove_if(loadedEntityList.begin(), loadedEntityList.end(),
                [&unloadedEntities](Entity* entity)
                {
                    return unloadedEntities.find(entity) != unloadedEntities.end();
                }),
            loadedEntityList.end());
    }

    // Remove unloaded entities from their chunks
    for (size_t j = 0; j < unloadedEntityList.size(); j++)
    {
        Entity* entity = unloadedEntityList[j];
        int chunkX = entity->chunkCoordX;
        int chunkZ = entity->chunkCoordZ;
        
        if (entity->addedToChunk && chunkExists(chunkX, chunkZ))
        {
            getChunkFromChunkCoords(chunkX, chunkZ)->removeEntity(entity);
        }
    }
    
    for (size_t j = 0; j < unloadedEntityList.size(); j++)
    {
        releaseEntitySkin(unloadedEntityList[j]);
        onEntityRemoved(unloadedEntityList[j]);
        deleteWorldOwnedEntity(unloadedEntityList[j]);
    }
    
    unloadedEntityList.clear();
    
    // Update loaded entities
    for (size_t l = 0; l < loadedEntityList.size(); l++)
    {
        Entity* entity = loadedEntityList[l];
#if defined(__ARM_ARCH)
        // Phone: far-mob physics dominated tick cost. Beyond 24 blocks from
        // every player, mobs tick at quarter rate.
        if (entity != nullptr && !entity->isDead &&
            dynamic_cast<EntityPlayer *>(entity) == nullptr)
        {
            if (getClosestPlayerToEntity(entity, 24.0) == nullptr &&
                ((entity->ticksExisted + entity->entityId) & 3) != 0)
                continue;
        }
#endif
        
        if (entity->ridingEntity != nullptr)
        {
            if (!entity->ridingEntity->isDead && entity->ridingEntity->riddenByEntity == entity)
            {
                continue;
            }
            
            entity->ridingEntity->riddenByEntity = nullptr;
            entity->ridingEntity = nullptr;
        }
        
        if (!entity->isDead)
        {
            updateEntity(entity);
        }
        
        if (entity->isDead)
        {
            int chunkX = entity->chunkCoordX;
            int chunkZ = entity->chunkCoordZ;
            
            if (entity->addedToChunk && chunkExists(chunkX, chunkZ))
            {
                getChunkFromChunkCoords(chunkX, chunkZ)->removeEntity(entity);
            }
            
            loadedEntityList.erase(loadedEntityList.begin() + l);
            l--;
            releaseEntitySkin(entity);
            onEntityRemoved(entity);
            deleteWorldOwnedEntity(entity);
        }
    }
    
    // Update tile entities
    updatingTileEntities = true;
    
    for (auto it = loadedTileEntityList.begin(); it != loadedTileEntityList.end();)
    {
        TileEntity* tileEntity = *it;
        
        if (!tileEntity->isInvalid())
        {
            tileEntity->updateEntity();
        }
        
        if (tileEntity->isInvalid())
        {
            it = loadedTileEntityList.erase(it);
            Chunk* chunk = getChunkFromChunkCoords(tileEntity->xCoord >> 4, tileEntity->zCoord >> 4);
            if (chunk != nullptr)
            {
                chunk->removeChunkBlockTileEntity(tileEntity->xCoord & 0xf, tileEntity->yCoord, tileEntity->zCoord & 0xf);
            }
            delete tileEntity;
        }
        else
        {
            ++it;
        }
    }
    
    updatingTileEntities = false;
    
    if (!tileEntitiesToAdd.empty())
    {
        for (auto it = tileEntitiesToAdd.begin(); it != tileEntitiesToAdd.end();)
        {
            TileEntity* tileEntity = *it;
            
            if (!tileEntity->isInvalid())
            {
                auto findIt = std::find(loadedTileEntityList.begin(), loadedTileEntityList.end(), tileEntity);
                if (findIt == loadedTileEntityList.end())
                {
                    loadedTileEntityList.push_back(tileEntity);
                }
                
                Chunk* chunk = getChunkFromChunkCoords(tileEntity->xCoord >> 4, tileEntity->zCoord >> 4);
                if (chunk != nullptr)
                {
                    chunk->setChunkBlockTileEntity(tileEntity->xCoord & 0xf, tileEntity->yCoord, tileEntity->zCoord & 0xf, tileEntity);
                }
                
                markBlockNeedsUpdate(tileEntity->xCoord, tileEntity->yCoord, tileEntity->zCoord);
            }
            
            it = tileEntitiesToAdd.erase(it);
        }
    }
}

void World::addLoadedTileEntities(const std::vector<TileEntity*>& collection)
{
    if (updatingTileEntities)
    {
        tileEntitiesToAdd.insert(tileEntitiesToAdd.end(), collection.begin(), collection.end());
    }
    else
    {
        loadedTileEntityList.insert(loadedTileEntityList.end(), collection.begin(), collection.end());
    }
}

void World::updateEntity(Entity* entity)
{
    updateEntityWithOptionalForce(entity, true);
}

void World::updateEntityWithOptionalForce(Entity* entity, bool flag)
{
    int i = MathHelper::floor_double(entity->posX);
    int j = MathHelper::floor_double(entity->posZ);
    const int RANGE = 32;
    
    if (flag && !checkChunksExist(i - RANGE, 0, j - RANGE, i + RANGE, 128, j + RANGE))
    {
        return;
    }
    
    entity->lastTickPosX = entity->posX;
    entity->lastTickPosY = entity->posY;
    entity->lastTickPosZ = entity->posZ;
    entity->prevRotationYaw = entity->rotationYaw;
    entity->prevRotationPitch = entity->rotationPitch;
    
    if (flag && entity->addedToChunk)
    {
        if (entity->ridingEntity != nullptr)
        {
            entity->updateRidden();
        }
        else
        {
            entity->onUpdate();
        }
    }
    
    if (std::isnan(entity->posX) || std::isinf(entity->posX))
    {
        entity->posX = entity->lastTickPosX;
    }
    
    if (std::isnan(entity->posY) || std::isinf(entity->posY))
    {
        entity->posY = entity->lastTickPosY;
    }
    
    if (std::isnan(entity->posZ) || std::isinf(entity->posZ))
    {
        entity->posZ = entity->lastTickPosZ;
    }
    
    if (std::isnan(entity->rotationPitch) || std::isinf(entity->rotationPitch))
    {
        entity->rotationPitch = entity->prevRotationPitch;
    }
    
    if (std::isnan(entity->rotationYaw) || std::isinf(entity->rotationYaw))
    {
        entity->rotationYaw = entity->prevRotationYaw;
    }
    
    int k = MathHelper::floor_double(entity->posX / 16.0);
    int l = MathHelper::floor_double(entity->posY / 16.0);
    int i1 = MathHelper::floor_double(entity->posZ / 16.0);
    
    if (!entity->addedToChunk || entity->chunkCoordX != k || entity->chunkCoordY != l || entity->chunkCoordZ != i1)
    {
        if (entity->addedToChunk && chunkExists(entity->chunkCoordX, entity->chunkCoordZ))
        {
            getChunkFromChunkCoords(entity->chunkCoordX, entity->chunkCoordZ)->removeEntityAtIndex(entity, entity->chunkCoordY);
        }
        
        if (chunkExists(k, i1))
        {
            entity->addedToChunk = true;
            getChunkFromChunkCoords(k, i1)->addEntity(entity);
        }
        else
        {
            entity->addedToChunk = false;
        }
    }
    
    if (flag && entity->addedToChunk && entity->riddenByEntity != nullptr)
    {
        if (entity->riddenByEntity->isDead || entity->riddenByEntity->ridingEntity != entity)
        {
            entity->riddenByEntity->ridingEntity = nullptr;
            entity->riddenByEntity = nullptr;
        }
        else
        {
            updateEntity(entity->riddenByEntity);
        }
    }
}

bool World::checkIfAABBIsClear(AxisAlignedBB* aabb)
{
    std::vector<Entity*> list = getEntitiesWithinAABBExcludingEntity(nullptr, aabb);
    
    for (size_t i = 0; i < list.size(); i++)
    {
        Entity* entity = list[i];
        if (!entity->isDead && entity->preventEntitySpawning)
        {
            return false;
        }
    }
    
    return true;
}

bool World::getIsAnyLiquid(AxisAlignedBB* aabb)
{
    int minX = MathHelper::floor_double(aabb->minX);
    int maxX = MathHelper::floor_double(aabb->maxX + 1.0);
    int minY = MathHelper::floor_double(aabb->minY);
    int maxY = MathHelper::floor_double(aabb->maxY + 1.0);
    int minZ = MathHelper::floor_double(aabb->minZ);
    int maxZ = MathHelper::floor_double(aabb->maxZ + 1.0);
    
    if (aabb->minX < 0.0)
    {
        minX--;
    }
    if (aabb->minY < 0.0)
    {
        minY--;
    }
    if (aabb->minZ < 0.0)
    {
        minZ--;
    }
    
    for (int x = minX; x < maxX; x++)
    {
        for (int y = minY; y < maxY; y++)
        {
            for (int z = minZ; z < maxZ; z++)
            {
                Block* block = Block::blocksList[getBlockId(x, y, z)];
                if (block != nullptr && block->blockMaterial->getIsLiquid())
                {
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool World::isBoundingBoxBurning(AxisAlignedBB* aabb)
{
    int minX = MathHelper::floor_double(aabb->minX);
    int maxX = MathHelper::floor_double(aabb->maxX + 1.0);
    int minY = MathHelper::floor_double(aabb->minY);
    int maxY = MathHelper::floor_double(aabb->maxY + 1.0);
    int minZ = MathHelper::floor_double(aabb->minZ);
    int maxZ = MathHelper::floor_double(aabb->maxZ + 1.0);
    
    if (checkChunksExist(minX, minY, minZ, maxX, maxY, maxZ))
    {
        for (int x = minX; x < maxX; x++)
        {
            for (int y = minY; y < maxY; y++)
            {
                for (int z = minZ; z < maxZ; z++)
                {
                    int blockId = getBlockId(x, y, z);
                    if (blockId == Block::fire->blockID || 
                        blockId == Block::lavaMoving->blockID || 
                        blockId == Block::lavaStill->blockID)
                    {
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}

bool World::handleMaterialAcceleration(AxisAlignedBB* aabb, Material* material, Entity* entity)
{
    int minX = MathHelper::floor_double(aabb->minX);
    int maxX = MathHelper::floor_double(aabb->maxX + 1.0);
    int minY = MathHelper::floor_double(aabb->minY);
    int maxY = MathHelper::floor_double(aabb->maxY + 1.0);
    int minZ = MathHelper::floor_double(aabb->minZ);
    int maxZ = MathHelper::floor_double(aabb->maxZ + 1.0);
    
    if (!checkChunksExist(minX, minY, minZ, maxX, maxY, maxZ))
    {
        return false;
    }
    
    bool flag = false;
    Vec3D* vec3d = Vec3D::createVector(0.0, 0.0, 0.0);
    
    for (int x = minX; x < maxX; x++)
    {
        for (int y = minY; y < maxY; y++)
        {
            for (int z = minZ; z < maxZ; z++)
            {
                Block* block = Block::blocksList[getBlockId(x, y, z)];
                if (block == nullptr || block->blockMaterial != material)
                {
                    continue;
                }
                
                double d1 = (float)(y + 1) - BlockFluid::getPercentAir(getBlockMetadata(x, y, z));
                if ((double)maxY >= d1)
                {
                    flag = true;
                    block->velocityToAddToEntity(this, x, y, z, entity, vec3d);
                }
            }
        }
    }
    
    if (vec3d->lengthVector() > 0.0)
    {
        vec3d = vec3d->normalize();
        double d = 0.014;
        entity->motionX += vec3d->xCoord * d;
        entity->motionY += vec3d->yCoord * d;
        entity->motionZ += vec3d->zCoord * d;
    }
    
    return flag;
}

bool World::isMaterialInBB(AxisAlignedBB* aabb, Material* material)
{
    int minX = MathHelper::floor_double(aabb->minX);
    int maxX = MathHelper::floor_double(aabb->maxX + 1.0);
    int minY = MathHelper::floor_double(aabb->minY);
    int maxY = MathHelper::floor_double(aabb->maxY + 1.0);
    int minZ = MathHelper::floor_double(aabb->minZ);
    int maxZ = MathHelper::floor_double(aabb->maxZ + 1.0);
    
    for (int x = minX; x < maxX; x++)
    {
        for (int y = minY; y < maxY; y++)
        {
            for (int z = minZ; z < maxZ; z++)
            {
                Block* block = Block::blocksList[getBlockId(x, y, z)];
                if (block != nullptr && block->blockMaterial == material)
                {
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool World::isAABBInMaterial(AxisAlignedBB* aabb, Material* material)
{
    int minX = MathHelper::floor_double(aabb->minX);
    int maxX = MathHelper::floor_double(aabb->maxX + 1.0);
    int minY = MathHelper::floor_double(aabb->minY);
    int maxY = MathHelper::floor_double(aabb->maxY + 1.0);
    int minZ = MathHelper::floor_double(aabb->minZ);
    int maxZ = MathHelper::floor_double(aabb->maxZ + 1.0);
    
    for (int x = minX; x < maxX; x++)
    {
        for (int y = minY; y < maxY; y++)
        {
            for (int z = minZ; z < maxZ; z++)
            {
                Block* block = Block::blocksList[getBlockId(x, y, z)];
                if (block == nullptr || block->blockMaterial != material)
                {
                    continue;
                }
                
                int metadata = getBlockMetadata(x, y, z);
                double d = y + 1;
                if (metadata < 8)
                {
                    d = (double)(y + 1) - (double)metadata / 8.0;
                }
                
                if (d >= aabb->minY)
                {
                    return true;
                }
            }
        }
    }
    
    return false;
}

Explosion* World::createExplosion(Entity* entity, double x, double y, double z, float strength)
{
    return newExplosion(entity, x, y, z, strength, false);
}

Explosion* World::newExplosion(Entity* entity, double x, double y, double z, float strength, bool isFlaming)
{
    Explosion* explosion = new Explosion(this, entity, x, y, z, strength);
    explosion->isFlaming = isFlaming;
    explosion->doExplosionA();
    explosion->doExplosionB(true);
    return explosion;
}

float World::getBlockDensity(Vec3D* vec, AxisAlignedBB* aabb) // func_675_a
{
    double d = 1.0 / ((aabb->maxX - aabb->minX) * 2.0 + 1.0);
    double d1 = 1.0 / ((aabb->maxY - aabb->minY) * 2.0 + 1.0);
    double d2 = 1.0 / ((aabb->maxZ - aabb->minZ) * 2.0 + 1.0);
    
    int visible = 0;
    int total = 0;
    
    for (float f = 0.0f; f <= 1.0f; f = (float)((double)f + d))
    {
        for (float f1 = 0.0f; f1 <= 1.0f; f1 = (float)((double)f1 + d1))
        {
            for (float f2 = 0.0f; f2 <= 1.0f; f2 = (float)((double)f2 + d2))
            {
                double d3 = aabb->minX + (aabb->maxX - aabb->minX) * (double)f;
                double d4 = aabb->minY + (aabb->maxY - aabb->minY) * (double)f1;
                double d5 = aabb->minZ + (aabb->maxZ - aabb->minZ) * (double)f2;
                
                if (rayTraceBlocks(Vec3D::createVector(d3, d4, d5), vec) == nullptr)
                {
                    visible++;
                }
                total++;
            }
        }
    }
    
    return (float)visible / (float)total;
}

void World::onBlockHit(EntityPlayer* player, int x, int y, int z, int side)
{
    if (side == 0) y--;
    if (side == 1) y++;
    if (side == 2) z--;
    if (side == 3) z++;
    if (side == 4) x--;
    if (side == 5) x++;
    
    if (getBlockId(x, y, z) == Block::fire->blockID)
    {
        func_28107_a(player, 1004, x, y, z, 0);
        setBlockWithNotify(x, y, z, 0);
    }
}

Entity* World::func_4085_a(const std::type_info& classType)
{
    return nullptr;
}

std::string World::func_687_d()
{
    return "All: " + std::to_string(loadedEntityList.size());
}

std::string World::func_21119_g()
{
    return chunkProvider->makeString();
}

TileEntity* World::getBlockTileEntity(int x, int y, int z)
{
    Chunk* chunk = getChunkFromChunkCoords(x >> 4, z >> 4);
    if (chunk != nullptr)
    {
        return chunk->getChunkBlockTileEntity(x & 0xf, y, z & 0xf);
    }
    
    return nullptr;
}

void World::setBlockTileEntity(int x, int y, int z, TileEntity* tileEntity)
{
    if (!tileEntity->isInvalid())
    {
        if (updatingTileEntities)
        {
            tileEntity->xCoord = x;
            tileEntity->yCoord = y;
            tileEntity->zCoord = z;
            tileEntitiesToAdd.push_back(tileEntity);
        }
        else
        {
            loadedTileEntityList.push_back(tileEntity);
            Chunk* chunk = getChunkFromChunkCoords(x >> 4, z >> 4);
            if (chunk != nullptr)
            {
                chunk->setChunkBlockTileEntity(x & 0xf, y, z & 0xf, tileEntity);
            }
        }
    }
}

void World::markTileEntityChunkModified(int_t x, int_t y, int_t z, TileEntity *tileentity)
{
    if (blockExists(x, y, z))
        getChunkFromBlockCoords(x, z)->setChunkModified();
    for (IWorldAccess *access : worldAccesses)
        access->doNothingWithTileEntity(x, y, z, tileentity);
}

void World::removeBlockTileEntity(int x, int y, int z)
{
    TileEntity* tileEntity = getBlockTileEntity(x, y, z);
    
    if (tileEntity != nullptr && updatingTileEntities)
    {
        tileEntity->invalidate();
    }
    else
    {
        if (tileEntity != nullptr)
        {
            auto it = std::find(loadedTileEntityList.begin(), loadedTileEntityList.end(), tileEntity);
            if (it != loadedTileEntityList.end())
            {
                loadedTileEntityList.erase(it);
            }
        }
        
        Chunk* chunk = getChunkFromChunkCoords(x >> 4, z >> 4);
        if (chunk != nullptr)
        {
            chunk->removeChunkBlockTileEntity(x & 0xf, y, z & 0xf);
        }
    }
}

bool World::isBlockOpaqueCube(int x, int y, int z)
{
    Block* block = Block::blocksList[getBlockId(x, y, z)];
    if (block == nullptr)
    {
        return false;
    }
    
    return block->isOpaqueCube();
}

bool World::isBlockNormalCube(int x, int y, int z)
{
    Block* block = Block::blocksList[getBlockId(x, y, z)];
    if (block == nullptr)
    {
        return false;
    }
    
    return block->blockMaterial->getIsTranslucent() && block->renderAsNormalBlock();
}

void World::saveWorldIndirectly(IProgressUpdate* progressUpdate)
{
    saveWorld(true, progressUpdate);
}

bool World::updatingLighting()
{
    if (lightingUpdatesCounter >= 50)
        return false;

    lightingUpdatesCounter++;

    try
    {
        // NOTE: the lighting drain is deliberately left vanilla (500 jobs, no
        // wall-clock ceiling, merge scan of 5, 1M cap). Bounding it was tried and
        // REGRESSED average FPS: a wider merge scan unions light boxes into
        // larger render-dirty regions, and drip-feeding the queue lets sections
        // get meshed before their light settles (then re-meshed), so the
        // renderer queue went from ~2-76 to ~500-966 and the frame moved from
        // tick-bound to mesh-bound.
        int count = 500;

        while (lightingToUpdate.size() > 0)
        {
            if (--count <= 0)
            {
                lightingUpdatesCounter--;
                return true;
            }

            MetadataChunkBlock* metadataChunkBlock = lightingToUpdate.back();
            lightingToUpdate.pop_back();
            metadataChunkBlock->updateLight(this);
            delete metadataChunkBlock;
        }

        lightingUpdatesCounter--;
        return false;
    }
    catch (...)
    {
        lightingUpdatesCounter--;
        throw;
    }
}

void World::scheduleLightingUpdate(EnumSkyBlock* enumSkyBlock, int minX, int minY, int minZ, int maxX, int maxY, int maxZ)
{
    scheduleLightingUpdate_do(enumSkyBlock, minX, minY, minZ, maxX, maxY, maxZ, true);
}

void World::scheduleLightingUpdate_do(EnumSkyBlock* enumSkyBlock, int minX, int minY, int minZ, int maxX, int maxY, int maxZ, bool flag)
{
    if (worldProvider->hasNoSky && enumSkyBlock == EnumSkyBlock::Sky)
    {
        return;
    }
    
    lightingUpdatesScheduled++;
    
    struct CounterGuard
    {
        int_t &counter;
        ~CounterGuard() { --counter; }
    } counterGuard{lightingUpdatesScheduled};

        if (lightingUpdatesScheduled == 50)
        {
            return;
        }
        
        int midX = (maxX + minX) / 2;
        int midZ = (maxZ + minZ) / 2;
        
        if (!blockExists(midX, 64, midZ))
        {
            return;
        }
        
        if (getChunkFromBlockCoords(midX, midZ)->isEmptyChunk())
        {
            return;
        }
        
        int size = lightingToUpdate.size();
        
        if (flag)
        {
            int checkCount = 5;
            if (checkCount > size)
            {
                checkCount = size;
            }
            
            for (int i = 0; i < checkCount; i++)
            {
                MetadataChunkBlock* metadataChunkBlock = lightingToUpdate[lightingToUpdate.size() - i - 1];
                if (metadataChunkBlock->skyBlock == enumSkyBlock && 
                    metadataChunkBlock->tryMerge(minX, minY, minZ, maxX, maxY, maxZ))
                {
                    return;
                }
            }
        }
        
        lightingToUpdate.push_back(new MetadataChunkBlock(enumSkyBlock, minX, minY, minZ, maxX, maxY, maxZ));
        
        const int MAX_UPDATES = 1000000;
        if (lightingToUpdate.size() > MAX_UPDATES)
        {
            // std::cout << "More than " << MAX_UPDATES << " updates, aborting lighting updates" << std::endl;
            for (MetadataChunkBlock *update : lightingToUpdate)
                delete update;
            lightingToUpdate.clear();
        }
}

void World::calculateInitialSkylight()
{
    int i = calculateSkylightSubtracted(1.0f);
    if (i != skylightSubtracted)
    {
        skylightSubtracted = i;
    }
}

void World::setAllowedMobSpawns(bool hostile, bool peaceful)
{
    spawnHostileMobs = hostile;
    spawnPeacefulMobs = peaceful;
}

void World::tick()
{
    updateWeather();
    
    if (isAllPlayersFullyAsleep())
    {
        bool flag = false;
        if (spawnHostileMobs && difficultySetting >= 1)
        {
            flag = SpawnerAnimals::performSleepSpawning(this, playerEntities);
        }
        
        if (!flag)
        {
            long time = worldInfo->getWorldTime() + 24000LL;
            worldInfo->setWorldTime(time - time % 24000LL);
            wakeUpAllPlayers();
        }
    }
    
    if (mobSpawningEnabled())
        SpawnerAnimals::performSpawning(this, spawnHostileMobs, spawnPeacefulMobs);
    chunkProvider->unload100OldestChunks();    
    int i = calculateSkylightSubtracted(1.0f);
    if (i != skylightSubtracted)
    {
        skylightSubtracted = i;
        for (size_t j = 0; j < worldAccesses.size(); j++)
        {
            worldAccesses[j]->updateAllRenderers();
        }
    }
    
    long time = worldInfo->getWorldTime() + 1LL;
    if (time % (long)autosavePeriod == 0LL)
    {
        saveWorld(false, nullptr);
    }
    
    worldInfo->setWorldTime(time);
    TickUpdates(false);
    updateBlocksAndPlayCaveSounds();
}

void World::func_27163_E()
{
    if (worldInfo->getRaining())
    {
        rainingStrength = 1.0f;
        if (worldInfo->getThundering())
        {
            thunderingStrength = 1.0f;
        }
    }
}

void World::updateWeather()
{
    if (worldProvider->hasNoSky)
    {
        return;
    }
    
    if (thunderTime > 0)
    {
        thunderTime--;
    }
    
    int thunderTimeValue = worldInfo->getThunderTime();
    if (thunderTimeValue <= 0)
    {
        if (worldInfo->getThundering())
        {
            worldInfo->setThunderTime(rand.nextInt(12000) + 3600);
        }
        else
        {
            worldInfo->setThunderTime(rand.nextInt(0x29040) + 12000);
        }
    }
    else
    {
        thunderTimeValue--;
        worldInfo->setThunderTime(thunderTimeValue);
        if (thunderTimeValue <= 0)
        {
            worldInfo->setThundering(!worldInfo->getThundering());
        }
    }
    
    int rainTime = worldInfo->getRainTime();
    if (rainTime <= 0)
    {
        if (worldInfo->getRaining())
        {
            worldInfo->setRainTime(rand.nextInt(12000) + 12000);
        }
        else
        {
            worldInfo->setRainTime(rand.nextInt(0x29040) + 12000);
        }
    }
    else
    {
        rainTime--;
        worldInfo->setRainTime(rainTime);
        if (rainTime <= 0)
        {
            worldInfo->setRaining(!worldInfo->getRaining());
        }
    }
    
    prevRainingStrength = rainingStrength;
    if (worldInfo->getRaining())
    {
        rainingStrength += 0.01f;
    }
    else
    {
        rainingStrength -= 0.01f;
    }
    
    if (rainingStrength < 0.0f)
    {
        rainingStrength = 0.0f;
    }
    if (rainingStrength > 1.0f)
    {
        rainingStrength = 1.0f;
    }
    
    prevThunderingStrength = thunderingStrength;
    if (worldInfo->getThundering())
    {
        thunderingStrength += 0.01f;
    }
    else
    {
        thunderingStrength -= 0.01f;
    }
    
    if (thunderingStrength < 0.0f)
    {
        thunderingStrength = 0.0f;
    }
    if (thunderingStrength > 1.0f)
    {
        thunderingStrength = 1.0f;
    }
}

void World::stopPrecipitation()
{
    worldInfo->setRainTime(0);
    worldInfo->setRaining(false);
    worldInfo->setThunderTime(0);
    worldInfo->setThundering(false);
}

void World::updateBlocksAndPlayCaveSounds()
{
    for (ChunkCoordIntPair *position : positionsToUpdate)
        delete position;
    positionsToUpdate.clear();
    
    for (size_t i = 0; i < playerEntities.size(); i++)
    {
        EntityPlayer* entityPlayer = playerEntities[i];
        int chunkX = MathHelper::floor_double(entityPlayer->posX / 16.0);
        int chunkZ = MathHelper::floor_double(entityPlayer->posZ / 16.0);
        const int RANGE = 9;
        
        for (int dx = -RANGE; dx <= RANGE; dx++)
        {
            for (int dz = -RANGE; dz <= RANGE; dz++)
            {
                positionsToUpdate.insert(new ChunkCoordIntPair(dx + chunkX, dz + chunkZ));
            }
        }
    }
    
    if (soundCounter > 0)
    {
        soundCounter--;
    }
    
    // Round-robin the resident-chunk scan. Visiting all 361 chunks around the
    // player every tick paid the per-chunk fixed cost (chunk lookup, ambient
    // probe, snow branch, lightning roll) 361 times a tick -- OptiCraft's frame
    // log recorded this phase at 162-183 ms against a 4-10 ms tick average.
    // Visiting a rotating slice spreads that fixed cost; the random block ticks
    // are scaled by the same factor so world speed is unchanged
    // (361 chunks x 80 == 5 chunks x ~5776 per tick).
    static std::vector<ChunkCoordIntPair *> s_tickOrder;
    static int_t s_tickCursor = 0;
    s_tickOrder.clear();
    s_tickOrder.reserve(positionsToUpdate.size());
    for (ChunkCoordIntPair *position : positionsToUpdate)
        s_tickOrder.push_back(position);

    const int_t totalChunks = (int_t)s_tickOrder.size();
    if (totalChunks == 0)
        return;

    int_t visitCount = totalChunks;
    if (kRandomTickChunksPerTick > 0 && totalChunks > kRandomTickChunksPerTick)
        visitCount = kRandomTickChunksPerTick;

    if (s_tickCursor >= totalChunks)
        s_tickCursor = 0;

    const int_t tickScale = (totalChunks + visitCount - 1) / visitCount;
    const int_t tickAttempts = kRandomTicksPerChunk * tickScale;

    for (int_t visited = 0; visited < visitCount; visited++)
    {
        ChunkCoordIntPair* chunkCoordIntPair = s_tickOrder[(s_tickCursor + visited) % totalChunks];
        int chunkX = chunkCoordIntPair->chunkXPos * 16;
        int chunkZ = chunkCoordIntPair->chunkZPos * 16;
        Chunk* chunk = getChunkFromChunkCoords(chunkCoordIntPair->chunkXPos, chunkCoordIntPair->chunkZPos);
        
        if (soundCounter == 0 && caveSoundsEnabled())
        {
            updateLCG = updateLCG * 3 + UPDATE_LCG_INCREMENT;
            int randValue = updateLCG >> 2;
            int localX = randValue & 0xf;
            int localZ = randValue >> 8 & 0xf;
            int localY = randValue >> 16 & 0x7f;
            int blockId = chunk->getBlockID(localX, localY, localZ);
            
            localX += chunkX;
            localZ += chunkZ;
            
            if (blockId == 0 && 
                getFullBlockLightValue(localX, localY, localZ) <= rand.nextInt(8) && 
                getSavedLightValue(EnumSkyBlock::Sky, localX, localY, localZ) <= 0)
            {
                EntityPlayer* closestPlayer = getClosestPlayer((double)localX + 0.5, (double)localY + 0.5, (double)localZ + 0.5, 8.0);
                if (closestPlayer != nullptr && 
                    closestPlayer->getDistanceSq((double)localX + 0.5, (double)localY + 0.5, (double)localZ + 0.5) > 4.0)
                {
                    playSoundEffect((double)localX + 0.5, (double)localY + 0.5, (double)localZ + 0.5, 
                                   "ambient.cave.cave", 0.7f, 0.8f + (float)rand.nextInt(100) / 100.0f * 0.2f);
                    soundCounter = rand.nextInt(12000) + 6000;
                }
            }
        }
        
        if (rand.nextInt(0x186a0) == 0 && func_27161_C() && func_27160_B())
        {
            updateLCG = updateLCG * 3 + UPDATE_LCG_INCREMENT;
            int randValue = updateLCG >> 2;
            int lightningX = chunkX + (randValue & 0xf);
            int lightningZ = chunkZ + (randValue >> 8 & 0xf);
            int lightningY = findTopSolidBlock(lightningX, lightningZ);
            
            if (canBlockBeRainedOn(lightningX, lightningY, lightningZ))
            {
                addWeatherEffect(new EntityLightningBolt(this, lightningX, lightningY, lightningZ));
                thunderTime = 2;
            }
        }
        
        if (rand.nextInt(16) == 0)
        {
            updateLCG = updateLCG * 3 + UPDATE_LCG_INCREMENT;
            int randValue = updateLCG >> 2;
            int snowX = randValue & 0xf;
            int snowZ = randValue >> 8 & 0xf;
            int snowY = findTopSolidBlock(snowX + chunkX, snowZ + chunkZ);
            
            if (getWorldChunkManager()->getBiomeGenAt(snowX + chunkX, snowZ + chunkZ)->getEnableSnow() && 
                snowY >= 0 && snowY < 128 && 
                chunk->getSavedLightValue(EnumSkyBlock::Block, snowX, snowY, snowZ) < 10)
            {
                int blockBelow = chunk->getBlockID(snowX, snowY - 1, snowZ);
                int blockAt = chunk->getBlockID(snowX, snowY, snowZ);
                
                if (func_27161_C() && blockAt == 0 && 
                    Block::snow->canPlaceBlockAt(this, snowX + chunkX, snowY, snowZ + chunkZ) && 
                    blockBelow != 0 && blockBelow != Block::ice->blockID && 
                    Block::blocksList[blockBelow]->blockMaterial->getIsSolid())
                {
                    setBlockWithNotify(snowX + chunkX, snowY, snowZ + chunkZ, Block::snow->blockID);
                }
                
                if (blockBelow == Block::waterStill->blockID && 
                    chunk->getBlockMetadata(snowX, snowY - 1, snowZ) == 0)
                {
                    setBlockWithNotify(snowX + chunkX, snowY - 1, snowZ + chunkZ, Block::ice->blockID);
                }
            }
        }
        
        int tickCount = 0;
        while (tickCount < tickAttempts)
        {
            updateLCG = updateLCG * 3 + UPDATE_LCG_INCREMENT;
            int randValue = updateLCG >> 2;
            int tickX = randValue & 0xf;
            int tickZ = randValue >> 8 & 0xf;
            int tickY = randValue >> 16 & 0x7f;
            int tickBlockId = chunk->blocks[tickX << 11 | tickZ << 7 | tickY] & 0xff;
            
            if (Block::tickOnLoad[tickBlockId])
            {
                Block::blocksList[tickBlockId]->updateTick(this, tickX + chunkX, tickY, tickZ + chunkZ, rand);
            }
            
            tickCount++;
        }
    }

    s_tickCursor = (s_tickCursor + visitCount) % totalChunks;
}

bool World::TickUpdates(bool flag)
{
    int size = scheduledTickTreeSet.size();
    if (size != scheduledTickSet.size())
    {
        throw std::runtime_error("TickNextTick list out of synch");
    }
    
    if (size > 1000)
    {
        size = 1000;
    }
    
    for (int j = 0; j < size; j++)
    {
        auto it = scheduledTickTreeSet.begin();
        NextTickListEntry* entry = *it;
        
        if (!flag && entry->scheduledTime > worldInfo->getWorldTime())
        {
            break;
        }
        
        scheduledTickTreeSet.erase(it);
        scheduledTickSet.erase(entry);
        
        const int RANGE = 8;
        if (!checkChunksExist(entry->xCoord - RANGE, entry->yCoord - RANGE, entry->zCoord - RANGE,
                              entry->xCoord + RANGE, entry->yCoord + RANGE, entry->zCoord + RANGE))
        {
            delete entry;
            continue;
        }
        
        int blockId = getBlockId(entry->xCoord, entry->yCoord, entry->zCoord);
        if (blockId == entry->blockID && blockId > 0)
        {
            Block::blocksList[blockId]->updateTick(this, entry->xCoord, entry->yCoord, entry->zCoord, rand);
        }
        delete entry;
    }
    
    return scheduledTickTreeSet.size() != 0;
}

void World::randomDisplayUpdates(int x, int y, int z)
{
    if (!particlesEnabled())
        return;

    const int RANGE = 16;
    
    // Vanilla probes 1000 times per tick (6 RNG draws plus a block lookup each);
    // on constrained hardware that was a 347 ms slowTick spike. A quarter keeps
    // ambient torches/lava/portals emitting at a quarter of the density.
    for (int l = 0; l < kRandomDisplayProbes; l++)
    {
        int randX = (x + rand.nextInt(RANGE)) - rand.nextInt(RANGE);
        int randY = (y + rand.nextInt(RANGE)) - rand.nextInt(RANGE);
        int randZ = (z + rand.nextInt(RANGE)) - rand.nextInt(RANGE);
        int blockId = getBlockId(randX, randY, randZ);
        
        if (blockId > 0)
        {
            Block::blocksList[blockId]->randomDisplayTick(this, randX, randY, randZ, rand);
        }
    }
}

std::vector<Entity*> &World::getEntitiesWithinAABBExcludingEntity(Entity* entity, AxisAlignedBB* aabb)
{
    entitiesWithinAABBExcludingEntity.clear();
    
    int minChunkX = MathHelper::floor_double((aabb->minX - 2.0) / 16.0);
    int maxChunkX = MathHelper::floor_double((aabb->maxX + 2.0) / 16.0);
    int minChunkZ = MathHelper::floor_double((aabb->minZ - 2.0) / 16.0);
    int maxChunkZ = MathHelper::floor_double((aabb->maxZ + 2.0) / 16.0);
    
    for (int cx = minChunkX; cx <= maxChunkX; cx++)
    {
        for (int cz = minChunkZ; cz <= maxChunkZ; cz++)
        {
            if (chunkExists(cx, cz))
            {
                getChunkFromChunkCoords(cx, cz)->getEntitiesWithinAABBForEntity(entity, aabb, entitiesWithinAABBExcludingEntity);
            }
        }
    }
    
    return entitiesWithinAABBExcludingEntity;
}

std::vector<Entity*> World::getEntitiesWithinAABB(const std::type_info& classType, AxisAlignedBB* aabb)
{
    int minChunkX = MathHelper::floor_double((aabb->minX - 2.0) / 16.0);
    int maxChunkX = MathHelper::floor_double((aabb->maxX + 2.0) / 16.0);
    int minChunkZ = MathHelper::floor_double((aabb->minZ - 2.0) / 16.0);
    int maxChunkZ = MathHelper::floor_double((aabb->maxZ + 2.0) / 16.0);
    
    std::vector<Entity*> result;
    
    for (int cx = minChunkX; cx <= maxChunkX; cx++)
    {
        for (int cz = minChunkZ; cz <= maxChunkZ; cz++)
        {
            if (chunkExists(cx, cz))
            {
                getChunkFromChunkCoords(cx, cz)->getEntitiesOfTypeWithinAAAB(classType, aabb, result);
            }
        }
    }
    
    return result;
}

std::vector<Entity*> &World::getLoadedEntityList()
{
    return loadedEntityList;
}

int_t World::countEntities(EnumCreatureTypeTag tag)
{
    int_t count = 0;
    for (Entity *entity : loadedEntityList)
    {
        switch (tag)
        {
        case EnumCreatureTypeTag::monster_tag:
            if (dynamic_cast<EntityMob *>(entity)) count++;
            break;
        case EnumCreatureTypeTag::creature_tag:
            if (dynamic_cast<EntityAnimal *>(entity)) count++;
            break;
        case EnumCreatureTypeTag::waterCreature_tag:
            if (dynamic_cast<EntityWaterMob *>(entity)) count++;
            break;
        }
    }
    return count;
}

void World::func_636_a(const std::vector<Entity*>& list)
{
    // Dedup (Java relied on GC; here a duplicate entry becomes a dangling pointer after
    // the entity is deleted+removed once by updateEntities -> crash in the render loop's
    // typeid(*entity)). Matches the existing guard in onEntityAdded around line 2985.
    for (size_t i = 0; i < list.size(); i++)
    {
        Entity* entity = list[i];
        if (std::find(loadedEntityList.begin(), loadedEntityList.end(), entity) == loadedEntityList.end())
            loadedEntityList.push_back(entity);
        obtainEntitySkin(entity);
    }
}

void World::func_632_b(const std::vector<Entity*>& list)
{
    for (Entity *entity : list)
    {
        // A server chunk-unload only removes the local player from that chunk's
        // spatial index.  Minecraft still owns and uses the player as its camera;
        // queueing it with ordinary mobs would make updateEntities() destroy it.
        if (isActiveClientEntity(entity))
        {
            entity->addedToChunk = false;
            continue;
        }

        if (std::find(unloadedEntityList.begin(), unloadedEntityList.end(), entity) == unloadedEntityList.end())
            unloadedEntityList.push_back(entity);
    }
}

void World::func_656_j()
{
    while (chunkProvider->unload100OldestChunks());
}

bool World::canBlockBePlacedAt(int blockId, int x, int y, int z, bool flag, int side)
{
    int existingBlockId = getBlockId(x, y, z);
    Block* existingBlock = Block::blocksList[existingBlockId];
    Block* newBlock = Block::blocksList[blockId];
    AxisAlignedBB* aabb = newBlock->getCollisionBoundingBoxFromPool(this, x, y, z);
    
    if (flag)
    {
        aabb = nullptr;
    }
    
    if (aabb != nullptr && !checkIfAABBIsClear(aabb))
    {
        return false;
    }
    
    if (existingBlock == Block::waterMoving || existingBlock == Block::waterStill || 
        existingBlock == Block::lavaMoving || existingBlock == Block::lavaStill || 
        existingBlock == Block::fire || existingBlock == Block::snow)
    {
        existingBlock = nullptr;
    }
    
    return blockId > 0 && existingBlock == nullptr && newBlock->canPlaceBlockOnSide(this, x, y, z, side);
}

PathEntity* World::getPathToEntity(Entity* entity, Entity* target, float maxRange)
{
    int startX = MathHelper::floor_double(entity->posX);
    int startY = MathHelper::floor_double(entity->posY);
    int startZ = MathHelper::floor_double(entity->posZ);
    int range = (int)(maxRange + 16.0f);
    int minX = startX - range;
    int minY = startY - range;
    int minZ = startZ - range;
    int maxX = startX + range;
    int maxY = startY + range;
    int maxZ = startZ + range;
    
    ChunkCache chunkCache(this, minX, minY, minZ, maxX, maxY, maxZ);
    Pathfinder pathfinder(&chunkCache);
    return pathfinder.createEntityPathTo(entity, target, maxRange);
}

PathEntity* World::getEntityPathToXYZ(Entity* entity, int x, int y, int z, float maxRange)
{
    int startX = MathHelper::floor_double(entity->posX);
    int startY = MathHelper::floor_double(entity->posY);
    int startZ = MathHelper::floor_double(entity->posZ);
    int range = (int)(maxRange + 8.0f);
    int minX = startX - range;
    int minY = startY - range;
    int minZ = startZ - range;
    int maxX = startX + range;
    int maxY = startY + range;
    int maxZ = startZ + range;
    
    ChunkCache chunkCache(this, minX, minY, minZ, maxX, maxY, maxZ);
    Pathfinder pathfinder(&chunkCache);
    return pathfinder.createEntityPathTo(entity, x, y, z, maxRange);
}

bool World::isBlockProvidingPowerTo(int x, int y, int z, int side)
{
    int blockId = getBlockId(x, y, z);
    if (blockId == 0)
    {
        return false;
    }
    
    return Block::blocksList[blockId]->isIndirectlyPoweringTo(this, x, y, z, side);
}

bool World::isBlockGettingPowered(int x, int y, int z)
{
    if (isBlockProvidingPowerTo(x, y - 1, z, 0)) return true;
    if (isBlockProvidingPowerTo(x, y + 1, z, 1)) return true;
    if (isBlockProvidingPowerTo(x, y, z - 1, 2)) return true;
    if (isBlockProvidingPowerTo(x, y, z + 1, 3)) return true;
    if (isBlockProvidingPowerTo(x - 1, y, z, 4)) return true;
    return isBlockProvidingPowerTo(x + 1, y, z, 5);
}

bool World::isBlockIndirectlyProvidingPowerTo(int x, int y, int z, int side)
{
    if (isBlockNormalCube(x, y, z))
    {
        return isBlockGettingPowered(x, y, z);
    }
    
    int blockId = getBlockId(x, y, z);
    if (blockId == 0)
    {
        return false;
    }
    
    return Block::blocksList[blockId]->isPoweringTo(this, x, y, z, side);
}

bool World::isBlockIndirectlyGettingPowered(int x, int y, int z)
{
    if (isBlockIndirectlyProvidingPowerTo(x, y - 1, z, 0)) return true;
    if (isBlockIndirectlyProvidingPowerTo(x, y + 1, z, 1)) return true;
    if (isBlockIndirectlyProvidingPowerTo(x, y, z - 1, 2)) return true;
    if (isBlockIndirectlyProvidingPowerTo(x, y, z + 1, 3)) return true;
    if (isBlockIndirectlyProvidingPowerTo(x - 1, y, z, 4)) return true;
    return isBlockIndirectlyProvidingPowerTo(x + 1, y, z, 5);
}

EntityPlayer* World::getClosestPlayerToEntity(Entity* entity, double maxDistance)
{
    return getClosestPlayer(entity->posX, entity->posY, entity->posZ, maxDistance);
}

EntityPlayer* World::getClosestPlayer(double x, double y, double z, double maxDistance)
{
    double closestDist = -1.0;
    EntityPlayer* closestPlayer = nullptr;
    
    for (size_t i = 0; i < playerEntities.size(); i++)
    {
        EntityPlayer* player = playerEntities[i];
        double distSq = player->getDistanceSq(x, y, z);
        
        if ((maxDistance < 0.0 || distSq < maxDistance * maxDistance) && 
            (closestDist == -1.0 || distSq < closestDist))
        {
            closestDist = distSq;
            closestPlayer = player;
        }
    }
    
    return closestPlayer;
}

EntityPlayer* World::getPlayerEntityByName(const jstring& name)
{
    for (size_t i = 0; i < playerEntities.size(); i++)
    {
        if (name == playerEntities[i]->username)
        {
            return playerEntities[i];
        }
    }
    
    return nullptr;
}

void World::setChunkData(int x, int y, int z, int width, int height, int depth, const std::vector<byte_t> &data)
{
    int chunkX = x >> 4;
    int chunkZ = z >> 4;
    int endChunkX = (x + width) - 1 >> 4;
    int endChunkZ = (z + depth) - 1 >> 4;
    int offset = 0;
    int minY = y;
    int maxY = y + height;
    
    if (minY < 0)
    {
        minY = 0;
    }
    if (maxY > 128)
    {
        maxY = 128;
    }
    
    for (int cx = chunkX; cx <= endChunkX; cx++)
    {
        int localMinX = x - cx * 16;
        int localMaxX = (x + width) - cx * 16;
        
        if (localMinX < 0)
        {
            localMinX = 0;
        }
        if (localMaxX > 16)
        {
            localMaxX = 16;
        }
        
        for (int cz = chunkZ; cz <= endChunkZ; cz++)
        {
            int localMinZ = z - cz * 16;
            int localMaxZ = (z + depth) - cz * 16;
            
            if (localMinZ < 0)
            {
                localMinZ = 0;
            }
            if (localMaxZ > 16)
            {
                localMaxZ = 16;
            }
            
            offset = getChunkFromChunkCoords(cx, cz)->setChunkData(const_cast<byte_t *>(data.data()), localMinX, minY, localMinZ, 
                                                                     localMaxX, maxY, localMaxZ, offset);
            markBlocksDirty(cx * 16 + localMinX - 1, minY, cz * 16 + localMinZ - 1,
                           cx * 16 + localMaxX + 1, maxY, cz * 16 + localMaxZ + 1);
        }
    }
}

void World::sendQuittingDisconnectingPacket()
{
}

void World::checkSessionLock()
{
    saveHandler->checkSessionLock();
}

void World::setWorldTime(long_t time)
{
    worldInfo->setWorldTime(time);
}

long_t World::getRandomSeed()
{
    return worldInfo->getRandomSeed();
}

long_t World::getWorldTime()
{
    return worldInfo->getWorldTime();
}

ChunkCoordinates* World::getSpawnPoint()
{
    return new ChunkCoordinates(worldInfo->getSpawnX(), worldInfo->getSpawnY(), worldInfo->getSpawnZ());
}

void World::setSpawnPoint(ChunkCoordinates* coordinates)
{
    worldInfo->setSpawn(coordinates->x, coordinates->y, coordinates->z);
}

void World::joinEntityInSurroundings(Entity* entity)
{
    int chunkX = MathHelper::floor_double(entity->posX / 16.0);
    int chunkZ = MathHelper::floor_double(entity->posZ / 16.0);
    const int RANGE = 2;
    
    for (int x = chunkX - RANGE; x <= chunkX + RANGE; x++)
    {
        for (int z = chunkZ - RANGE; z <= chunkZ + RANGE; z++)
        {
            getChunkFromChunkCoords(x, z);
        }
    }
    
    auto it = std::find(loadedEntityList.begin(), loadedEntityList.end(), entity);
    if (it == loadedEntityList.end())
    {
        loadedEntityList.push_back(entity);
    }
}

bool World::loadTexture(EntityPlayer* player, int x, int y, int z)
{
    return true;
}

void World::setEntityState(Entity* entity, byte_t byte0) // func_9425_a
{
}

void World::updateEntityList()
{
    for (auto it = unloadedEntityList.begin(); it != unloadedEntityList.end();)
    {
        Entity* entity = *it;
        auto findIt = std::find(loadedEntityList.begin(), loadedEntityList.end(), entity);
        if (findIt != loadedEntityList.end())
        {
            loadedEntityList.erase(findIt);
        }
        ++it;
    }
    
    for (size_t i = 0; i < unloadedEntityList.size(); i++)
    {
        Entity* entity = unloadedEntityList[i];
        int chunkX = entity->chunkCoordX;
        int chunkZ = entity->chunkCoordZ;
        
        if (entity->addedToChunk && chunkExists(chunkX, chunkZ))
        {
            getChunkFromChunkCoords(chunkX, chunkZ)->removeEntity(entity);
        }
    }
    
    for (size_t j = 0; j < unloadedEntityList.size(); j++)
    {
        releaseEntitySkin(unloadedEntityList[j]);
    }
    
    unloadedEntityList.clear();
    
    for (size_t k = 0; k < loadedEntityList.size(); k++)
    {
        Entity* entity = loadedEntityList[k];
        
        if (entity->ridingEntity != nullptr)
        {
            if (!entity->ridingEntity->isDead && entity->ridingEntity->riddenByEntity == entity)
            {
                continue;
            }
            
            entity->ridingEntity->riddenByEntity = nullptr;
            entity->ridingEntity = nullptr;
        }
        
        if (entity->isDead)
        {
            int chunkX = entity->chunkCoordX;
            int chunkZ = entity->chunkCoordZ;
            
            if (entity->addedToChunk && chunkExists(chunkX, chunkZ))
            {
                getChunkFromChunkCoords(chunkX, chunkZ)->removeEntity(entity);
            }
            
            loadedEntityList.erase(loadedEntityList.begin() + k);
            k--;
            releaseEntitySkin(entity);
        }
    }
}

IChunkProvider* World::getIChunkProvider()
{
    return chunkProvider;
}

void World::playNoteAt(int x, int y, int z, int instrument, int pitch)
{
    int blockId = getBlockId(x, y, z);
    if (blockId > 0)
    {
        Block::blocksList[blockId]->playBlock(this, x, y, z, instrument, pitch);
    }
}

WorldInfo* World::getWorldInfo()
{
    return worldInfo;
}

void World::updateAllPlayersSleepingFlag()
{
    allPlayersSleeping = !playerEntities.empty();
    
    for (auto it = playerEntities.begin(); it != playerEntities.end(); ++it)
    {
        EntityPlayer* player = *it;
        if (!player->isPlayerSleeping())
        {
            allPlayersSleeping = false;
            break;
        }
    }
}

void World::wakeUpAllPlayers()
{
    allPlayersSleeping = false;
    
    for (auto it = playerEntities.begin(); it != playerEntities.end(); ++it)
    {
        EntityPlayer* player = *it;
        if (player->isPlayerSleeping())
        {
            player->wakeUpPlayer(false, false, true);
        }
    }
    
    stopPrecipitation();
}

bool World::isAllPlayersFullyAsleep()
{
    if (allPlayersSleeping && !multiplayerWorld)
    {
        for (auto it = playerEntities.begin(); it != playerEntities.end(); ++it)
        {
            EntityPlayer* player = *it;
            if (!player->isPlayerFullyAsleep())
            {
                return false;
            }
        }
        
        return true;
    }
    
    return false;
}

float World::func_27166_f(float partialTicks)
{
    return (prevThunderingStrength + (thunderingStrength - prevThunderingStrength) * partialTicks) * func_27162_g(partialTicks);
}

float World::func_27162_g(float partialTicks)
{
    return prevRainingStrength + (rainingStrength - prevRainingStrength) * partialTicks;
}

float World::getRainStrength(float f)
{
    return func_27162_g(f);
}

float World::getThunderStrength(float f)
{
    return func_27166_f(f);
}

void World::func_27158_h(float strength)
{
    prevRainingStrength = strength;
    rainingStrength = strength;
}

bool World::func_27160_B()
{
    return (double)func_27166_f(1.0f) > 0.9;
}

bool World::func_27161_C()
{
    return (double)func_27162_g(1.0f) > 0.2;
}

bool World::canBlockBeRainedOn(int x, int y, int z)
{
    if (!func_27161_C())
    {
        return false;
    }
    
    if (!canBlockSeeTheSky(x, y, z))
    {
        return false;
    }
    
    if (findTopSolidBlock(x, z) > y)
    {
        return false;
    }
    
    BiomeGenBase* biome = getWorldChunkManager()->getBiomeGenAt(x, z);
    if (biome->getEnableSnow())
    {
        return false;
    }
    
    return biome->canSpawnLightningBolt();
}

void World::setItemData(const jstring& key, MapDataBase* mapData)
{
    mapStorage->setData(key, mapData);
}

MapDataBase* World::loadItemData(const std::type_info& classType, const jstring& key)
{
    return mapStorage->loadData(classType, key);
}

int World::getUniqueDataId(const jstring& key)
{
    return mapStorage->getUniqueDataId(key);
}

void World::func_28106_e(int x, int y, int z, int type, int data)
{
    func_28107_a(nullptr, x, y, z, type, data);
}

void World::func_28107_a(EntityPlayer* player, int type, int x, int y, int z, int data)
{
    for (size_t i = 0; i < worldAccesses.size(); i++)
    {
        worldAccesses[i]->playAuxSFX(player, type, x, y, z, data);
    }
}
bool World::isSafeToSave(int_t counter)
{
    return autosavePeriod != 0 && counter % autosavePeriod != 0;
}

void World::isTrapdoorOpen(int_t i, int_t j, int_t k, int_t l, int_t i1)
{
    func_28107_a(nullptr, i, j, k, l, i1);
}

void World::getTrapdoorFacing(EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t l, int_t i1)
{
    func_28107_a(entityplayer, i, j, k, l, i1);
}
