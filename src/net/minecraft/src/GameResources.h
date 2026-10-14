#pragma once
#include <string>
#include <fstream>
#include <memory>

// Centralized resource path resolution.
// All resource lookups should go through here instead of each file having
// its own getExeDir() copy.
namespace GameResources
{
    // Returns the directory containing the .exe (e.g. "C:\...\bin\Debug")
    std::string getExeDir();

    // Resolves a Minecraft-style path (e.g. "/terrain.png" or "terrain.png")
    // inside {exeDir}/assets. Audio is intentionally handled separately by
    // ThreadDownloadResources from {exeDir}/resources.
    // Returns the path if it exists, or an empty string otherwise.
    std::string resolve(const std::string &mcPath);

    // Opens a resource stream. Returns nullptr if not found.
    std::unique_ptr<std::ifstream> open(const std::string &mcPath);
}
