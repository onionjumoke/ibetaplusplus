#pragma once

#include "Item.h"
#include <string>

class ItemRecord : public Item {
public:
    ItemRecord(int id, const std::string& recordName);

    bool onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) override;

    const std::string recordName;
};
