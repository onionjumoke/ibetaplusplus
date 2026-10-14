#pragma once

#include "java/Type.h"

#include "IChunkLoader.h"
#include <string>

class World;
class Chunk;

// net.minecraft.src.McRegionChunkLoader
class McRegionChunkLoader : public IChunkLoader
{
public:
    explicit McRegionChunkLoader(const std::string &worldDir);

    Chunk* loadChunk(World *world, int_t x, int_t z) override;
    void saveChunk(World *world, Chunk *chunk) override;
    void saveExtraChunkData(World *world, Chunk *chunk) override;
    void addRandomArmor() override;
    void saveExtraData() override;

private:
    std::string worldDir;
};
