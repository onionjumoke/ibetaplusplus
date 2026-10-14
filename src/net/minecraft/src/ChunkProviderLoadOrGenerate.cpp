#include "ChunkProviderLoadOrGenerate.h"

#include "java/System.h"
#include <chrono>

static long_t g_genBudgetWindowStart = 0;
static long_t g_genBudgetUsedNs = 0;

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "World.h"
#include "Chunk.h"
#include "IChunkLoader.h"
#include "IProgressUpdate.h"
#include "java/String.h"

ChunkProviderLoadOrGenerate::ChunkProviderLoadOrGenerate()
	: blankChunk(nullptr)
	, chunkProvider(nullptr)
	, chunkLoader(nullptr)
	, worldObj(nullptr)
	, lastQueriedChunkXPos(0)
	, lastQueriedChunkZPos(0)
	, lastQueriedChunk(nullptr)
	, curChunkX(0)
	, curChunkY(0)
{
	std::memset(chunks, 0, sizeof(chunks));
}

ChunkProviderLoadOrGenerate::~ChunkProviderLoadOrGenerate()
{
	for (Chunk *&chunk : chunks)
	{
		if (chunk != nullptr && chunk != blankChunk)
			delete chunk;
		chunk = nullptr;
	}
	delete blankChunk;
	delete chunkLoader;
	delete chunkProvider;
}

void ChunkProviderLoadOrGenerate::setCurrentChunkOver(int_t i, int_t j)
{
	curChunkX = i;
	curChunkY = j;
}

bool ChunkProviderLoadOrGenerate::canChunkExist(int_t i, int_t j)
{
	const int_t byte0 = 15;
	return i >= curChunkX - byte0
		&& j >= curChunkY - byte0
		&& i <= curChunkX + byte0
		&& j <= curChunkY + byte0;
}

bool ChunkProviderLoadOrGenerate::chunkExists(int_t i, int_t j)
{
	if (!canChunkExist(i, j)) return false;
	if (i == lastQueriedChunkXPos && j == lastQueriedChunkZPos && lastQueriedChunk != nullptr)
		return true;
	int_t k  = i & 0x1f;
	int_t l  = j & 0x1f;
	int_t i1 = k + l * 32;
	return chunks[i1] != nullptr
		&& (chunks[i1] == blankChunk || chunks[i1]->isAtLocation(i, j));
}

Chunk *ChunkProviderLoadOrGenerate::prepareChunk(int_t i, int_t j)
{
	return provideChunk(i, j);
}

Chunk *ChunkProviderLoadOrGenerate::provideChunk(int_t i, int_t j)
{
	if (i == lastQueriedChunkXPos && j == lastQueriedChunkZPos && lastQueriedChunk != nullptr)
		return lastQueriedChunk;
	if (!worldObj->findingSpawnPoint && !canChunkExist(i, j))
		return blankChunk;

	int_t k  = i & 0x1f;
	int_t l  = j & 0x1f;
	int_t i1 = k + l * 32;

	if (!chunkExists(i, j))
	{
		// Streaming budget (OptiCraft-style): generating or decoding one chunk
		// can take tens of ms, and bursting dozens per tick caused the
		// multi-hundred-ms walking hitches. Allow only a few per 50 ms window;
		// callers just retry (renderers defer on hasRealData) so terrain fills
		// in smoothly instead of in stalls.
		if (!worldObj->findingSpawnPoint)
		{
			// Time-based: allow at most ~8 ms of generation/decoding per 50 ms
			// window. A count-based cap (4 chunks) still bursted 4 * ~15 ms
			// into one tick, which was the residual hitch.
			long_t budgetNow = System::currentTimeMillis();
			if (budgetNow - g_genBudgetWindowStart > 50)
			{
				g_genBudgetWindowStart = budgetNow;
				g_genBudgetUsedNs = 0;
			}
			if (g_genBudgetUsedNs >= 8000000LL)
				return blankChunk;
		}

		if (chunks[i1] != nullptr)
		{
			chunks[i1]->onChunkUnload();
			saveChunkToFile(chunks[i1]);
			saveExtraChunkData(chunks[i1]);
			if (chunks[i1] != blankChunk)
				delete chunks[i1];
		}
		int64_t genStart = std::chrono::duration_cast<std::chrono::nanoseconds>(
			std::chrono::steady_clock::now().time_since_epoch()).count();
		Chunk *chunk = loadChunkFromFile(i, j);
		if (chunk == nullptr)
		{
			if (chunkProvider == nullptr)
				chunk = blankChunk;
			else
			{
				chunk = chunkProvider->provideChunk(i, j);
				chunk->remapBlocks();
			}
		}
		if (!worldObj->findingSpawnPoint)
		{
			int64_t genEnd = std::chrono::duration_cast<std::chrono::nanoseconds>(
				std::chrono::steady_clock::now().time_since_epoch()).count();
			g_genBudgetUsedNs += (genEnd - genStart);
		}
		chunks[i1] = chunk;
		chunk->onChunkLoadData();
		if (chunks[i1] != nullptr)
			chunks[i1]->onChunkLoad();
		// Renderer system: real chunk data just landed — rebuild deferred
		// WorldRenderers covering this chunk now.
		// Expand one block past the chunk border: the neighbouring chunks'
		// renderers cull their boundary faces against this chunk, so they
		// must rebuild too. Without this a chunk that loaded later stays
		// half-rendered (missing edge faces) until the world is reloaded.
		worldObj->markBlocksDirty(i * 16 - 1, 0, j * 16 - 1, i * 16 + 16, 127, j * 16 + 16);

		if (!chunks[i1]->isTerrainPopulated
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
	lastQueriedChunkXPos = i;
	lastQueriedChunkZPos = j;
	lastQueriedChunk = chunks[i1];
	return chunks[i1];
}

Chunk *ChunkProviderLoadOrGenerate::loadChunkFromFile(int_t i, int_t j)
{
	if (chunkLoader == nullptr) return blankChunk;
	try
	{
		Chunk *chunk = chunkLoader->loadChunk(worldObj, i, j);
		if (chunk != nullptr)
			chunk->lastSaveTime = worldObj->getWorldTime();
		return chunk;
	}
	catch (...)
	{
		printf("ChunkProviderLoadOrGenerate::loadChunkFromFile – exception %d,%d\n", i, j);
	}
	return blankChunk;
}

void ChunkProviderLoadOrGenerate::saveExtraChunkData(Chunk *chunk)
{
	if (chunkLoader == nullptr) return;
	try
	{
		chunkLoader->saveExtraChunkData(worldObj, chunk);
	}
	catch (...)
	{
		printf("ChunkProviderLoadOrGenerate::saveExtraChunkData – exception\n");
	}
}

void ChunkProviderLoadOrGenerate::saveChunkToFile(Chunk *chunk)
{
	if (chunkLoader == nullptr) return;
	try
	{
		chunk->lastSaveTime = worldObj->getWorldTime();
		chunkLoader->saveChunk(worldObj, chunk);
	}
	catch (...)
	{
		printf("ChunkProviderLoadOrGenerate::saveChunkToFile – exception\n");
	}
}

void ChunkProviderLoadOrGenerate::populate(IChunkProvider *ichunkprovider, int_t i, int_t j)
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

bool ChunkProviderLoadOrGenerate::saveChunks(bool flag, IProgressUpdate *iprogressupdate)
{
	long_t t_start = System::currentTimeMillis();
	long_t t_save = 0;
	int_t i = 0;
	int_t j = 0;
	if (iprogressupdate != nullptr)
	{
		for (int_t k = 0; k < 1024; k++)
		{
			if (chunks[k] != nullptr && chunks[k]->needsSaving(flag))
				j++;
		}
	}
	int_t l = 0;
	for (int_t i1 = 0; i1 < 1024; i1++)
	{
		if (chunks[i1] == nullptr) continue;
		if (flag && !chunks[i1]->neverSave)
			saveExtraChunkData(chunks[i1]);
		if (!chunks[i1]->needsSaving(flag)) continue;
		{
			long_t s0 = System::currentTimeMillis();
			saveChunkToFile(chunks[i1]);
			t_save += System::currentTimeMillis() - s0;
		}
		chunks[i1]->isModified = false;
		if (++i == 2 && !flag)
		{
			long_t t_end = System::currentTimeMillis();
			if (t_end - t_start >= 30)
				(void)0;
			return false;
		}
		if (iprogressupdate != nullptr && ++l % 10 == 0)
			iprogressupdate->setLoadingProgress((l * 100) / j);
	}
	{
		long_t t_end = System::currentTimeMillis();
		if (t_end - t_start >= 30)
			(void)0;
	}
	if (flag)
	{
		if (chunkLoader == nullptr) return true;
		chunkLoader->saveExtraData();
	}
	return true;
}

bool ChunkProviderLoadOrGenerate::unload100OldestChunks()
{
	if (chunkLoader != nullptr)
		chunkLoader->addRandomArmor();
	return chunkProvider != nullptr && chunkProvider->unload100OldestChunks();
}

bool ChunkProviderLoadOrGenerate::canSave()
{
	return true;
}

jstring ChunkProviderLoadOrGenerate::makeString()
{
	return u"ChunkCache: " + String::fromInt(1024);
}
