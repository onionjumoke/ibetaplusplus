#include "SaveHandler.h"

#include "ChunkLoader.h"
#include "McRegionChunkLoader.h"
#include "CompressedStreamTools.h"
#include "NBTTagCompound.h"
#include "WorldInfo.h"
#include "WorldProvider.h"
#include "WorldProviderHell.h"
#include "MinecraftException.h"
#include "java/IOUtil.h"
#include "java/String.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <cstdio>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <sys/stat.h>
#define MKDIR(p) mkdir(p, 0755)
#endif

static bool fileExists(const std::string &path)
{
    std::ifstream f(path);
    return f.good();
}

static void mkdirs(const std::string &path)
{
    // Simple recursive mkdir for subdirectory chains
    for (size_t i = 1; i < path.size(); i++)
    {
        if (path[i] == '/' || path[i] == '\\')
            MKDIR(path.substr(0, i).c_str());
    }
    MKDIR(path.c_str());
}

static void deleteFile(const std::string &path) { ::remove(path.c_str()); }
static void renameFile(const std::string &from, const std::string &to) { ::rename(from.c_str(), to.c_str()); }

SaveHandler::SaveHandler(const std::string &baseDir, const std::string &name, bool createPlayers) :
    saveDirectory(baseDir + "/" + name),
    playersDirectory(baseDir + "/" + name + "/players"),
    dataDirectory(baseDir + "/" + name + "/data")
{
    sessionTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    mkdirs(saveDirectory);
    mkdirs(dataDirectory);
    if (createPlayers)
        mkdirs(playersDirectory);

    writeSessionLock();
}

void SaveHandler::writeSessionLock()
{
    try
    {
        std::ofstream f(saveDirectory + "/session.lock", std::ios::binary);
        IOUtil::writeLong(f, sessionTime);
    }
    catch (...)
    {
        throw std::runtime_error("Failed to check session lock, aborting");
    }
}

void SaveHandler::checkSessionLock()
{
    try
    {
        std::ifstream f(saveDirectory + "/session.lock", std::ios::binary);
        long_t t = IOUtil::readLong(f);
        if (t != sessionTime)
            throw MinecraftException("The save is being accessed from another location, aborting");
    }
    catch (const MinecraftException &)
    {
        throw;
    }
    catch (...)
    {
        throw MinecraftException("Failed to check session lock, aborting");
    }
}

IChunkLoader* SaveHandler::getChunkLoader(WorldProvider *worldprovider)
{
    if (dynamic_cast<WorldProviderHell*>(worldprovider))
    {
        std::string dim1 = saveDirectory + "/DIM-1";
        mkdirs(dim1);
        return new ChunkLoader(File::open(String::fromUTF8(dim1)), true);
    }
    return new ChunkLoader(File::open(String::fromUTF8(saveDirectory)), true);
}

WorldInfo* SaveHandler::loadWorldInfo()
{
    for (const std::string &name : { "level.dat", "level.dat_old" })
    {
        std::string path = saveDirectory + "/" + name;
        if (!fileExists(path)) continue;
        try
        {
            std::ifstream f(path, std::ios::binary);
            NBTTagCompound *root = CompressedStreamTools::readGzippedCompound(f);
            return new WorldInfo(root->getCompoundTag(u"Data"));
        }
        catch (const std::exception &e)
        {
            // Do NOT fail silently: a null here makes the world load as "new"
            // with the seed passed by GuiSelectWorld (0), which is the classic
            // "terrain changed after reload" symptom.
            fprintf(stderr, "Beta++: loadWorldInfo failed for %s: %s\n", path.c_str(), e.what());
        }
        catch (...)
        {
            fprintf(stderr, "Beta++: loadWorldInfo failed for %s: unknown error\n", path.c_str());
        }
    }
    return nullptr;
}

void SaveHandler::saveWorldInfoAndPlayer(WorldInfo *worldinfo, const std::vector<EntityPlayer *> &players)
{
    NBTTagCompound *nbttagcompound = worldinfo->getNBTTagCompoundWithPlayer(players);
    NBTTagCompound root;
    root.setTag(u"Data", nbttagcompound);

    std::string pathNew = saveDirectory + "/level.dat_new";
    std::string pathOld = saveDirectory + "/level.dat_old";
    std::string pathCur = saveDirectory + "/level.dat";

    try
    {
        std::ofstream f(pathNew, std::ios::binary);
        CompressedStreamTools::writeGzippedCompoundToOutputStream(&root, f);
        f.close();

        if (fileExists(pathOld)) deleteFile(pathOld);
        renameFile(pathCur, pathOld);
        if (fileExists(pathCur)) deleteFile(pathCur);
        renameFile(pathNew, pathCur);
        if (fileExists(pathNew)) deleteFile(pathNew);
    }
    catch (...) {}
}

void SaveHandler::saveWorldInfo(WorldInfo *worldinfo)
{
    NBTTagCompound root;
    root.setTag(u"Data", worldinfo->getNBTTagCompound());

    std::string pathNew = saveDirectory + "/level.dat_new";
    std::string pathOld = saveDirectory + "/level.dat_old";
    std::string pathCur = saveDirectory + "/level.dat";

    try
    {
        std::ofstream f(pathNew, std::ios::binary);
        CompressedStreamTools::writeGzippedCompoundToOutputStream(&root, f);
        f.close();

        if (fileExists(pathOld)) deleteFile(pathOld);
        renameFile(pathCur, pathOld);
        if (fileExists(pathCur)) deleteFile(pathCur);
        renameFile(pathNew, pathCur);
        if (fileExists(pathNew)) deleteFile(pathNew);
    }
    catch (...) {}
}

std::string SaveHandler::getMapFile(const jstring &s)
{
    return dataDirectory + "/" + String::toUTF8(s) + ".dat";
}
