#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "IChunkProvider.h"
#include "java/Type.h"
#include "java/String.h"

class World;
class Chunk;
class IChunkLoader;
class IProgressUpdate;

// net.minecraft.src.ChunkProvider
class ChunkProvider : public IChunkProvider
{
public:
	ChunkProvider(World *world, IChunkLoader *ichunkloader, IChunkProvider *ichunkprovider);
	~ChunkProvider() override;

	bool    chunkExists(int_t i, int_t j) override;
	Chunk  *prepareChunk(int_t i, int_t j) override;
	Chunk  *provideChunk(int_t i, int_t j) override;
	void    populate(IChunkProvider *ichunkprovider, int_t i, int_t j) override;
	bool    saveChunks(bool flag, IProgressUpdate *iprogressupdate) override;
	bool    unload100OldestChunks() override;
	bool    canSave() override;
	jstring makeString() override;

private:
	Chunk *loadChunkFromFile(int_t i, int_t j);
	void   saveExtraChunkData(Chunk *chunk);  // func_28063_a
	void   saveChunkToFile(Chunk *chunk);     // func_28062_b

	std::unordered_set<int_t>        droppedChunksSet;
	Chunk                           *blankChunk;       // field_28064_b
	IChunkProvider                  *chunkProvider;
	IChunkLoader                    *chunkLoader;
	std::unordered_map<int_t, Chunk *> chunkMap;
	std::vector<Chunk *>              chunkList;
	World                           *worldObj;         // field_28066_g
};
