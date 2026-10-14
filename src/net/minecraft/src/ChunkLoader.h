#pragma once

#include "IChunkLoader.h"
#include "java/Type.h"
#include "java/File.h"
#include "java/String.h"

class World;
class Chunk;
class NBTTagCompound;

// net.minecraft.src.ChunkLoader
class ChunkLoader : public IChunkLoader
{
public:
	ChunkLoader(File *file, bool flag);

	Chunk *loadChunk(World *world, int_t i, int_t j) override;
	void   saveChunk(World *world, Chunk *chunk) override;

	static void storeChunkInCompound(Chunk *chunk, World *world, NBTTagCompound *nbttagcompound);
	static Chunk *loadChunkIntoWorldFromCompound(World *world, NBTTagCompound *nbttagcompound);

	void addRandomArmor() override;   // func_814_a – no-op flush/tick
	void saveExtraData() override;
	void saveExtraChunkData(World *world, Chunk *chunk) override;

private:
	File *chunkFileForXZ(int_t i, int_t j);

	File *saveDir;
	bool  createIfNecessary;
};
