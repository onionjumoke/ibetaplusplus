#include "SaveConverterMcRegion.h"

#include "SaveOldDir.h"
#include "SaveFormatComparator.h"
#include "WorldInfo.h"
#include "RegionFileCache.h"
#include "RegionFile.h"
#include "ChunkFile.h"
#include "ChunkFilePattern.h"
#include "IProgressUpdate.h"
#include "MathHelper.h"
#include "java/String.h"
#include <algorithm>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <zlib.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <sys/stat.h>
#include <dirent.h>
#define MKDIR(p) mkdir(p, 0755)
#endif

static bool isDirectory(const std::string &path)
{
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(path.c_str());
    return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
#endif
}

static std::vector<std::string> listDirectory(const std::string &path)
{
    std::vector<std::string> entries;
#ifdef _WIN32
    WIN32_FIND_DATAA data;
    HANDLE h = FindFirstFileA((path + "/*").c_str(), &data);
    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            std::string name = data.cFileName;
            if (name != "." && name != "..")
                entries.push_back(path + "/" + name);
        } while (FindNextFileA(h, &data));
        FindClose(h);
    }
#else
    DIR *dir = opendir(path.c_str());
    if (dir)
    {
        struct dirent *e;
        while ((e = readdir(dir)))
        {
            std::string name = e->d_name;
            if (name != "." && name != "..")
                entries.push_back(path + "/" + name);
        }
        closedir(dir);
    }
#endif
    return entries;
}

static std::string fileName(const std::string &path)
{
    size_t pos = path.find_last_of("/\\");
    return pos == std::string::npos ? path : path.substr(pos + 1);
}

static std::vector<byte_t> gzipDecompress(const std::vector<byte_t> &compressed)
{
    std::vector<byte_t> out(1024 * 1024);
    z_stream zs{};
    inflateInit2(&zs, 16 + MAX_WBITS);
    zs.next_in  = (Bytef*)compressed.data();
    zs.avail_in = (uInt)compressed.size();
    zs.next_out  = reinterpret_cast<Bytef *>(out.data());
    zs.avail_out = (uInt)out.size();
    inflate(&zs, Z_FINISH);
    uLong actual = zs.total_out;
    inflateEnd(&zs);
    out.resize(actual);
    return out;
}

SaveConverterMcRegion::SaveConverterMcRegion(const std::string &baseDir) :
    SaveFormatOld(baseDir)
{
}

jstring SaveConverterMcRegion::func_22178_a()
{
    return String::fromUTF8("Scaevolus' McRegion");
}

std::vector<SaveFormatComparator *> SaveConverterMcRegion::getSaveList()
{
    std::vector<SaveFormatComparator *> result;

    for (const std::string &entry : listDirectory(baseDirectory))
    {
        if (!isDirectory(entry)) continue;
        std::string name = fileName(entry);
        WorldInfo *info = getWorldInfoUTF8(name);
        if (!info) continue;

        bool needsConversion = info->getSaveVersion() != 19132;
        std::string displayName = String::toUTF8(info->getWorldName());
        if (displayName.empty()) displayName = name;

        result.push_back(new SaveFormatComparator(name, displayName, info->getLastTimePlayed(),
                                                   info->getSizeOnDisk(), needsConversion));
    }
    return result;
}

void SaveConverterMcRegion::flushCache()
{
    RegionFileCache::clearCache();
}

ISaveHandler* SaveConverterMcRegion::getSaveLoader(const jstring &s, bool flag)
{
    return new SaveOldDir(baseDirectory, String::toUTF8(s), flag);
}

bool SaveConverterMcRegion::isOldMapFormat(const jstring &s)
{
    WorldInfo *info = getWorldInfoUTF8(String::toUTF8(s));
    return info != nullptr && info->getSaveVersion() == 0;
}

void SaveConverterMcRegion::scanChunkFiles(const std::string &dir,
    std::vector<ChunkFile> &chunks, std::vector<std::string> &folders)
{
    for (const std::string &sub1 : listDirectory(dir))
    {
        if (!isDirectory(sub1)) continue;
        folders.push_back(sub1);

        for (const std::string &sub2 : listDirectory(sub1))
        {
            if (!isDirectory(sub2)) continue;
            for (const std::string &chunkPath : listDirectory(sub2))
            {
                std::string name = fileName(chunkPath);
                std::smatch m;
                if (std::regex_match(name, m, ChunkFilePattern::field_22189_a))
                {
                    File *f = File::open(String::fromUTF8(chunkPath));
                    chunks.push_back(ChunkFile(f));
                }
            }
        }
    }
}

void SaveConverterMcRegion::convertChunkFiles(const std::string &worldDir,
    std::vector<ChunkFile> &chunks, int_t doneCount, int_t totalCount, IProgressUpdate *progress)
{
    std::sort(chunks.begin(), chunks.end());
    std::vector<byte_t> buf(4096);

    for (ChunkFile &cf : chunks)
    {
        int_t cx = cf.getChunkX();
        int_t cz = cf.getChunkZ();

        RegionFile *rf = RegionFileCache::getRegionFile(worldDir, cx, cz);
        if (!rf->hasChunk(cx & 0x1f, cz & 0x1f))
        {
            try
            {
                // Read old gzip-compressed chunk file, decompress, write raw to region
                std::ifstream fin(String::toUTF8(cf.getFile()->toString()), std::ios::binary);
                std::vector<byte_t> compressed((std::istreambuf_iterator<char>(fin)), {});
                std::vector<byte_t> raw = gzipDecompress(compressed);
                rf->write(cx & 0x1f, cz & 0x1f, raw.data(), (int_t)raw.size());
            }
            catch (...) {}
        }

        doneCount++;
        if (totalCount > 0)
            progress->setLoadingProgress((int_t)std::round(100.0 * doneCount / totalCount));
    }

    RegionFileCache::clearCache();
}

void SaveConverterMcRegion::deleteFolders(std::vector<std::string> &folders,
    int_t doneCount, int_t totalCount, IProgressUpdate *progress)
{
    for (const std::string &folder : folders)
    {
        for (const std::string &entry : listDirectory(folder))
            ::remove(entry.c_str());
        ::rmdir(folder.c_str());

        doneCount++;
        if (totalCount > 0)
            progress->setLoadingProgress((int_t)std::round(100.0 * doneCount / totalCount));
    }
}

bool SaveConverterMcRegion::convertMapFormat(const jstring &s, IProgressUpdate *iprogressupdate)
{
    iprogressupdate->setLoadingProgress(0);

    std::string worldName = String::toUTF8(s);
    std::string worldDir  = baseDirectory + "/" + worldName;
    std::string dim1Dir   = worldDir + "/DIM-1";

    std::vector<ChunkFile> chunks, chunks1;
    std::vector<std::string> folders, folders1;

    printf("Scanning folders...\n");
    scanChunkFiles(worldDir, chunks, folders);
    if (isDirectory(dim1Dir))
        scanChunkFiles(dim1Dir, chunks1, folders1);

    int_t total = (int_t)(chunks.size() + chunks1.size() + folders.size() + folders1.size());
    printf("Total conversion count is %d\n", total);

    convertChunkFiles(worldDir,  chunks,  0,                  total, iprogressupdate);
    convertChunkFiles(dim1Dir,   chunks1, (int_t)chunks.size(), total, iprogressupdate);

    WorldInfo *info = getWorldInfoUTF8(worldName);
    if (info)
    {
        info->setSaveVersion(19132);
        ISaveHandler *handler = getSaveLoader(s, false);
        handler->saveWorldInfo(info);
        delete handler;
    }

    deleteFolders(folders,  (int_t)(chunks.size() + chunks1.size()), total, iprogressupdate);
    deleteFolders(folders1, (int_t)(chunks.size() + chunks1.size() + folders.size()), total, iprogressupdate);

    return true;
}
