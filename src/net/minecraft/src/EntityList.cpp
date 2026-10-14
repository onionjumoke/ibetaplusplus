#include "EntityList.h"

#include <iostream>

#include "SpawnerAnimals.h"
#include "EntityArrow.h"
#include "EntityBoat.h"
#include "EntityChicken.h"
#include "EntityCow.h"
#include "EntityCreeper.h"
#include "EntityFallingSand.h"
#include "EntityGhast.h"
#include "EntityGiantZombie.h"
#include "EntityItem.h"
#include "EntityLiving.h"
#include "EntityMinecart.h"
#include "EntityMob.h"
#include "EntityPainting.h"
#include "EntityPig.h"
#include "EntityPigZombie.h"
#include "EntitySheep.h"
#include "EntitySkeleton.h"
#include "EntitySlime.h"
#include "EntitySnowball.h"
#include "EntitySpider.h"
#include "EntitySquid.h"
#include "EntityTNTPrimed.h"
#include "EntityWolf.h"
#include "EntityZombie.h"
#include "NBTTagCompound.h"
#include "World.h"

std::map<jstring, std::function<Entity *(World *)>> EntityList::stringToClassMapping;
std::map<std::type_index, jstring> EntityList::classToStringMapping;
std::map<int_t, std::function<Entity *(World *)>> EntityList::IDtoClassMapping;
std::map<std::type_index, int_t> EntityList::classToIDMapping;

Entity *EntityList::createEntityInWorld(jstring s, World *world)
{
    Entity *entity = nullptr;
    try
    {
        auto it = stringToClassMapping.find(s);
        if (it != stringToClassMapping.end())
        {
            entity = it->second(world);
        }
    }
    catch (std::exception &exception)
    {
        std::cerr << exception.what() << std::endl;
    }
    return entity;
}

Entity *EntityList::createEntityFromNBT(NBTTagCompound *nbttagcompound, World *world)
{
    Entity *entity = nullptr;
    try
    {
        auto it = stringToClassMapping.find(nbttagcompound->getString(u"id"));
        if (it != stringToClassMapping.end())
        {
            entity = it->second(world);
        }
    }
    catch (std::exception &exception)
    {
        std::cerr << exception.what() << std::endl;
    }
    if (entity != nullptr)
    {
        entity->readFromNBT(nbttagcompound);
    }
    else
    {
        std::cout << "Skipping Entity with id " << String::toUTF8(nbttagcompound->getString(u"id")) << std::endl;
    }
    return entity;
}

Entity *EntityList::createEntity(int_t i, World *world)
{
    Entity *entity = nullptr;
    try
    {
        auto it = IDtoClassMapping.find(i);
        if (it != IDtoClassMapping.end())
        {
            entity = it->second(world);
        }
    }
    catch (std::exception &exception)
    {
        std::cerr << exception.what() << std::endl;
    }
    if (entity == nullptr)
    {
        std::cout << "Skipping Entity with id " << i << std::endl;
    }
    return entity;
}

int_t EntityList::getEntityID(Entity *entity)
{
    // Java HashMap.get() returns null for unmapped classes; mirror that with a sentinel
    // instead of std::map::at() (which throws "invalid map<K,T> key" and crashed the game
    // whenever an unregistered entity — e.g. the player — was processed).
    auto it = classToIDMapping.find(std::type_index(typeid(*entity)));
    return it != classToIDMapping.end() ? it->second : 0;
}

jstring EntityList::getEntityString(Entity *entity)
{
    // Java returns null here for unregistered entities; Entity::addEntityID checks for that
    // (s.empty()) and skips saving them. Return empty instead of throwing from .at().
    auto it = classToStringMapping.find(std::type_index(typeid(*entity)));
    return it != classToStringMapping.end() ? it->second : jstring();
}

static struct EntityListInit
{
    EntityListInit()
    {
        EntityList::addMapping<EntityArrow>(u"Arrow", 10);
        EntityList::addMapping<EntitySnowball>(u"Snowball", 11);
        EntityList::addMapping<EntityItem>(u"Item", 1);
        EntityList::addMapping<EntityPainting>(u"Painting", 9);
        EntityList::addMapping<EntityLiving>(u"Mob", 48);
        EntityList::addMapping<EntityMob>(u"Monster", 49);
        EntityList::addMapping<EntityCreeper>(u"Creeper", 50);
        EntityList::addMapping<EntitySkeleton>(u"Skeleton", 51);
        EntityList::addMapping<EntitySpider>(u"Spider", 52);
        EntityList::addMapping<EntityGiantZombie>(u"Giant", 53);
        EntityList::addMapping<EntityZombie>(u"Zombie", 54);
        EntityList::addMapping<EntitySlime>(u"Slime", 55);
        EntityList::addMapping<EntityGhast>(u"Ghast", 56);
        EntityList::addMapping<EntityPigZombie>(u"PigZombie", 57);
        EntityList::addMapping<EntityPig>(u"Pig", 90);
        EntityList::addMapping<EntitySheep>(u"Sheep", 91);
        EntityList::addMapping<EntityCow>(u"Cow", 92);
        EntityList::addMapping<EntityChicken>(u"Chicken", 93);
        EntityList::addMapping<EntitySquid>(u"Squid", 94);
        EntityList::addMapping<EntityWolf>(u"Wolf", 95);
        EntityList::addMapping<EntityTNTPrimed>(u"PrimedTnt", 20);
        EntityList::addMapping<EntityFallingSand>(u"FallingSand", 21);
        EntityList::addMapping<EntityMinecart>(u"Minecart", 40);
        EntityList::addMapping<EntityBoat>(u"Boat", 41);

        // Register EntityLiving subclasses with SpawnerAnimals so performSpawning can instantiate them.
        // Java used reflection (Class.getConstructor().newInstance()); here we register lambdas.
#define SPAWN_REG(T) SpawnerAnimals_registerFactory(std::type_index(typeid(T)), [](World *w) -> EntityLiving * { return new T(w); })
        SPAWN_REG(EntityCreeper);
        SPAWN_REG(EntitySkeleton);
        SPAWN_REG(EntitySpider);
        SPAWN_REG(EntityGiantZombie);
        SPAWN_REG(EntityZombie);
        SPAWN_REG(EntitySlime);
        SPAWN_REG(EntityGhast);
        SPAWN_REG(EntityPigZombie);
        SPAWN_REG(EntityPig);
        SPAWN_REG(EntitySheep);
        SPAWN_REG(EntityCow);
        SPAWN_REG(EntityChicken);
        SPAWN_REG(EntitySquid);
        SPAWN_REG(EntityWolf);
        SPAWN_REG(EntityMob);
        SPAWN_REG(EntityLiving);
#undef SPAWN_REG
    }
} entityListInit;
