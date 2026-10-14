#include "ChunkProviderClient.h"

#include <algorithm>
#include <sstream>

#include "World.h"
#include "Chunk.h"
#include "EmptyChunk.h"
#include "NibbleArray.h"
#include "ChunkCoordIntPair.h"
#include "java/String.h"

ChunkProviderClient::ChunkProviderClient(World *world)
	: worldObj(world)
{
	blankChunk = new EmptyChunk(world, std::vector<byte_t>(32768, 0), 0, 0);
}

ChunkProviderClient::~ChunkProviderClient()
{
	for (auto &entry : chunkMapping)
		delete entry.second;
	delete blankChunk;
}

bool ChunkProviderClient::chunkExists(int_t i, int_t j)
{
	return chunkMapping.find(ChunkCoordIntPair::chunkXZ2Int(i, j)) != chunkMapping.end();
}

void ChunkProviderClient::unloadChunk(int_t i, int_t j)
{
	Chunk *chunk = provideChunk(i, j);
	if (!chunk->isEmptyChunk())
		chunk->onChunkUnload();
	chunkMapping.erase(ChunkCoordIntPair::chunkXZ2Int(i, j));
	chunkList.erase(std::remove(chunkList.begin(), chunkList.end(), chunk), chunkList.end());
	if (chunk != blankChunk)
		delete chunk;
}

Chunk *ChunkProviderClient::prepareChunk(int_t i, int_t j)
{
	// Idempotent: servers re-send PreChunk whenever the view shifts (and on
	// every teleport). The old version always allocated a fresh empty chunk
	// and replaced the map entry, silently wiping received terrain -> the
	// "some chunks never render" MP bug.
	auto existing = chunkMapping.find(ChunkCoordIntPair::chunkXZ2Int(i, j));
	if (existing != chunkMapping.end())
		return existing->second;

	std::vector<byte_t> abyte0(32768, 0);
	Chunk *chunk = new Chunk(worldObj, abyte0, i, j);
	std::fill(chunk->skylightMap.data.begin(), chunk->skylightMap.data.end(), (byte_t)-1);
	chunkMapping[ChunkCoordIntPair::chunkXZ2Int(i, j)] = chunk;
	chunkList.push_back(chunk);
	chunk->isChunkLoaded = true;
	return chunk;
}

Chunk *ChunkProviderClient::provideChunk(int_t i, int_t j)
{
	auto it = chunkMapping.find(ChunkCoordIntPair::chunkXZ2Int(i, j));
	if (it == chunkMapping.end())
		return blankChunk;
	return it->second;
}

bool ChunkProviderClient::saveChunks(bool flag, IProgressUpdate *iprogressupdate)
{
	return true;
}

bool ChunkProviderClient::unload100OldestChunks()
{
	return false;
}

bool ChunkProviderClient::canSave()
{
	return false;
}

void ChunkProviderClient::populate(IChunkProvider *ichunkprovider, int_t i, int_t j)
{
}

jstring ChunkProviderClient::makeString()
{
	return u"MultiplayerChunkCache: " + String::fromInt((int_t)chunkMapping.size());
}
