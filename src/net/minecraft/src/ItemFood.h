#pragma once

#include "Item.h"

class ItemFood : public Item {
public:
    ItemFood(int id, int healAmount, bool isWolfsFavoriteMeat);

    ItemStack* onItemRightClick(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) override;
    int getHealAmount();
    bool getIsWolfsFavoriteMeat();

protected:
    int healAmount;
    bool isWolfsFavoriteMeat;
};
