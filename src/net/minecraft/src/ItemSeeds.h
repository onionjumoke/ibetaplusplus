#pragma once

#include "Item.h"

class ItemSeeds : public Item {
public:
    ItemSeeds(int id, int cropBlockId);

    bool onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) override;

private:
    int field_318_a;
};
