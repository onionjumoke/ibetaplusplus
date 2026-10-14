#include "ChunkProvider.h"

#include <cstdio>
#include <algorithm>
#include <cstdlib>
#include <sstream>

#include "World.h"
#include "Chunk.h"
#include "EmptyChunk.h"
#include "IChunkLoader.h"
#include "ChunkCoordIntPair.h"
#include "java/String.h"
#include "java/System.h"
#include "MathHelper.h"
#include "client/Minecraft.h"
#include "GameSettings.h"
#include "EntityPlayerSP.h"

// NOTE: chunk generation is intentionally synchronous. An earlier time-based
// streaming budget returned a shared blank (all-air) chunk when the budget was
// spent, which made block lookups alternate between real and blank terrain and
// wedged entities in partially-loaded chunks. Correct block data matters more
// than avoiding the occasional generation hitch.

ChunkProvider::ChunkProvider(World *world, IChunkLoader *ichunkloader, IChunkProvider *ichunkprovider)
	: worldObj(world)
	, chunkLoader(ichunkloader)
	, chunkProvider(ichunkprovider)
{
	blankChunk = new EmptyChunk(world, std::vector<byte_t>(32768, 0), 0, 0);
}

ChunkProvider::~ChunkProvider()
{
	std::unordered_set<Chunk *> chunks(chunkList.begin(), chunkList.end());
	for (Chunk *chunk : chunks)
		if (chunk != blankChunk) delete chunk;
	delete blankChunk;
	delete chunkLoader;
	delete chunkProvider;
}

bool ChunkProvider::chunkExists(int_t i, int_t j)
{
	return chunkMap.count(ChunkCoordIntPair::chunkXZ2Int(i, j)) != 0;
}

Chunk *ChunkProvider::prepareChunk(int_t i, int_t j)
{
	int_t k = ChunkCoordIntPair::chunkXZ2Int(i, j);
	droppedChunksSet.erase(k);

	auto it = chunkMap.find(k);
	Chunk *chunk = (it != chunkMap.end()) ? it->second : nullptr;

	if (chunk == nullptr)
	{
		// NEVER throttle generation by handing back the shared blank chunk: it
		// is an all-air EmptyChunk at (0,0), so World::getBlockId alternated
		// between real and blank terrain and entities in "rendered but not
		// loaded" chunks (the player and distant mobs) collided inconsistently
		// and wedged in place. Generation is synchronous; the outer-ring budget
		// is deliberately removed.
		chunk = loadChunkFromFile(i, j);
		if (chunk == nullptr)
		{
			if (chunkProvider == nullptr)
				chunk = blankChunk;
			else
				chunk = chunkProvider->provideChunk(i, j);
		}

		chunkMap[k] = chunk;
		chunkList.push_back(chunk);
		if (chunk != nullptr)
		{
			chunk->onChunkLoadData();
			chunk->onChunkLoad();
		}
		if (!chunk->isTerrainPopulated
			&& chunkExists(i + 1, j + 1)
			&& chunkExists(i,     j + 1)
			&& chunkExists(i + 1, j))
		{
			populate(this, i, j);
		}
		if (chunkExists(i - 1, j) && !provideChunk(i - 1, j)->isTerrainPopulated
			&& chunkExists(i - 1, j + 1)
			&& chunkExists(i,     j + 1)
			&& chunkExists(i - 1, j))
		{
			populate(this, i - 1, j);
		}
		if (chunkExists(i, j - 1) && !provideChunk(i, j - 1)->isTerrainPopulated
			&& chunkExists(i + 1, j - 1)
			&& chunkExists(i,     j - 1)
			&& chunkExists(i + 1, j))
		{
			populate(this, i, j - 1);
		}
		if (chunkExists(i - 1, j - 1) && !provideChunk(i - 1, j - 1)->isTerrainPopulated
			&& chunkExists(i - 1, j - 1)
			&& chunkExists(i,     j - 1)
			&& chunkExists(i - 1, j))
		{
			populate(this, i - 1, j - 1);
		}
	}
	return chunk;
}

Chunk *ChunkProvider::provideChunk(int_t i, int_t j)
{
	auto it = chunkMap.find(ChunkCoordIntPair::chunkXZ2Int(i, j));
	if (it == chunkMap.end())
		return prepareChunk(i, j);
	return it->second;
}

Chunk *ChunkProvider::loadChunkFromFile(int_t i, int_t j)
{
	if (chunkLoader == nullptr) return nullptr;
	try
	{
		Chunk *chunk = chunkLoader->loadChunk(worldObj, i, j);
		if (chunk != nullptr)
			chunk->lastSaveTime = worldObj->getWorldTime();
		return chunk;
	}
	catch (...)
	{
		printf("ChunkProvider::loadChunkFromFile – exception loading %d,%d\n", i, j);
	}
	return nullptr;
}

void ChunkProvider::saveExtraChunkData(Chunk *chunk)
{
	if (chunkLoader == nullptr) return;
	try
	{
		chunkLoader->saveExtraChunkData(worldObj, chunk);
	}
	catch (...)
	{
		printf("ChunkProvider::saveExtraChunkData – exception\n");
	}
}

void ChunkProvider::saveChunkToFile(Chunk *chunk)
{
	if (chunkLoader == nullptr) return;
	try
	{
		chunk->lastSaveTime = worldObj->getWorldTime();
		chunkLoader->saveChunk(worldObj, chunk);
	}
	catch (...)
	{
		printf("ChunkProvider::saveChunkToFile – exception\n");
	}
}

void ChunkProvider::populate(IChunkProvider *ichunkprovider, int_t i, int_t j)
{
	Chunk *chunk = provideChunk(i, j);
	if (!chunk->isTerrainPopulated)
	{
		chunk->isTerrainPopulated = true;
		if (chunkProvider != nullptr)
		{
			chunkProvider->populate(ichunkprovider, i, j);
			chunk->setChunkModified();
		}
	}
}

bool ChunkProvider::saveChunks(bool flag, IProgressUpdate *iprogressupdate)
{
	int_t i = 0;
	for (int_t j = 0; j < (int_t)chunkList.size(); j++)
	{
		Chunk *chunk = chunkList[j];
		if (flag && !chunk->neverSave)
			saveExtraChunkData(chunk);
		if (!chunk->needsSaving(flag))
			continue;
		saveChunkToFile(chunk);
		chunk->isModified = false;
		// Smaller batch: a runtime save used to write up to 24 chunks in one
		// call (~0.5 s of synchronous flash I/O). 8 keeps the hitch short and
		// the rest are written on the next autosave.
		if (++i == 8 && !flag)
			return false;
	}
	if (flag)
	{
		if (chunkLoader == nullptr) return true;
		chunkLoader->saveExtraData();
	}
	return true;
}

bool ChunkProvider::unload100OldestChunks()
{
	// Each unload writes the chunk to disk synchronously. Vanilla's loop runs up
	// to 100 iterations, so crossing a border -- which drops a whole row of
	// chunks at once -- could save dozens of chunks inside a single tick, a
	// multi-hundred-ms hitch. Spread the disk work: at most this many unloads
	// per call. World::tick calls this every tick, so the queue still drains.
	const int_t maxUnloadsPerTick = 4;
	int_t unloaded = 0;
	while (unloaded < maxUnloadsPerTick && !droppedChunksSet.empty())
	{
		int_t key = *droppedChunksSet.begin();
		auto it = chunkMap.find(key);
		if (it == chunkMap.end())
		{
			droppedChunksSet.erase(droppedChunksSet.begin());
			continue;
		}
		Chunk *chunk = it->second;
		chunk->onChunkUnload();
		saveChunkToFile(chunk);
		saveExtraChunkData(chunk);
		droppedChunksSet.erase(droppedChunksSet.begin());
		chunkMap.erase(key);
		chunkList.erase(std::remove(chunkList.begin(), chunkList.end(), chunk), chunkList.end());
		if (chunk != blankChunk)
			delete chunk;
		unloaded++;
	}
	if (chunkLoader != nullptr)
		chunkLoader->addRandomArmor();
	return chunkProvider != nullptr && chunkProvider->unload100OldestChunks();
}

bool ChunkProvider::canSave()
{
	return true;
}

jstring ChunkProvider::makeString()
{
	return u"ServerChunkCache: " + String::fromInt((int_t)chunkMap.size())
		+ u" Drop: " + String::fromInt((int_t)droppedChunksSet.size());
}
