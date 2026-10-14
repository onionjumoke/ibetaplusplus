#pragma once

#include "Item.h"

class ItemBow : public Item {
public:
    ItemBow(int id);

    ItemStack* onItemRightClick(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) override;
};
