#pragma once

#include "java/String.h"

class Chunk;
class IProgressUpdate;

// net.minecraft.src.IChunkProvider
class IChunkProvider
{
public:
	virtual ~IChunkProvider() = default;
	virtual bool chunkExists(int i, int j) = 0;
	virtual Chunk *provideChunk(int i, int j) = 0;
	virtual Chunk *prepareChunk(int i, int j) = 0;
	virtual void populate(IChunkProvider *ichunkprovider, int i, int j) = 0;
	virtual bool saveChunks(bool flag, IProgressUpdate *iprogressupdate) = 0;
	virtual bool unload100OldestChunks() = 0;
	virtual bool canSave() = 0;
	virtual jstring makeString() = 0;
};
