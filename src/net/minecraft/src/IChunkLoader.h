#pragma once

class World;
class Chunk;

// net.minecraft.src.IChunkLoader
class IChunkLoader
{
public:
	virtual ~IChunkLoader() = default;
	virtual Chunk *loadChunk(World *world, int i, int j) = 0;
	virtual void saveChunk(World *world, Chunk *chunk) = 0;
	virtual void saveExtraChunkData(World *world, Chunk *chunk) = 0;
	virtual void addRandomArmor() = 0;
	virtual void saveExtraData() = 0;
};
