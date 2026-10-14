#include "DataWatcher.h"

#include "WatchableObject.h"
#include "ItemStack.h"
#include "ChunkCoordinates.h"
#include "Packet.h"
#include "java/IOUtil.h"
#include <stdexcept>
#include <sstream>

DataWatcher::DataWatcher() :
    objectChanged(false)
{
}

DataWatcher::~DataWatcher()
{
    for (auto &entry : watchedObjects)
        delete entry.second;
}

// Returns the type id for a given std::any value (matching Java's dataTypes map)
int_t DataWatcher::getTypeId(const std::any &obj)
{
    if (obj.type() == typeid(byte_t))   return 0;
    if (obj.type() == typeid(short_t))  return 1;
    if (obj.type() == typeid(int_t))    return 2;
    if (obj.type() == typeid(float))    return 3;
    if (obj.type() == typeid(std::string)) return 4;
    if (obj.type() == typeid(ItemStack*))  return 5;
    if (obj.type() == typeid(ChunkCoordinates*)) return 6;

    std::ostringstream oss;
    oss << "Unknown data type: " << obj.type().name();
    throw std::invalid_argument(oss.str());
}

void DataWatcher::addObject(int_t id, std::any obj)
{
    int_t typeId = getTypeId(obj);

    if (id > 31)
    {
        std::ostringstream oss;
        oss << "Data value id is too big with " << id << "! (Max is 31)";
        throw std::invalid_argument(oss.str());
    }

    if (watchedObjects.count(id))
    {
        std::ostringstream oss;
        oss << "Duplicate id value for " << id << "!";
        throw std::invalid_argument(oss.str());
    }

    watchedObjects[id] = new WatchableObject(typeId, id, std::move(obj));
}

byte_t DataWatcher::getWatchableObjectByte(int_t id)
{
    return std::any_cast<byte_t>(watchedObjects.at(id)->getObject());
}

int_t DataWatcher::getWatchableObjectInt(int_t id)
{
    return std::any_cast<int_t>(watchedObjects.at(id)->getObject());
}

std::string DataWatcher::getWatchableObjectString(int_t id)
{
    return std::any_cast<std::string>(watchedObjects.at(id)->getObject());
}

static bool anyEquals(const std::any &a, const std::any &b)
{
    if (a.type() != b.type()) return false;
    if (a.type() == typeid(byte_t))        return std::any_cast<byte_t>(a)        == std::any_cast<byte_t>(b);
    if (a.type() == typeid(short_t))       return std::any_cast<short_t>(a)       == std::any_cast<short_t>(b);
    if (a.type() == typeid(int_t))         return std::any_cast<int_t>(a)         == std::any_cast<int_t>(b);
    if (a.type() == typeid(float))         return std::any_cast<float>(a)         == std::any_cast<float>(b);
    if (a.type() == typeid(std::string))   return std::any_cast<std::string>(a)   == std::any_cast<std::string>(b);
    // ItemStack* and ChunkCoordinates* — pointer equality only; never falsely suppress network updates
    return false;
}

void DataWatcher::updateObject(int_t id, std::any obj)
{
    WatchableObject *wo = watchedObjects.at(id);
    if (!anyEquals(obj, wo->getObject()))
    {
        wo->setObject(std::move(obj));
        wo->setWatching(true);
        objectChanged = true;
    }
}

void DataWatcher::writeWatchableObject(std::ostream &os, WatchableObject *wo)
{
    int_t header = ((wo->getObjectType() << 5) | (wo->getDataValueId() & 0x1f)) & 0xff;
    IOUtil::writeByte(os, header);

    switch (wo->getObjectType())
    {
    case 0:
        IOUtil::writeByte(os, std::any_cast<byte_t>(wo->getObject()));
        break;
    case 1:
        IOUtil::writeShort(os, std::any_cast<short_t>(wo->getObject()));
        break;
    case 2:
        IOUtil::writeInt(os, std::any_cast<int_t>(wo->getObject()));
        break;
    case 3:
        IOUtil::writeFloat(os, std::any_cast<float>(wo->getObject()));
        break;
    case 4:
    {
        jstring s = String::fromUTF8(std::any_cast<std::string>(wo->getObject()));
        Packet::writeString(s, os);
        break;
    }
    case 5:
    {
        ItemStack *stack = std::any_cast<ItemStack*>(wo->getObject());
        IOUtil::writeShort(os, stack->itemID);
        IOUtil::writeByte(os, stack->stackSize);
        IOUtil::writeShort(os, stack->getItemDamage());
        break;
    }
    case 6:
    {
        ChunkCoordinates *cc = std::any_cast<ChunkCoordinates*>(wo->getObject());
        IOUtil::writeInt(os, cc->x);
        IOUtil::writeInt(os, cc->y);
        IOUtil::writeInt(os, cc->z);
        break;
    }
    }
}

void DataWatcher::writeObjectsInListToStream(const std::vector<WatchableObject*> &list, std::ostream &os)
{
    for (WatchableObject *wo : list)
        writeWatchableObject(os, wo);
    IOUtil::writeByte(os, 127);
}

void DataWatcher::writeWatchableObjects(std::ostream &os)
{
    for (auto &pair : watchedObjects)
        writeWatchableObject(os, pair.second);
    IOUtil::writeByte(os, 127);
}

std::vector<WatchableObject*> DataWatcher::readWatchableObjects(std::istream &is)
{
    std::vector<WatchableObject*> list;
    byte_t b = IOUtil::readByte(is);

    while (b != 127)
    {
        int_t typeId    = (b & 0xe0) >> 5;
        int_t valueId   = b & 0x1f;
        WatchableObject *wo = nullptr;

        switch (typeId)
        {
        case 0:
            wo = new WatchableObject(typeId, valueId, (byte_t)IOUtil::readByte(is));
            break;
        case 1:
            wo = new WatchableObject(typeId, valueId, (short_t)IOUtil::readShort(is));
            break;
        case 2:
            wo = new WatchableObject(typeId, valueId, (int_t)IOUtil::readInt(is));
            break;
        case 3:
            wo = new WatchableObject(typeId, valueId, (float)IOUtil::readFloat(is));
            break;
        case 4:
        {
            jstring s = Packet::readString(is, 64);
            wo = new WatchableObject(typeId, valueId, String::toUTF8(s));
            break;
        }
        case 5:
        {
            int_t itemId   = IOUtil::readShort(is);
            int_t count    = IOUtil::readByte(is);
            int_t damage   = IOUtil::readShort(is);
            wo = new WatchableObject(typeId, valueId, (ItemStack*)new ItemStack(itemId, count, damage));
            break;
        }
        case 6:
        {
            int_t x = IOUtil::readInt(is);
            int_t y = IOUtil::readInt(is);
            int_t z = IOUtil::readInt(is);
            wo = new WatchableObject(typeId, valueId, (ChunkCoordinates*)new ChunkCoordinates(x, y, z));
            break;
        }
        }

        if (wo != nullptr)
            list.push_back(wo);

        b = IOUtil::readByte(is);
    }

    return list;
}

void DataWatcher::updateWatchedObjectsFromList(const std::vector<WatchableObject*> &list)
{
    for (WatchableObject *incoming : list)
    {
        auto it = watchedObjects.find(incoming->getDataValueId());
        if (it != watchedObjects.end())
            it->second->setObject(incoming->getObject());
    }
}
