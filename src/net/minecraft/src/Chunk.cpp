#include "Chunk.h"

#include <cstdio>
#include <algorithm>
#include <typeinfo>

#include "Block.h"
#include "BlockContainer.h"
#include "World.h"
#include "Entity.h"
#include "TileEntity.h"
#include "AxisAlignedBB.h"
#include "MathHelper.h"
#include "ChunkBlockMap.h"
#include "WorldProvider.h"

bool Chunk::isLit = false;

Chunk::Chunk(World *world, int_t i, int_t j)
	: worldObj(world)
	, xPosition(i)
	, zPosition(j)
	, data(0)
	, skylightMap(0)
	, blocklightMap(0)
	, isTerrainPopulated(false)
	, isModified(false)
	, hasEntities(false)
	, neverSave(false)
	, lastSaveTime(0LL)
	, isChunkLoaded(false)
	, lowestBlockHeight(0)
{
	std::fill(heightMap, heightMap + 256, (byte_t)0);
}

Chunk::Chunk(World *world, std::vector<byte_t> abyte0, int_t i, int_t j)
	: Chunk(world, i, j)
{
	blocks = std::move(abyte0);
	if ((int_t)blocks.size() != 16 * 16 * 128)
		blocks.assign(16 * 16 * 128, 0); // MP packet / corrupt save
	int_t sz = (int_t)blocks.size();
	data = NibbleArray(sz);
	skylightMap = NibbleArray(sz);
	blocklightMap = NibbleArray(sz);
}

bool Chunk::isAtLocation(int_t i, int_t j)
{
	return i == xPosition && j == zPosition;
}

int_t Chunk::getHeightValue(int_t i, int_t j)
{
	return heightMap[j << 4 | i] & 0xff;
}

void Chunk::initBlockLight()
{
}

void Chunk::generateHeightMap()
{
	int_t i = 127;
	for (int_t j = 0; j < 16; j++)
	{
		for (int_t k = 0; k < 16; k++)
		{
			int_t l = 127;
			for (int_t i1 = j << 11 | k << 7; l > 0 && Block::lightOpacity[blocks[(i1 + l) - 1] & 0xff] == 0; l--) {}
			heightMap[k << 4 | j] = (byte_t)l;
			if (l < i)
				i = l;
		}
	}
	lowestBlockHeight = i;
	isModified = true;
}

void Chunk::generateSkylightMap()
{
	int_t i = 127;
	for (int_t j = 0; j < 16; j++)
	{
		for (int_t l = 0; l < 16; l++)
		{
			int_t j1 = 127;
			int_t k1;
			for (k1 = j << 11 | l << 7; j1 > 0 && Block::lightOpacity[blocks[(k1 + j1) - 1] & 0xff] == 0; j1--) {}
			heightMap[l << 4 | j] = (byte_t)j1;
			if (j1 < i)
				i = j1;
			if (worldObj->worldProvider->hasNoSky)
				continue;
			int_t l1 = 15;
			int_t i2 = 127;
			do
			{
				l1 -= Block::lightOpacity[blocks[k1 + i2] & 0xff];
				if (l1 > 0)
					skylightMap.setNibble(j, i2, l, l1);
			} while (--i2 > 0 && l1 > 0);
		}
	}

	lowestBlockHeight = i;
	for (int_t k = 0; k < 16; k++)
	{
		for (int_t i1 = 0; i1 < 16; i1++)
		{
			propagateSkyLightEdges(k, i1);
		}
	}
	isModified = true;
}

void Chunk::onChunkLoadData()
{
}

void Chunk::propagateSkyLightEdges(int_t i, int_t j)
{
	int_t k = getHeightValue(i, j);
	int_t l  = xPosition * 16 + i;
	int_t i1 = zPosition * 16 + j;
	updateSkyLightNeighbor(l - 1, i1, k);
	updateSkyLightNeighbor(l + 1, i1, k);
	updateSkyLightNeighbor(l, i1 - 1, k);
	updateSkyLightNeighbor(l, i1 + 1, k);
}

void Chunk::updateSkyLightNeighbor(int_t i, int_t j, int_t k)
{
	int_t l = worldObj->getHeightValue(i, j);
	if (l > k)
	{
		worldObj->scheduleLightingUpdate(EnumSkyBlock::Sky, i, k, j, i, l, j);
		isModified = true;
	}
	else if (l < k)
	{
		worldObj->scheduleLightingUpdate(EnumSkyBlock::Sky, i, l, j, i, k, j);
		isModified = true;
	}
}

void Chunk::updateHeightmapAt(int_t i, int_t j, int_t k)
{
	int_t l  = heightMap[k << 4 | i] & 0xff;
	int_t i1 = l;
	if (j > l)
		i1 = j;
	for (int_t j1 = i << 11 | k << 7; i1 > 0 && Block::lightOpacity[blocks[(j1 + i1) - 1] & 0xff] == 0; i1--) {}
	if (i1 == l)
		return;
	worldObj->markBlocksDirtyVertical(i, k, i1, l);
	heightMap[k << 4 | i] = (byte_t)i1;
	if (i1 < lowestBlockHeight)
	{
		lowestBlockHeight = i1;
	}
	else
	{
		int_t k1 = 127;
		for (int_t i2 = 0; i2 < 16; i2++)
		{
			for (int_t k2 = 0; k2 < 16; k2++)
			{
				if ((heightMap[k2 << 4 | i2] & 0xff) < k1)
					k1 = heightMap[k2 << 4 | i2] & 0xff;
			}
		}
		lowestBlockHeight = k1;
	}
	int_t l1 = xPosition * 16 + i;
	int_t j2 = zPosition * 16 + k;
	if (i1 < l)
	{
		for (int_t l2 = i1; l2 < l; l2++)
			skylightMap.setNibble(i, l2, k, 15);
	}
	else
	{
		worldObj->scheduleLightingUpdate(EnumSkyBlock::Sky, l1, l, j2, l1, i1, j2);
		for (int_t i3 = l; i3 < i1; i3++)
			skylightMap.setNibble(i, i3, k, 0);
	}
	int_t j3 = 15;
	int_t k3 = i1;
	int_t tmp_i1 = i1;
	while (tmp_i1 > 0 && j3 > 0)
	{
		tmp_i1--;
		int_t l3 = Block::lightOpacity[getBlockID(i, tmp_i1, k)];
		if (l3 == 0)
			l3 = 1;
		j3 -= l3;
		if (j3 < 0)
			j3 = 0;
		skylightMap.setNibble(i, tmp_i1, k, j3);
	}
	for (; tmp_i1 > 0 && Block::lightOpacity[getBlockID(i, tmp_i1 - 1, k)] == 0; tmp_i1--) {}
	if (tmp_i1 != k3)
		worldObj->scheduleLightingUpdate(EnumSkyBlock::Sky, l1 - 1, tmp_i1, j2 - 1, l1 + 1, k3, j2 + 1);
	isModified = true;
}

int_t Chunk::getBlockID(int_t i, int_t j, int_t k)
{
	int_t idx = i << 11 | k << 7 | j;
	if (idx < 0 || idx >= (int_t)blocks.size())
		return 0;
	return blocks[idx] & 0xff;
}

bool Chunk::setBlockIDWithMetadata(int_t i, int_t j, int_t k, int_t l, int_t i1)
{
	byte_t byte0 = (byte_t)l;
	int_t j1 = heightMap[k << 4 | i] & 0xff;
	int_t k1 = blocks[i << 11 | k << 7 | j] & 0xff;
	if (k1 == l && data.getNibble(i, j, k) == i1)
		return false;
	int_t l1 = xPosition * 16 + i;
	int_t i2 = zPosition * 16 + k;
	blocks[i << 11 | k << 7 | j] = (byte_t)(byte0 & 0xff);
	if (k1 != 0 && !worldObj->multiplayerWorld)
		Block::blocksList[k1]->onBlockRemoval(worldObj, l1, j, i2);
	data.setNibble(i, j, k, i1);
	if (!worldObj->worldProvider->hasNoSky)
	{
		if (Block::lightOpacity[byte0 & 0xff] != 0)
		{
			if (j >= j1)
				updateHeightmapAt(i, j + 1, k);
		}
		else if (j == j1 - 1)
		{
			updateHeightmapAt(i, j, k);
		}
		worldObj->scheduleLightingUpdate(EnumSkyBlock::Sky, l1, j, i2, l1, j, i2);
	}
	worldObj->scheduleLightingUpdate(EnumSkyBlock::Block, l1, j, i2, l1, j, i2);
	propagateSkyLightEdges(i, k);
	data.setNibble(i, j, k, i1);
	if (l != 0)
		Block::blocksList[l]->onBlockAdded(worldObj, l1, j, i2);
	isModified = true;
	return true;
}

bool Chunk::setBlockID(int_t i, int_t j, int_t k, int_t l)
{
	byte_t byte0 = (byte_t)l;
	int_t i1 = heightMap[k << 4 | i] & 0xff;
	int_t j1 = blocks[i << 11 | k << 7 | j] & 0xff;
	if (j1 == l)
		return false;
	int_t k1 = xPosition * 16 + i;
	int_t l1 = zPosition * 16 + k;
	blocks[i << 11 | k << 7 | j] = (byte_t)(byte0 & 0xff);
	if (j1 != 0)
		Block::blocksList[j1]->onBlockRemoval(worldObj, k1, j, l1);
	data.setNibble(i, j, k, 0);
	if (Block::lightOpacity[byte0 & 0xff] != 0)
	{
		if (j >= i1)
			updateHeightmapAt(i, j + 1, k);
	}
	else if (j == i1 - 1)
	{
		updateHeightmapAt(i, j, k);
	}
	worldObj->scheduleLightingUpdate(EnumSkyBlock::Sky, k1, j, l1, k1, j, l1);
	worldObj->scheduleLightingUpdate(EnumSkyBlock::Block, k1, j, l1, k1, j, l1);
	propagateSkyLightEdges(i, k);
	if (l != 0 && !worldObj->multiplayerWorld)
		Block::blocksList[l]->onBlockAdded(worldObj, k1, j, l1);
	isModified = true;
	return true;
}

int_t Chunk::getBlockMetadata(int_t i, int_t j, int_t k)
{
	return data.getNibble(i, j, k);
}

void Chunk::setBlockMetadata(int_t i, int_t j, int_t k, int_t l)
{
	isModified = true;
	data.setNibble(i, j, k, l);
}

int_t Chunk::getSavedLightValue(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k)
{
	if (enumskyblock == EnumSkyBlock::Sky)
		return skylightMap.getNibble(i, j, k);
	if (enumskyblock == EnumSkyBlock::Block)
		return blocklightMap.getNibble(i, j, k);
	return 0;
}

void Chunk::setLightValue(EnumSkyBlock *enumskyblock, int_t i, int_t j, int_t k, int_t l)
{
	isModified = true;
	if (enumskyblock == EnumSkyBlock::Sky)
		skylightMap.setNibble(i, j, k, l);
	else if (enumskyblock == EnumSkyBlock::Block)
		blocklightMap.setNibble(i, j, k, l);
}

int_t Chunk::getBlockLightValue(int_t i, int_t j, int_t k, int_t l)
{
	int_t i1 = skylightMap.getNibble(i, j, k);
	if (i1 > 0)
		isLit = true;
	i1 -= l;
	int_t j1 = blocklightMap.getNibble(i, j, k);
	if (j1 > i1)
		i1 = j1;
	return i1;
}

void Chunk::addEntity(Entity *entity)
{
	hasEntities = true;
	int_t i = MathHelper::floor_double(entity->posX / 16.0);
	int_t j = MathHelper::floor_double(entity->posZ / 16.0);
	if (i != xPosition || j != zPosition)
	{
		printf("Wrong location! %p\n", (void *)entity);
	}
	int_t k = MathHelper::floor_double(entity->posY / 16.0);
	if (k < 0) k = 0;
	if (k >= (int_t)(sizeof(entities) / sizeof(entities[0])))
		k = (int_t)(sizeof(entities) / sizeof(entities[0])) - 1;
	entity->addedToChunk = true;
	entity->chunkCoordX = xPosition;
	entity->chunkCoordY = k;
	entity->chunkCoordZ = zPosition;
	entities[k].push_back(entity);
}

void Chunk::removeEntity(Entity *entity)
{
	removeEntityAtIndex(entity, entity->chunkCoordY);
}

void Chunk::removeEntityAtIndex(Entity *entity, int_t i)
{
	if (i < 0) i = 0;
	int_t len = (int_t)(sizeof(entities) / sizeof(entities[0]));
	if (i >= len) i = len - 1;
	auto &v = entities[i];
	v.erase(std::remove(v.begin(), v.end(), entity), v.end());
}

bool Chunk::canBlockSeeTheSky(int_t i, int_t j, int_t k)
{
	return j >= (heightMap[k << 4 | i] & 0xff);
}

TileEntity *Chunk::getChunkBlockTileEntity(int_t i, int_t j, int_t k)
{
	ChunkPosition chunkposition(i, j, k);
	auto it = chunkTileEntityMap.find(chunkposition);
	TileEntity *tileentity = (it != chunkTileEntityMap.end()) ? it->second : nullptr;
	if (tileentity == nullptr)
	{
		int_t l = getBlockID(i, j, k);
		if (!Block::isBlockContainer[l])
			return nullptr;
		BlockContainer *blockcontainer = static_cast<BlockContainer *>(Block::blocksList[l]);
		blockcontainer->onBlockAdded(worldObj, xPosition * 16 + i, j, zPosition * 16 + k);
		it = chunkTileEntityMap.find(chunkposition);
		tileentity = (it != chunkTileEntityMap.end()) ? it->second : nullptr;
	}
	if (tileentity != nullptr && tileentity->isInvalid())
	{
		chunkTileEntityMap.erase(chunkposition);
		return nullptr;
	}
	return tileentity;
}

void Chunk::addTileEntity(TileEntity *tileentity)
{
	int_t i = tileentity->xCoord - xPosition * 16;
	int_t j = tileentity->yCoord;
	int_t k = tileentity->zCoord - zPosition * 16;
	setChunkBlockTileEntity(i, j, k, tileentity);
	if (isChunkLoaded)
		worldObj->loadedTileEntityList.push_back(tileentity);
}

void Chunk::setChunkBlockTileEntity(int_t i, int_t j, int_t k, TileEntity *tileentity)
{
	ChunkPosition chunkposition(i, j, k);
	tileentity->worldObj = worldObj;
	tileentity->xCoord  = xPosition * 16 + i;
	tileentity->yCoord  = j;
	tileentity->zCoord  = zPosition * 16 + k;
	int_t blockId = getBlockID(i, j, k);
	if (blockId == 0 || !(Block::blocksList[blockId] && dynamic_cast<BlockContainer *>(Block::blocksList[blockId])))
	{
		printf("Attempted to place a tile entity where there was no entity tile!\n");
		return;
	}
	tileentity->validate();
	chunkTileEntityMap[chunkposition] = tileentity;
}

void Chunk::removeChunkBlockTileEntity(int_t i, int_t j, int_t k)
{
	ChunkPosition chunkposition(i, j, k);
	if (isChunkLoaded)
	{
		auto it = chunkTileEntityMap.find(chunkposition);
		if (it != chunkTileEntityMap.end())
		{
			TileEntity *tileentity = it->second;
			chunkTileEntityMap.erase(it);
			if (tileentity != nullptr)
				tileentity->invalidate();
		}
	}
}

void Chunk::onChunkLoad()
{
	isChunkLoaded = true;
	std::vector<TileEntity *> tileEntities;
	tileEntities.reserve(chunkTileEntityMap.size());
	for (auto &kv : chunkTileEntityMap)
		tileEntities.push_back(kv.second);
	worldObj->addLoadedTileEntities(tileEntities);
	int_t len = (int_t)(sizeof(entities) / sizeof(entities[0]));
	for (int_t i = 0; i < len; i++)
		worldObj->func_636_a(entities[i]);
}

void Chunk::onChunkUnload()
{
	isChunkLoaded = false;
	for (auto &kv : chunkTileEntityMap)
		kv.second->invalidate();
	int_t len = (int_t)(sizeof(entities) / sizeof(entities[0]));
	for (int_t i = 0; i < len; i++)
		worldObj->func_632_b(entities[i]);
}

void Chunk::setChunkModified()
{
	isModified = true;
}

void Chunk::getEntitiesWithinAABBForEntity(Entity *entity, AxisAlignedBB *axisalignedbb, std::vector<Entity *> &list)
{
	int_t i = MathHelper::floor_double((axisalignedbb->minY - 2.0) / 16.0);
	int_t j = MathHelper::floor_double((axisalignedbb->maxY + 2.0) / 16.0);
	if (i < 0) i = 0;
	int_t len = (int_t)(sizeof(entities) / sizeof(entities[0]));
	if (j >= len) j = len - 1;
	for (int_t k = i; k <= j; k++)
	{
		const std::vector<Entity *> &list1 = entities[k];
		for (int_t l = 0; l < (int_t)list1.size(); l++)
		{
			Entity *entity1 = list1[l];
			if (entity1 != entity && entity1->boundingBox->intersectsWith(axisalignedbb))
				list.push_back(entity1);
		}
	}
}

void Chunk::getEntitiesOfTypeWithinAAAB(const std::type_info &type, AxisAlignedBB *axisalignedbb, std::vector<Entity *> &list)
{
	int_t i = MathHelper::floor_double((axisalignedbb->minY - 2.0) / 16.0);
	int_t j = MathHelper::floor_double((axisalignedbb->maxY + 2.0) / 16.0);
	if (i < 0) i = 0;
	int_t len = (int_t)(sizeof(entities) / sizeof(entities[0]));
	if (j >= len) j = len - 1;
	for (int_t k = i; k <= j; k++)
	{
		const std::vector<Entity *> &list1 = entities[k];
		for (int_t l = 0; l < (int_t)list1.size(); l++)
		{
			Entity *entity = list1[l];
			if (typeid(*entity) == type && entity->boundingBox->intersectsWith(axisalignedbb))
				list.push_back(entity);
		}
	}
}

bool Chunk::needsSaving(bool flag)
{
	if (neverSave)
		return false;
	if (flag)
	{
		if (hasEntities && worldObj->getWorldTime() != lastSaveTime)
			return true;
	}
	else if (hasEntities && worldObj->getWorldTime() >= lastSaveTime + 600LL)
	{
		return true;
	}
	return isModified;
}

int_t Chunk::setChunkData(byte_t *abyte0, int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1, int_t k1)
{
	// Bounds guard: a desynced MapChunk header used to hand us huge ranges
	// (e.g. ySize 3949), and the index arithmetic below then wrote far past
	// the chunk arrays (heap corruption / SIGSEGV in memcpy).
	if (i < 0 || l > 16 || k < 0 || j1 > 16 || j < 0 || i1 > 128 || i >= l || j >= i1 || k >= j1)
	{
		fprintf(stderr, "Beta++: setChunkData REJECTED x[%d,%d) y[%d,%d) z[%d,%d)\n",
		        (int)i, (int)l, (int)j, (int)i1, (int)k, (int)j1);
		return k1;
	}
	hasRealData = true;
	for (int_t l1 = i; l1 < l; l1++)
	{
		for (int_t l2 = k; l2 < j1; l2++)
		{
			int_t l3 = l1 << 11 | l2 << 7 | j;
			int_t l4 = i1 - j;
			std::copy(abyte0 + k1, abyte0 + k1 + l4, blocks.data() + l3);
			k1 += l4;
		}
	}
	// Sanitize: MP data can contain ids this build has no Block for; every
	// later Block::blocksList[id]->... deref would crash. Treat as air.
	for (int_t sx = i; sx < l; sx++)
		for (int_t sz = k; sz < j1; sz++)
			for (int_t sy = j; sy < i1; sy++)
			{
				int_t sIdx = sx << 11 | sz << 7 | sy;
				int_t sId = blocks[sIdx] & 0xff;
				if (Block::blocksList[sId] == nullptr)
					blocks[sIdx] = 0;
			}
	generateHeightMap();
	for (int_t i2 = i; i2 < l; i2++)
	{
		for (int_t i3 = k; i3 < j1; i3++)
		{
			int_t i4 = (i2 << 11 | i3 << 7 | j) >> 1;
			int_t i5 = (i1 - j) / 2;
			std::copy(abyte0 + k1, abyte0 + k1 + i5, data.data.data() + i4);
			k1 += i5;
		}
	}
	for (int_t j2 = i; j2 < l; j2++)
	{
		for (int_t j3 = k; j3 < j1; j3++)
		{
			int_t j4 = (j2 << 11 | j3 << 7 | j) >> 1;
			int_t j5 = (i1 - j) / 2;
			std::copy(abyte0 + k1, abyte0 + k1 + j5, blocklightMap.data.data() + j4);
			k1 += j5;
		}
	}
	for (int_t k2 = i; k2 < l; k2++)
	{
		for (int_t k3 = k; k3 < j1; k3++)
		{
			int_t k4 = (k2 << 11 | k3 << 7 | j) >> 1;
			int_t k5 = (i1 - j) / 2;
			std::copy(abyte0 + k1, abyte0 + k1 + k5, skylightMap.data.data() + k4);
			k1 += k5;
		}
	}
	return k1;
}

Random Chunk::getRandomWithSeed(long_t l)
{
	return Random(worldObj->getRandomSeed()
		+ (long_t)(xPosition * xPosition * 0x4c1906)
		+ (long_t)(xPosition * 0x5ac0db)
		+ (long_t)(zPosition * zPosition) * 0x4307a7L
		+ (long_t)(zPosition * 0x5f24f)
		^ l);
}

bool Chunk::isEmptyChunk()
{
	return false;
}

void Chunk::remapBlocks()
{
	ChunkBlockMap::remapBlockArray(blocks.data(), (int_t)blocks.size());
}
