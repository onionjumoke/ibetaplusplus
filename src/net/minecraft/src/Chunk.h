#pragma once

#include <vector>
#include <unordered_map>
#include <typeinfo>

#include "java/Type.h"
#include "java/Random.h"
#include "NibbleArray.h"
#include "ChunkPosition.h"
#include "EnumSkyBlock.h"

class World;
class Entity;
class TileEntity;
class BlockContainer;
class AxisAlignedBB;

struct ChunkPositionHash
{
	size_t operator()(const ChunkPosition &p) const { return (size_t)(uint_t)p.hashCode(); }
};

struct ChunkPositionEqual
{
	bool operator()(const ChunkPosition &a, const ChunkPosition &b) const { return a.equals(b); }
};

// net.minecraft.src.Chunk
class Chunk
{
public:
	Chunk(World *world, int_t i, int_t j);
	Chunk(World *world, std::vector<byte_t> abyte0, int_t i, int_t j);

	virtual bool isAtLocation(int_t i, int_t j);
	virtual int_t getHeightValue(int_t i, int_t j);

	virtual void initBlockLight();            // func_1014_a – empty init for block light recovery
	virtual void generateHeightMap();
	virtual void generateSkylightMap();       // func_1024_c – fills height map + sky light
	virtual void onChunkLoadData();           // func_4143_d – called after loading, empty in base

	int_t getBlockID(int_t i, int_t j, int_t k);
	virtual bool setBlockIDWithMetadata(int_t i, int_t j, int_t k, int_t l, int_t i1);
	virtual bool setBlockID(int_t i, int_t j, int_t k, int_t l);
	virtual int_t getBlockMetadata(int_t i, int_t j, int_t k);
	virtual void setBlockMetadata(int_t i, int_t j, int_t k, int_t l);

	virtual int_t getSavedLightValue(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k);
	virtual void  setLightValue(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k, int_t l);
	virtual int_t getBlockLightValue(int_t i, int_t j, int_t k, int_t l);

	virtual void addEntity(Entity *entity);
	virtual void removeEntity(Entity *entity);
	virtual void removeEntityAtIndex(Entity *entity, int_t i);
	virtual bool canBlockSeeTheSky(int_t i, int_t j, int_t k);

	virtual TileEntity *getChunkBlockTileEntity(int_t i, int_t j, int_t k);
	virtual void addTileEntity(TileEntity *tileentity);
	virtual void setChunkBlockTileEntity(int_t i, int_t j, int_t k, TileEntity *tileentity);
	virtual void removeChunkBlockTileEntity(int_t i, int_t j, int_t k);

	virtual void onChunkLoad();
	virtual void onChunkUnload();
	virtual void setChunkModified();

	virtual void getEntitiesWithinAABBForEntity(Entity *entity, AxisAlignedBB *axisalignedbb, std::vector<Entity *> &list);
	virtual void getEntitiesOfTypeWithinAAAB(const std::type_info &type, AxisAlignedBB *axisalignedbb, std::vector<Entity *> &list);

	virtual bool needsSaving(bool flag);

	virtual int_t setChunkData(byte_t *abyte0, int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1, int_t k1);

	Random getRandomWithSeed(long_t l);       // func_997_a
	virtual bool isEmptyChunk();              // func_21167_h – returns false; true in EmptyChunk
	void remapBlocks();                       // func_25124_i – remaps block IDs via ChunkBlockMap

	static bool isLit;

	std::vector<byte_t> blocks;
	bool isChunkLoaded;
	bool hasRealData = false;
	World *worldObj;
	NibbleArray data;
	NibbleArray skylightMap;
	NibbleArray blocklightMap;
	byte_t heightMap[256];
	int_t lowestBlockHeight;
	const int_t xPosition;
	const int_t zPosition;
	std::unordered_map<ChunkPosition, TileEntity *, ChunkPositionHash, ChunkPositionEqual> chunkTileEntityMap;
	std::vector<Entity *> entities[8];
	bool isTerrainPopulated;
	bool isModified;
	bool neverSave;
	bool hasEntities;
	long_t lastSaveTime;

private:
	void propagateSkyLightEdges(int_t i, int_t j);         // func_996_c
	void updateSkyLightNeighbor(int_t i, int_t j, int_t k); // func_1020_f
	void updateHeightmapAt(int_t i, int_t j, int_t k);      // func_1003_g
};
