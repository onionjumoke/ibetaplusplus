#include "RegionFileCache.h"

#include "RegionFile.h"
#include "RegionFileChunkBuffer.h"
#include <sstream>
#include <fstream>
#include <cstdlib>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <sys/stat.h>
#define MKDIR(p) mkdir(p, 0755)
#endif

std::map<std::string, RegionFile*> RegionFileCache::cache;

// Build the path to the .mcr file for chunk (chunkX, chunkZ)
static std::string regionPath(const std::string &worldDir, int_t chunkX, int_t chunkZ)
{
    std::string regionDir = worldDir + "/region";
    MKDIR(regionDir.c_str());

    std::ostringstream oss;
    oss << regionDir << "/r." << (chunkX >> 5) << "." << (chunkZ >> 5) << ".mcr";
    return oss.str();
}

RegionFile* RegionFileCache::getRegionFile(const std::string &worldDir, int_t chunkX, int_t chunkZ)
{
    std::string path = regionPath(worldDir, chunkX, chunkZ);

    auto it = cache.find(path);
    if (it != cache.end())
        return it->second;

    if ((int_t)cache.size() >= 256)
        clearCache();

    RegionFile *rf = new RegionFile(path);
    cache[path] = rf;
    return rf;
}

void RegionFileCache::clearCache()
{
    for (auto &pair : cache)
    {
        pair.second->close();
        delete pair.second;
    }
    cache.clear();
}

int_t RegionFileCache::getSizeDelta(const std::string &worldDir, int_t chunkX, int_t chunkZ)
{
    return getRegionFile(worldDir, chunkX, chunkZ)->getSizeDelta();
}

// Returns a stringstream-backed istream with the decompressed chunk data, or nullptr
std::istream* RegionFileCache::getChunkInputStream(const std::string &worldDir, int_t chunkX, int_t chunkZ)
{
    RegionFile *rf = getRegionFile(worldDir, chunkX, chunkZ);
    std::vector<byte_t> data = rf->getChunkData(chunkX & 0x1f, chunkZ & 0x1f);
    if (data.empty())
        return nullptr;

    std::string s(reinterpret_cast<char*>(data.data()), data.size());
    return new std::istringstream(s, std::ios::binary);
}

// Returns an ostream that, when destroyed/closed, compresses and writes via RegionFile::write
// We implement this via RegionFileChunkBuffer's close() method. We wrap it with a stringstream
// and flush on close by using a helper.
std::ostream* RegionFileCache::getChunkOutputStream(const std::string &worldDir, int_t chunkX, int_t chunkZ)
{
    // We use a stringstream as write buffer; caller calls close() on the RegionFileChunkBuffer wrapper.
    // Since C++ doesn't have Java's DataOutputStream wrapping, we return an ostringstream.
    // The McRegionChunkLoader calls dataoutputstream.close() which we intercept via the RegionFileChunkBuffer.
    // We return a std::ostringstream here; McRegionChunkLoader must explicitly flush to RegionFile.
    // This is handled in McRegionChunkLoader::saveChunk.
    return new std::ostringstream(std::ios::binary);
}
