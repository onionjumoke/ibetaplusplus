#pragma once

#include "ItemBlock.h"

class ItemCloth : public ItemBlock {
public:
    ItemCloth(int id);

    int getIconFromDamage(int damage) override;
    int getPlacedBlockMetadata(int damage) override;
    std::string getItemNameIS(ItemStack* itemstack) override;
};
