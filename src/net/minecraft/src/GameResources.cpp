#include "GameResources.h"
#include <algorithm>
#include <filesystem>

#include <SDL2/SDL.h>

namespace fs = std::filesystem;

namespace GameResources
{

std::string getExeDir()
{
    char *basePath = SDL_GetBasePath();
    if (basePath != nullptr)
    {
        fs::path path(basePath);
        SDL_free(basePath);
        return path.lexically_normal().string();
    }
    return fs::current_path().string();
}

std::string resolve(const std::string &mcPath)
{
    std::string path = mcPath;
    if (!path.empty() && path[0] == '/')
        path = path.substr(1);

    fs::path resolved = fs::path(getExeDir()) / "assets" / fs::path(path);
    std::ifstream input(resolved, std::ios::binary);
    return input.good() ? resolved.string() : "";
}

std::unique_ptr<std::ifstream> open(const std::string &mcPath)
{
    std::string resolved = resolve(mcPath);
    if (resolved.empty())
        return nullptr;
    auto f = std::make_unique<std::ifstream>(resolved, std::ios::binary);
    if (!f->good())
        return nullptr;
    return f;
}

} // namespace GameResources
