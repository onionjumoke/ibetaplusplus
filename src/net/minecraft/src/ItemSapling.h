#pragma once

#include "ItemBlock.h"

class ItemSapling : public ItemBlock {
public:
    ItemSapling(int id);

    int getPlacedBlockMetadata(int damage) override;
    int getIconFromDamage(int damage) override;
};
