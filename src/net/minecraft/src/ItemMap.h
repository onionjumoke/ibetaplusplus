#pragma once

#include "ItemMapBase.h"

class MapData;
class World;
class Entity;
class ItemStack;

class ItemMap : public ItemMapBase {
public:
    ItemMap(int id);

    static MapData* getMapData(int_t id, World* world);
    static MapData* getHorizon(short word0, World* world);
    MapData* getAverageGroundLevel(ItemStack* itemstack, World* world);
    void initDimension(World* world, Entity* entity, MapData* mapdata);
    void onUpdate(ItemStack* itemstack, World* world, Entity* entity, int i, bool flag) override;
    void onCreated(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) override;
};
