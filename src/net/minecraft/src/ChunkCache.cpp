#include "ChunkCache.h"

#include "World.h"
#include "Chunk.h"
#include "Block.h"
#include "Material.h"
#include "WorldProvider.h"

ChunkCache::ChunkCache(World *world, int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1)
	: worldObj(world)
{
	chunkX = i >> 4;
	chunkZ = k >> 4;
	int_t k1 = l >> 4;
	int_t l1 = j1 >> 4;
	chunkArrayWidth = (k1 - chunkX) + 1;
	chunkArrayDepth = (l1 - chunkZ) + 1;
	chunkArray.assign(chunkArrayWidth, std::vector<Chunk *>(chunkArrayDepth, nullptr));
	for (int_t i2 = chunkX; i2 <= k1; i2++)
	{
		for (int_t j2 = chunkZ; j2 <= l1; j2++)
		{
			{
		Chunk *chunk = world->getChunkFromChunkCoords(i2, j2);
		chunkArray[i2 - chunkX][j2 - chunkZ] = (chunk != nullptr && chunk->hasRealData) ? chunk : nullptr;
		}
		}
	}
}

ChunkCache::~ChunkCache()
{
}

int_t ChunkCache::getBlockId(int_t i, int_t j, int_t k)
{
	if (j < 0) return 0;
	if (j >= 128) return 0;
	int_t l  = (i >> 4) - chunkX;
	int_t i1 = (k >> 4) - chunkZ;
	if (l < 0 || l >= chunkArrayWidth || i1 < 0 || i1 >= chunkArrayDepth)
		return 0;
	Chunk *chunk = chunkArray[l][i1];
	if (chunk == nullptr) return 0;
	return chunk->getBlockID(i & 0xf, j, k & 0xf);
}

TileEntity *ChunkCache::getBlockTileEntity(int_t i, int_t j, int_t k)
{
    int_t l  = (i >> 4) - chunkX;
    int_t i1 = (k >> 4) - chunkZ;
    if (l < 0 || l >= chunkArrayWidth || i1 < 0 || i1 >= chunkArrayDepth)
        return nullptr;
    Chunk *chunk = chunkArray[l][i1];
    if (chunk == nullptr)
        return nullptr;
	return chunk->getChunkBlockTileEntity(i & 0xf, j, k & 0xf);
}

float ChunkCache::getBrightness(int_t i, int_t j, int_t k, int_t l)
{
	int_t i1 = getLightValue(i, j, k);
	if (i1 < l) i1 = l;
	return worldObj->worldProvider->lightBrightnessTable[i1];
}

float ChunkCache::getLightBrightness(int_t i, int_t j, int_t k)
{
	return worldObj->worldProvider->lightBrightnessTable[getLightValue(i, j, k)];
}

int_t ChunkCache::getLightValue(int_t i, int_t j, int_t k)
{
	return getLightValueExt(i, j, k, true);
}

int_t ChunkCache::getLightValueExt(int_t i, int_t j, int_t k, bool flag)
{
	// bounds check using the Java hex literals (matching original)
	if (i < (int_t)0xfe17b800 || k < (int_t)0xfe17b800 || i >= (int_t)0x1e84800 || k > (int_t)0x1e84800)
		return 15;
	if (flag)
	{
		int_t l = getBlockId(i, j, k);
		if (l == Block::stairSingle->blockID
			|| l == Block::tilledField->blockID
			|| l == Block::stairCompactPlanks->blockID
			|| l == Block::stairCompactCobblestone->blockID)
		{
			int_t k1 = getLightValueExt(i, j + 1, k, false);
			int_t i2 = getLightValueExt(i + 1, j, k, false);
			int_t j2 = getLightValueExt(i - 1, j, k, false);
			int_t k2 = getLightValueExt(i, j, k + 1, false);
			int_t l2 = getLightValueExt(i, j, k - 1, false);
			if (i2 > k1) k1 = i2;
			if (j2 > k1) k1 = j2;
			if (k2 > k1) k1 = k2;
			if (l2 > k1) k1 = l2;
			return k1;
		}
	}
	if (j < 0) return 0;
	if (j >= 128)
	{
		int_t i1 = 15 - worldObj->skylightSubtracted;
		if (i1 < 0) i1 = 0;
		return i1;
	}
	int_t j1 = (i >> 4) - chunkX;
	int_t l1 = (k >> 4) - chunkZ;
	// getLightValueExt recurses to i+/-1 and j+1, so coordinates can land
	// outside the cached chunk window. Guard like getBlockId; never index
	// or dereference an out-of-range / null chunk (heap corruption / null deref).
	if (j1 < 0 || j1 >= chunkArrayWidth || l1 < 0 || l1 >= chunkArrayDepth)
	{
		int_t i1 = 15 - worldObj->skylightSubtracted;
		if (i1 < 0) i1 = 0;
		return i1;
	}
	Chunk *chunk = chunkArray[j1][l1];
	if (chunk == nullptr)
	{
		int_t i1 = 15 - worldObj->skylightSubtracted;
		if (i1 < 0) i1 = 0;
		return i1;
	}
	return chunk->getBlockLightValue(i & 0xf, j, k & 0xf, worldObj->skylightSubtracted);
}

int_t ChunkCache::getBlockMetadata(int_t i, int_t j, int_t k)
{
	if (j < 0) return 0;
	if (j >= 128) return 0;
	int_t l  = (i >> 4) - chunkX;
	int_t i1 = (k >> 4) - chunkZ;
	if (l < 0 || l >= chunkArrayWidth || i1 < 0 || i1 >= chunkArrayDepth)
		return 0;
	Chunk *chunk = chunkArray[l][i1];
	if (chunk == nullptr)
		return 0;
	return chunk->getBlockMetadata(i & 0xf, j, k & 0xf);
}

Material *ChunkCache::getBlockMaterial(int_t i, int_t j, int_t k)
{
	int_t l = getBlockId(i, j, k);
	if (l == 0)
		return Material::air;
	Block *block = Block::blocksList[l];
	if (block == nullptr) // unregistered id (bad MP chunk data) — never deref null
		return Material::air;
	return block->blockMaterial;
}

WorldChunkManager *ChunkCache::getWorldChunkManager()
{
	return worldObj->getWorldChunkManager();
}

bool ChunkCache::isBlockOpaqueCube(int_t i, int_t j, int_t k)
{
	Block *block = Block::blocksList[getBlockId(i, j, k)];
	if (block == nullptr) return false;
	return block->isOpaqueCube();
}

bool ChunkCache::isBlockNormalCube(int_t i, int_t j, int_t k)
{
	Block *block = Block::blocksList[getBlockId(i, j, k)];
	if (block == nullptr) return false;
	return block->blockMaterial->getIsSolid() && block->renderAsNormalBlock();
}
