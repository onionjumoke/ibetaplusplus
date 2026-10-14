#include "McRegionChunkLoader.h"

#include "RegionFileCache.h"
#include "RegionFile.h"
#include "CompressedStreamTools.h"
#include "NBTTagCompound.h"
#include "ChunkLoader.h"
#include "Chunk.h"
#include "World.h"
#include "WorldInfo.h"
#include "java/System.h"
#include <sstream>
#include <cstdio>

McRegionChunkLoader::McRegionChunkLoader(const std::string &dir) :
    worldDir(dir)
{
}

Chunk* McRegionChunkLoader::loadChunk(World *world, int_t x, int_t z)
{
    std::istream *is = RegionFileCache::getChunkInputStream(worldDir, x, z);
    if (is == nullptr)
        return nullptr;

    NBTTagCompound *nbttagcompound = nullptr;
    try
    {
        nbttagcompound = CompressedStreamTools::readCompound(*is);
    }
    catch (...)
    {
        delete is;
        return nullptr;
    }
    delete is;

    if (!nbttagcompound->hasKey(u"Level"))
    {
        printf("Chunk file at %d,%d is missing level data, skipping\n", x, z);
        return nullptr;
    }

    NBTTagCompound *level = nbttagcompound->getCompoundTag(u"Level");
    if (!level->hasKey(u"Blocks"))
    {
        printf("Chunk file at %d,%d is missing block data, skipping\n", x, z);
        return nullptr;
    }

    Chunk *chunk = ChunkLoader::loadChunkIntoWorldFromCompound(world, level);

    if (!chunk->isAtLocation(x, z))
    {
        printf("Chunk file at %d,%d is in the wrong location; relocating. (Expected %d, %d, got %d, %d)\n",
               x, z, x, z, chunk->xPosition, chunk->zPosition);
        nbttagcompound->setInteger(u"xPos", x);
        nbttagcompound->setInteger(u"zPos", z);
        chunk = ChunkLoader::loadChunkIntoWorldFromCompound(world, nbttagcompound->getCompoundTag(u"Level"));
    }

    chunk->onChunkLoadData();
    return chunk;
}

void McRegionChunkLoader::saveChunk(World *world, Chunk *chunk)
{
    world->checkSessionLock();

    try
    {
        long_t t0 = System::currentTimeMillis();
        NBTTagCompound *nbttagcompound  = new NBTTagCompound();
        NBTTagCompound *nbttagcompound1 = new NBTTagCompound();
        nbttagcompound->setTag(u"Level", nbttagcompound1);
        ChunkLoader::storeChunkInCompound(chunk, world, nbttagcompound1);
        long_t t1 = System::currentTimeMillis();

        // Serialize to bytes
        std::ostringstream oss(std::ios::binary);
        CompressedStreamTools::writeCompound(nbttagcompound, oss);

        std::string raw = oss.str();
        long_t t2 = System::currentTimeMillis();

        // Write to region file
        RegionFile *rf = RegionFileCache::getRegionFile(worldDir, chunk->xPosition, chunk->zPosition);
        rf->write(chunk->xPosition & 0x1f, chunk->zPosition & 0x1f,
                  reinterpret_cast<const byte_t*>(raw.data()), (int_t)raw.size());
        long_t t3 = System::currentTimeMillis();

        WorldInfo *worldinfo = world->getWorldInfo();
        worldinfo->setSizeOnDisk(worldinfo->getSizeOnDisk() +
            (long_t)RegionFileCache::getSizeDelta(worldDir, chunk->xPosition, chunk->zPosition));
        long_t t4 = System::currentTimeMillis();

        if (t4 - t0 >= 30)
            fprintf(stderr, "Beta++: SAVECHUNK total=%ldms store=%ldms nbt=%ldms write=%ldms delta=%ldms bytes=%d\n",
                    (long)(t4 - t0), (long)(t1 - t0), (long)(t2 - t1), (long)(t3 - t2), (long)(t4 - t3),
                    (int)raw.size());
    }
    catch (...)
    {
        // silently catch, matching Java behavior
    }
}

void McRegionChunkLoader::saveExtraChunkData(World *world, Chunk *chunk)
{
}

void McRegionChunkLoader::addRandomArmor()
{
}

void McRegionChunkLoader::saveExtraData()
{
}
