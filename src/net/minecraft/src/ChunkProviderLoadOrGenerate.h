#pragma once

#include "IChunkProvider.h"
#include "java/Type.h"
#include "java/String.h"

class World;
class Chunk;
class IChunkLoader;
class IProgressUpdate;

// net.minecraft.src.ChunkProviderLoadOrGenerate
class ChunkProviderLoadOrGenerate : public IChunkProvider
{
public:
	ChunkProviderLoadOrGenerate();
	~ChunkProviderLoadOrGenerate() override;

	void setCurrentChunkOver(int_t i, int_t j);
	bool canChunkExist(int_t i, int_t j);

	bool   chunkExists(int_t i, int_t j) override;
	Chunk *prepareChunk(int_t i, int_t j) override;
	Chunk *provideChunk(int_t i, int_t j) override;
	void   populate(IChunkProvider *ichunkprovider, int_t i, int_t j) override;
	bool   saveChunks(bool flag, IProgressUpdate *iprogressupdate) override;
	bool   unload100OldestChunks() override;
	bool   canSave() override;
	jstring makeString() override;

	Chunk        *blankChunk;
	IChunkProvider *chunkProvider;
	IChunkLoader   *chunkLoader;
	Chunk         *chunks[1024];
	World         *worldObj;
	int_t          lastQueriedChunkXPos;
	int_t          lastQueriedChunkZPos;

private:
	Chunk *loadChunkFromFile(int_t i, int_t j);  // func_542_c
	void   saveExtraChunkData(Chunk *chunk);
	void   saveChunkToFile(Chunk *chunk);

	Chunk *lastQueriedChunk;
	int_t  curChunkX;
	int_t  curChunkY;
};
