#include "AchievementMap.h"

#include <fstream>
#include <sstream>
#include <cstdio>

AchievementMap AchievementMap::instance;

AchievementMap::AchievementMap()
{
    try
    {
        std::ifstream file("achievement/map.txt");
        if (!file.is_open())
            return;

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty())
                continue;
            std::size_t comma = line.find(',');
            if (comma == std::string::npos)
                continue;
            int_t id = std::stoi(line.substr(0, comma));
            std::string guid = line.substr(comma + 1);
            guidMap[id] = guid;
        }
    }
    catch (...)
    {
        // silently ignore — matching Java catch(Exception exception) { e.printStackTrace() }
    }
}

std::string AchievementMap::getGuid(int_t i)
{
    auto it = instance.guidMap.find(i);
    if (it != instance.guidMap.end())
        return it->second;
    return "";
}
