#pragma once

#include <vector>
#include "IBlockAccess.h"
#include "java/Type.h"

class World;
class Chunk;
class TileEntity;
class Material;
class WorldChunkManager;

// net.minecraft.src.ChunkCache
class ChunkCache : public IBlockAccess
{
public:
	ChunkCache(World *world, int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1);
	~ChunkCache();

	int_t getBlockId(int_t i, int_t j, int_t k) override;
	TileEntity *getBlockTileEntity(int_t i, int_t j, int_t k) override;

	float getBrightness(int_t i, int_t j, int_t k, int_t l) override;
	float getLightBrightness(int_t i, int_t j, int_t k) override;
	int_t getLightValue(int_t i, int_t j, int_t k);
	int_t getLightValueExt(int_t i, int_t j, int_t k, bool flag);

	int_t getBlockMetadata(int_t i, int_t j, int_t k) override;
	Material *getBlockMaterial(int_t i, int_t j, int_t k) override;
	WorldChunkManager *getWorldChunkManager() override;

	bool isBlockOpaqueCube(int_t i, int_t j, int_t k) override;
	bool isBlockNormalCube(int_t i, int_t j, int_t k) override;

private:
	int_t chunkX;
	int_t chunkZ;
	int_t chunkArrayWidth;
	int_t chunkArrayDepth;
	std::vector<std::vector<Chunk *>> chunkArray; // [x][z]
	World *worldObj;
};
