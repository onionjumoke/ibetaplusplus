#pragma once

#include <vector>
#include <set>
#include <unordered_set>
#include <typeinfo>

#include "java/Type.h"
#include "java/String.h"
#include "java/Random.h"

#include "IBlockAccess.h"
#include "EnumSkyBlock.h"
#include "EnumCreatureType.h"
#include "NextTickListEntry.h"
#include "ChunkCoordIntPair.h"

class WorldProvider;
class WorldInfo;
class MapStorage;
class ISaveHandler;
class IChunkProvider;
class IChunkLoader;
class Chunk;
class EntityPlayer;
class Entity;
class IWorldAccess;
class Vec3D;
class AxisAlignedBB;
class Material;
class MovingObjectPosition;
class TileEntity;
class Explosion;
class MetadataChunkBlock;
class ChunkCoordinates;
class PathEntity;
class MapDataBase;
class BiomeGenBase;
class NBTTagCompound;
class IProgressUpdate;
class WorldChunkManager;

// net.minecraft.src.World
class World : public IBlockAccess
{
public:
	WorldChunkManager *getWorldChunkManager() override;

	World(ISaveHandler *isavehandler, const jstring &s, WorldProvider *worldprovider, long_t l);
	World(World *world, WorldProvider *worldprovider);
	World(ISaveHandler *isavehandler, const jstring &s, long_t l);
	World(ISaveHandler *isavehandler, const jstring &s, long_t l, WorldProvider *worldprovider);
	~World() override;

protected:
	virtual IChunkProvider *getChunkProvider();
	virtual void getInitialSpawnLocation();

public:
	virtual void setSpawnLocation();
	int_t getFirstUncoveredBlock(int_t i, int_t j);
	void emptyMethod1();
	void spawnPlayerWithLoadedChunks(EntityPlayer *entityplayer);
	void saveWorld(bool flag, IProgressUpdate *iprogressupdate);

private:
	void saveLevel();

public:
	bool getAnimationFrame(int_t i);
	bool saveAllChunks(int_t progressStage);


	// Pause-menu "Unstuck": move a player out of solid blocks (or drop them in
	// from above as a last resort).
	void unstickPlayer(EntityPlayer *player);

	// Generator type applied to the next NEW world (0 default, 1 superflat).
	static void setNextWorldType(int_t t);

	int_t getBlockId(int_t i, int_t j, int_t k) override;
	bool isAirBlock(int_t i, int_t j, int_t k);
	bool blockExists(int_t i, int_t j, int_t k);
	bool doChunksNearChunkExist(int_t i, int_t j, int_t k, int_t l);
	bool checkChunksExist(int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1);

private:
	bool chunkExists(int_t i, int_t j);

public:
	Chunk *getChunkFromBlockCoords(int_t i, int_t j);
	Chunk *getChunkFromChunkCoords(int_t i, int_t j);
	virtual bool setBlockAndMetadata(int_t i, int_t j, int_t k, int_t l, int_t i1);
	virtual bool setBlock(int_t i, int_t j, int_t k, int_t l);
	Material *getBlockMaterial(int_t i, int_t j, int_t k) override;
	int_t getBlockMetadata(int_t i, int_t j, int_t k) override;
	void setBlockMetadataWithNotify(int_t i, int_t j, int_t k, int_t l);
	virtual bool setBlockMetadata(int_t i, int_t j, int_t k, int_t l);
	bool setBlockWithNotify(int_t i, int_t j, int_t k, int_t l);
	bool setBlockAndMetadataWithNotify(int_t i, int_t j, int_t k, int_t l, int_t i1);
	void markBlockNeedsUpdate(int_t i, int_t j, int_t k);

protected:
	void notifyBlockChange(int_t i, int_t j, int_t k, int_t l);

public:
	void markBlocksDirtyVertical(int_t i, int_t j, int_t k, int_t l);
	void markBlockAsNeedsUpdate(int_t i, int_t j, int_t k);
	void markBlocksDirty(int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1);
	void notifyBlocksOfNeighborChange(int_t i, int_t j, int_t k, int_t l);

private:
	void notifyBlockOfNeighborChange(int_t i, int_t j, int_t k, int_t l);

public:
	bool canBlockSeeTheSky(int_t i, int_t j, int_t k);
	int_t getFullBlockLightValue(int_t i, int_t j, int_t k);
	int_t getBlockLightValue(int_t i, int_t j, int_t k);
	int_t getBlockLightValue_do(int_t i, int_t j, int_t k, bool flag);
	bool canExistingBlockSeeTheSky(int_t i, int_t j, int_t k);
	int_t getHeightValue(int_t i, int_t j);
	void neighborLightPropagationChanged(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k, int_t l);
	int_t getSavedLightValue(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k);
	void setLightValue(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k, int_t l);
	float getBrightness(int_t i, int_t j, int_t k, int_t l) override;
	float getLightBrightness(int_t i, int_t j, int_t k) override;
	bool isDaytime();
	MovingObjectPosition *rayTraceBlocks(Vec3D *vec3d, Vec3D *vec3d1);
	MovingObjectPosition *rayTraceBlocks_do(Vec3D *vec3d, Vec3D *vec3d1, bool flag);
	MovingObjectPosition *rayTraceBlocks(Vec3D *start, Vec3D *end, bool stopOnLiquid, bool ignoreNonCollidable);
	MovingObjectPosition *getDoorMaterial(Vec3D *vec3d, Vec3D *vec3d1, bool flag, bool flag1);
	void playSoundAtEntity(Entity *entity, const jstring &s, float f, float f1);
	void playSoundEffect(double d, double d1, double d2, const jstring &s, float f, float f1);
	void playRecord(const jstring &s, int_t i, int_t j, int_t k);
	void spawnParticle(const jstring &s, double d, double d1, double d2, double d3, double d4, double d5);
	bool addWeatherEffect(Entity *entity);
	virtual bool entityJoinedWorld(Entity *entity);

protected:
	virtual void obtainEntitySkin(Entity *entity);
	virtual void releaseEntitySkin(Entity *entity);

public:
	virtual void setEntityDead(Entity *entity);
	// C++ port only (Java relied on GC): World owns entities and `delete`s them in
	// updateEntities. Non-owning indexes in subclasses (e.g. WorldClient's
	// entitySpawnQueue / knownEntities / entityHash) must drop the pointer before it
	// is freed, otherwise a stale pointer left in the spawn queue is dereferenced on
	// the next tick (dynamic_cast on freed memory -> access violation). Called right
	// before every `delete entity`. No-op in base World.
	virtual void onEntityRemoved(Entity *entity);
	void detachEntityForWorldChange(Entity *entity);
	void addWorldAccess(IWorldAccess *iworldaccess);
	void removeWorldAccess(IWorldAccess *iworldaccess);
	std::vector<AxisAlignedBB *> &getCollidingBoundingBoxes(Entity *entity, AxisAlignedBB *axisalignedbb);
	int_t calculateSkylightSubtracted(float f);
	Vec3D *getBlockLightValue_do(Entity *entity, float f);
	float getCelestialAngle(float f);
	Vec3D *getEntityBrightness(float f);
	Vec3D *getFogColor(float f);
	Vec3D *getSkyColor(Entity *entity, float f);   // func_4079_a
	Vec3D *getCloudFogColor(float f);              // func_628_d
	float getRainStrength(float f);
	float getThunderStrength(float f);
	int_t findTopSolidBlock(int_t i, int_t j);
	float getStarBrightness(float f);
	virtual void scheduleBlockUpdate(int_t i, int_t j, int_t k, int_t l, int_t i1);
	void updateEntities();
	void getPistonMovingProgress(const std::vector<TileEntity *> &collection);
	void updateEntity(Entity *entity);
	void updateEntityWithOptionalForce(Entity *entity, bool flag);
	bool checkIfAABBIsClear(AxisAlignedBB *axisalignedbb);
	bool getIsAnyLiquid(AxisAlignedBB *axisalignedbb);
	bool isBoundingBoxBurning(AxisAlignedBB *axisalignedbb);
	bool handleMaterialAcceleration(AxisAlignedBB *axisalignedbb, Material *material, Entity *entity);
	bool isMaterialInBB(AxisAlignedBB *axisalignedbb, Material *material);
	bool isAABBInMaterial(AxisAlignedBB *axisalignedbb, Material *material);
	Explosion *createExplosion(Entity *entity, double d, double d1, double d2, float f);
	Explosion *newExplosion(Entity *entity, double d, double d1, double d2, float f, bool flag);
	float getLivingSound(Vec3D *vec3d, AxisAlignedBB *axisalignedbb);
	void onBlockHit(EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t l);
	Entity *getSkyBlockType(const std::type_info &class1);
	jstring getHurtSound();
	jstring getContainerItem();
	TileEntity *getBlockTileEntity(int_t i, int_t j, int_t k) override;
	void setBlockTileEntity(int_t i, int_t j, int_t k, TileEntity *tileentity);
	void removeBlockTileEntity(int_t i, int_t j, int_t k);
	bool isBlockOpaqueCube(int_t i, int_t j, int_t k) override;
	bool isBlockNormalCube(int_t i, int_t j, int_t k) override;
	void saveWorldIndirectly(IProgressUpdate *iprogressupdate);
	bool isSafeToSave(int_t counter);
	bool updatingLighting();
	void scheduleLightingUpdate(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1);
	void scheduleLightingUpdate_do(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1, bool flag);
	void calculateInitialSkylight();
	void setAllowedMobSpawns(bool flag, bool flag1);
	virtual void tick();

private:
	void initCraftingStats();

protected:
	virtual void updateWeather();

private:
	void stopPrecipitation();

protected:
	virtual void updateBlocksAndPlayCaveSounds();

public:
	virtual bool TickUpdates(bool flag);
	void randomDisplayUpdates(int_t i, int_t j, int_t k);
	std::vector<Entity *> &getEntitiesWithinAABBExcludingEntity(Entity *entity, AxisAlignedBB *axisalignedbb);
	std::vector<Entity *> getEntitiesWithinAABB(const std::type_info &class1, AxisAlignedBB *axisalignedbb);
	std::vector<Entity *> &getLoadedEntityList();
	// Java: func_698_b — chunk-modified notification when a TE state changes;
	// fans out to every IWorldAccess listener.
	void markTileEntityChunkModified(int_t i, int_t j, int_t k, TileEntity *tileentity);
	int_t countEntities(EnumCreatureTypeTag tag);
	void getShadowSize(const std::vector<Entity *> &list);
	void getEntityBrightnessForRender(const std::vector<Entity *> &list);
	void getTextureData();
	bool canBlockBePlacedAt(int_t i, int_t j, int_t k, int_t l, bool flag, int_t i1);
	PathEntity *getPathToEntity(Entity *entity, Entity *entity1, float f);
	PathEntity *getEntityPathToXYZ(Entity *entity, int_t i, int_t j, int_t k, float f);
	bool isBlockProvidingPowerTo(int_t i, int_t j, int_t k, int_t l);
	bool isBlockGettingPowered(int_t i, int_t j, int_t k);
	bool isBlockIndirectlyProvidingPowerTo(int_t i, int_t j, int_t k, int_t l);
	bool isBlockIndirectlyGettingPowered(int_t i, int_t j, int_t k);
	EntityPlayer *getClosestPlayerToEntity(Entity *entity, double d);
	EntityPlayer *getClosestPlayer(double d, double d1, double d2, double d3);
	EntityPlayer *getPlayerEntityByName(const jstring &s);
	void setChunkData(int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1, const std::vector<byte_t> &abyte0);
	virtual void sendQuittingDisconnectingPacket();
	void setMapStorage(MapStorage *storage);
	void checkSessionLock();
	void setWorldTime(long_t l);
	long_t getRandomSeed();
	long_t getWorldTime();
	ChunkCoordinates *getSpawnPoint();
	void setSpawnPoint(ChunkCoordinates *chunkcoordinates);
	void joinEntityInSurroundings(Entity *entity);
	bool loadTexture(EntityPlayer *entityplayer, int_t i, int_t j, int_t k);
	void getEntityLivingSound(Entity *entity, byte_t byte0);
	void updateEntityList();
	IChunkProvider *getIChunkProvider();
	void playNoteAt(int_t i, int_t j, int_t k, int_t l, int_t i1);
	void func_28106_e(int_t type, int_t x, int_t y, int_t z, int_t data);
	void func_28107_a(EntityPlayer *player, int_t type, int_t x, int_t y, int_t z, int_t data);
	WorldInfo *getWorldInfo();
	void updateAllPlayersSleepingFlag();

protected:
	void wakeUpAllPlayers();

public:
	bool isAllPlayersFullyAsleep();
	float initToolsStats(float f);
	float initBlockStats(float f);
	void registerStat(float f);
	bool initStats();
	bool initItemStats();
	bool canBlockBeRainedOn(int_t i, int_t j, int_t k);
	void setItemData(const jstring &s, MapDataBase *mapdatabase);
	MapDataBase *loadItemData(const std::type_info &class1, const jstring &s);
	int_t getUniqueDataId(const jstring &s);
	void isTrapdoorOpen(int_t i, int_t j, int_t k, int_t l, int_t i1);
	void getTrapdoorFacing(EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t l, int_t i1);

	bool scheduledUpdatesAreImmediate;

private:
	std::vector<MetadataChunkBlock *> lightingToUpdate;

public:
	std::vector<Entity *> loadedEntityList;

private:
	std::vector<Entity *> unloadedEntityList;
	std::set<NextTickListEntry *, NextTickListEntryComparator> scheduledTickTreeSet;
	std::unordered_set<NextTickListEntry *, NextTickListEntryHash, NextTickListEntryEqual> scheduledTickSet;

public:
	std::vector<TileEntity *> loadedTileEntityList;

private:
	std::vector<TileEntity *> entityID;

public:
	std::vector<EntityPlayer *> playerEntities;
	std::vector<Entity *> weatherEffects;

private:
	long_t maxDamage;

public:
	int_t skylightSubtracted;

protected:
	int_t itemName;
	const int_t itemIconIndex = 0x3c6ef35f;
	// Java defaults these to 0.0F. In C++ they MUST be initialized: if left as garbage that
	// happens to be NaN, updateWeather()'s clamps (NaN<0 and NaN>1 are both false) never fix it,
	// so func_27162_g/func_27166_f return NaN -> calculateSkylightSubtracted's (int)(f2*11) becomes
	// INT_MIN -> getBlockLightValue underflows to 0 everywhere -> hostile mobs spawn in daylight and
	// flowers/crops drop (canBlockStay sees light 0). Root cause of the [SPAWN-DBG]/[PLANT-DROP] bugs.
	float prevRainingStrength = 0.0f;
	float rainingStrength = 0.0f;
	float prevThunderingStrength = 0.0f;
	float thunderingStrength = 0.0f;
	int_t chatMessage;

public:
	int_t chatColor;
	bool editingBlocks;

private:
	long_t lockTimestamp;

protected:
	int_t autosavePeriod;

public:
	int_t difficultySetting = 0;
	Random rand;
	bool isNewWorld;
	WorldProvider *worldProvider;

protected:
	std::vector<IWorldAccess *> worldAccesses;
	IChunkProvider *chunkProvider;
	ISaveHandler *saveHandler;
	WorldInfo *worldInfo;

public:
	bool findingSpawnPoint;

private:
	bool allPlayersSleeping;

public:
	MapStorage *worldType;

private:
	std::vector<AxisAlignedBB *> collidingBoundingBoxes;
	bool pistonExtending;
	int_t lightingUpdatesCounter;
	bool spawnHostileMobs;
	bool spawnPeacefulMobs;
	static int_t lightingUpdatesScheduled;
	std::unordered_set<ChunkCoordIntPair *, ChunkCoordIntPairHash, ChunkCoordIntPairEqual> positionsToUpdate;
	int_t soundCounter;
	std::vector<Entity *> maxStackSize;

public:
	bool multiplayerWorld;

	// Lightning flash timer (field_27172_i in obfuscated Java)
	int_t field_27172_i;

	void setEntityState(Entity *entity, byte_t byte0);        // func_9425_a – fan out entity-state event to clients
	float getBlockDensity(Vec3D *vec, AxisAlignedBB *aabb);   // func_675_a – explosion ray-sampling
	void func_636_a(const std::vector<Entity *> &list);       // bulk-add entities to loadedEntityList
	void func_632_b(const std::vector<Entity *> &list);       // bulk-queue entities for unload
	void addLoadedTileEntities(const std::vector<TileEntity *> &collection);
	bool func_27160_B();                                       // true if rainingStrength > 0.9
	bool func_27161_C();                                       // true if rainingStrength > 0.2
	float func_27162_g(float partialTicks);                    // internal: raw rain lerp
	float func_27166_f(float partialTicks);                    // internal: thunder * rain
	void func_27158_h(float strength);                         // set rain strength instantly

	// Methods implemented in World.cpp but missing from header
	Entity *func_4085_a(const std::type_info &classType);
	std::string func_687_d();
	std::string func_21119_g();
	void func_656_j();
	void func_27163_E();

private:
	std::vector<TileEntity *> tileEntitiesToAdd;
	long_t field_1019_F;
	int_t updateLCG;
	int_t thunderTime;
	std::vector<Entity *> entitiesWithinAABBExcludingEntity;
	MapStorage *mapStorage;
	bool ownsSaveHandler = true;
	bool ownsMapStorage = true;
	bool updatingTileEntities = false;
};
