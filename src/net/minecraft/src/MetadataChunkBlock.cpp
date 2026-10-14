#include "MetadataChunkBlock.h"

#include "World.h"
#include "Chunk.h"
#include "Block.h"
#include "EnumSkyBlock.h"
#include <cstdio>

MetadataChunkBlock::MetadataChunkBlock(EnumSkyBlock *skyBlock, int_t minX, int_t minY, int_t minZ,
                                       int_t maxX, int_t maxY, int_t maxZ) :
    skyBlock(skyBlock),
    minX(minX), minY(minY), minZ(minZ),
    maxX(maxX), maxY(maxY), maxZ(maxZ)
{
}

void MetadataChunkBlock::updateLight(World *world)
{
    int_t dx = (maxX - minX) + 1;
    int_t dy = (maxY - minY) + 1;
    int_t dz = (maxZ - minZ) + 1;
    int_t total = dx * dy * dz;

    if (total > 32768)
    {
        printf("Light too large, skipping!\n");
        return;
    }

    int_t prevChunkX = 0, prevChunkZ = 0;
    bool hasPrev = false;
    bool prevValid = false;

    // Fast-path for World::getSavedLightValue: the metadata block is almost always
    // within a single chunk, yet each block reads itself + 6 neighbours, and every
    // World::getSavedLightValue call resolves the chunk twice through the provider
    // (chunkExists + getChunkFromChunkCoords) which only has a 1-entry cache that
    // thrashes at chunk borders. Cache the resolved Chunk* by chunk coords here.
    // Semantics are bit-identical to World::getSavedLightValue: Y is clamped to
    // [0,127]; out-of-bounds horizontally returns defaultLightValue; a non-existent
    // chunk returns 0. The chunkExists() guard is preserved so we never trigger
    // chunk generation the original wouldn't.
    Chunk *cachedChunk = nullptr;
    int_t cachedChunkX = 0, cachedChunkZ = 0;
    bool cacheValid = false;
    auto fastSavedLight = [&](int_t lx, int_t ly, int_t lz) -> int_t
    {
        if (ly < 0) ly = 0;
        else if (ly >= 128) ly = 127;

        if (lx < -0x01e84800 || lz < -0x01e84800 || lx >= 0x01e84800 || lz >= 0x01e84800)
            return skyBlock->defaultLightValue;

        int_t cx = lx >> 4;
        int_t cz = lz >> 4;
        if (!cacheValid || cx != cachedChunkX || cz != cachedChunkZ)
        {
            cachedChunkX = cx;
            cachedChunkZ = cz;
            cacheValid = true;
            // World::chunkExists is private (matches Java); blockExists(x,64,z) is the
            // public equivalent: with Y in range it returns chunkExists(x>>4, z>>4).
            cachedChunk = world->blockExists(lx, 64, lz)
                ? world->getChunkFromChunkCoords(cx, cz)
                : nullptr;
        }

        if (cachedChunk == nullptr)
            return 0;

        return cachedChunk->getSavedLightValue(const_cast<EnumSkyBlock*>(skyBlock), lx & 0xf, ly, lz & 0xf);
    };

    for (int_t x = minX; x <= maxX; x++)
    {
        for (int_t z = minZ; z <= maxZ; z++)
        {
            int_t chunkX = x >> 4;
            int_t chunkZ = z >> 4;
            bool valid = false;

            if (hasPrev && chunkX == prevChunkX && chunkZ == prevChunkZ)
            {
                valid = prevValid;
            }
            else
            {
                valid = world->doChunksNearChunkExist(x, 0, z, 1);
                if (valid)
                {
                    Chunk *chunk = world->getChunkFromChunkCoords(x >> 4, z >> 4);
                    if (chunk->isEmptyChunk())
                        valid = false;
                }
                prevValid = valid;
                prevChunkX = chunkX;
                prevChunkZ = chunkZ;
                hasPrev = true;
            }

            if (!valid) continue;

            int_t localMinY = minY;
            int_t localMaxY = maxY;
            if (localMinY < 0) localMinY = 0;
            if (localMaxY >= 128) localMaxY = 127;

            for (int_t y = localMinY; y <= localMaxY; y++)
            {
                int_t currentLight = fastSavedLight(x, y, z);
                int_t newLight = 0;
                int_t blockId = world->getBlockId(x, y, z);
                int_t opacity = Block::lightOpacity[blockId];
                if (opacity == 0) opacity = 1;

                if (skyBlock == EnumSkyBlock::Sky)
                {
                    if (world->canExistingBlockSeeTheSky(x, y, z))
                        newLight = 15;
                }
                else if (skyBlock == EnumSkyBlock::Block)
                {
                    newLight = Block::lightValue[blockId];
                }

                if (opacity < 15 || newLight > 0)
                {
                    int_t n = fastSavedLight(x - 1, y, z);
                    int_t best = n;
                    n = fastSavedLight(x + 1, y, z);
                    if (n > best) best = n;
                    n = fastSavedLight(x, y - 1, z);
                    if (n > best) best = n;
                    n = fastSavedLight(x, y + 1, z);
                    if (n > best) best = n;
                    n = fastSavedLight(x, y, z - 1);
                    if (n > best) best = n;
                    n = fastSavedLight(x, y, z + 1);
                    if (n > best) best = n;

                    best -= opacity;
                    if (best < 0) best = 0;
                    if (newLight > best) best = newLight;
                    newLight = best;
                }

                if (currentLight == newLight) continue;

                world->setLightValue(const_cast<EnumSkyBlock*>(skyBlock), x, y, z, newLight);
                int_t spread = newLight - 1;
                if (spread < 0) spread = 0;

                world->neighborLightPropagationChanged(const_cast<EnumSkyBlock*>(skyBlock), x - 1, y, z, spread);
                world->neighborLightPropagationChanged(const_cast<EnumSkyBlock*>(skyBlock), x, y - 1, z, spread);
                world->neighborLightPropagationChanged(const_cast<EnumSkyBlock*>(skyBlock), x, y, z - 1, spread);

                if (x + 1 >= maxX)
                    world->neighborLightPropagationChanged(const_cast<EnumSkyBlock*>(skyBlock), x + 1, y, z, spread);
                if (y + 1 >= maxY)
                    world->neighborLightPropagationChanged(const_cast<EnumSkyBlock*>(skyBlock), x, y + 1, z, spread);
                if (z + 1 >= maxZ)
                    world->neighborLightPropagationChanged(const_cast<EnumSkyBlock*>(skyBlock), x, y, z + 1, spread);
            }
        }
    }
}

bool MetadataChunkBlock::tryMerge(int_t inMinX, int_t inMinY, int_t inMinZ,
                                   int_t inMaxX, int_t inMaxY, int_t inMaxZ)
{
    // Exact containment check
    if (inMinX >= minX && inMinY >= minY && inMinZ >= minZ &&
        inMaxX <= maxX && inMaxY <= maxY && inMaxZ <= maxZ)
        return true;

    // Near-containment check with small expansion
    const int_t expand = 1;
    if (inMinX >= minX - expand && inMinY >= minY - expand && inMinZ >= minZ - expand &&
        inMaxX <= maxX + expand && inMaxY <= maxY + expand && inMaxZ <= maxZ + expand)
    {
        int_t oldVol = (maxX - minX) * (maxY - minY) * (maxZ - minZ);

        int_t newMinX = inMinX < minX ? inMinX : minX;
        int_t newMinY = inMinY < minY ? inMinY : minY;
        int_t newMinZ = inMinZ < minZ ? inMinZ : minZ;
        int_t newMaxX = inMaxX > maxX ? inMaxX : maxX;
        int_t newMaxY = inMaxY > maxY ? inMaxY : maxY;
        int_t newMaxZ = inMaxZ > maxZ ? inMaxZ : maxZ;

        int_t newVol = (newMaxX - newMinX) * (newMaxY - newMinY) * (newMaxZ - newMinZ);

        if (newVol - oldVol <= 2)
        {
            minX = newMinX; minY = newMinY; minZ = newMinZ;
            maxX = newMaxX; maxY = newMaxY; maxZ = newMaxZ;
            return true;
        }
    }

    return false;
}
