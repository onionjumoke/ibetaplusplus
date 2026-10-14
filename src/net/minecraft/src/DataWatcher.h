#pragma once

#include <map>
#include <vector>
#include <any>
#include <iostream>
#include "java/Type.h"

class WatchableObject;
class ItemStack;
class ChunkCoordinates;

// net.minecraft.src.DataWatcher
class DataWatcher
{
public:
    DataWatcher();
    ~DataWatcher();

    void addObject(int_t id, std::any obj);

    byte_t getWatchableObjectByte(int_t id);
    int_t getWatchableObjectInt(int_t id);
    std::string getWatchableObjectString(int_t id);

    void updateObject(int_t id, std::any obj);

    static void writeObjectsInListToStream(const std::vector<WatchableObject*> &list, std::ostream &os);
    void writeWatchableObjects(std::ostream &os);

    static std::vector<WatchableObject*> readWatchableObjects(std::istream &is);
    void updateWatchedObjectsFromList(const std::vector<WatchableObject*> &list);

private:
    static void writeWatchableObject(std::ostream &os, WatchableObject *obj);
    static int_t getTypeId(const std::any &obj);

    std::map<int_t, WatchableObject*> watchedObjects;
    bool objectChanged;
};
