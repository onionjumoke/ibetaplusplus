#pragma once

#include "ItemBlock.h"

class ItemSlab : public ItemBlock {
public:
    ItemSlab(int id);

    int getIconFromDamage(int damage) override;
    int getPlacedBlockMetadata(int damage) override;
    std::string getItemNameIS(ItemStack* itemstack) override;
};
