#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <mutex>
#include "java/Type.h"

// net.minecraft.src.RegionFile
// Uses zlib for compression (matching Java's DeflaterOutputStream / InflaterInputStream)
class RegionFile
{
public:
    explicit RegionFile(const std::string &path);
    ~RegionFile();

    int_t getSizeDelta();
    std::vector<byte_t> getChunkData(int_t x, int_t z);
    std::vector<byte_t> getChunkDataForWrite(int_t x, int_t z);
    void write(int_t x, int_t z, const byte_t *data, int_t length);
    void close();
    bool hasChunk(int_t x, int_t z);

    // Used only by RegionFileCache
    const std::string filePath;

private:
    bool outOfBounds(int_t x, int_t z);
    int_t getOffset(int_t x, int_t z);
    void setOffset(int_t x, int_t z, int_t value);
    void setTimestamp(int_t x, int_t z, int_t value);
    void writeSector(int_t sectorNum, const byte_t *data, int_t length);

    std::fstream dataFile;
    int_t offsets[1024];
    int_t timestamps[1024];
    std::vector<bool> sectorFree;
    int_t sizeDelta;
    std::mutex mtx;

    static const byte_t emptySector[4096];
};
