#pragma once

#include "java/Type.h"
#include "java/String.h"
#include <functional>
#include <map>
#include <typeindex>

class World;
class Entity;
class NBTTagCompound;

// net.minecraft.src.EntityList
class EntityList
{
public:
    static Entity *createEntityInWorld(jstring s, World *world);
    static Entity *createEntityFromNBT(NBTTagCompound *nbttagcompound, World *world);
    static Entity *createEntity(int_t i, World *world);
    static int_t getEntityID(Entity *entity);
    static jstring getEntityString(Entity *entity);

    // Java addMapping(Class, String, int) -> C++ template; factory replaces reflection
    template<typename T>
    static void addMapping(jstring s, int_t i)
    {
        auto factory = [](World *w) -> Entity * { return new T(w); };
        stringToClassMapping[s] = factory;
        classToStringMapping[std::type_index(typeid(T))] = s;
        IDtoClassMapping[i] = factory;
        classToIDMapping[std::type_index(typeid(T))] = i;
    }

private:
    static std::map<jstring, std::function<Entity *(World *)>> stringToClassMapping;
    static std::map<std::type_index, jstring> classToStringMapping;
    static std::map<int_t, std::function<Entity *(World *)>> IDtoClassMapping;
    static std::map<std::type_index, int_t> classToIDMapping;
};
