#pragma once

#include "ItemBlock.h"

class ItemLeaves : public ItemBlock {
public:
    ItemLeaves(int id);

    int getPlacedBlockMetadata(int damage) override;
    int getIconFromDamage(int damage) override;
    int getColorFromDamage(int damage) override;
};
