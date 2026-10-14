#pragma once

#include "Item.h"

class ItemArmor : public Item {
public:
    ItemArmor(int id, int armorLevel, int renderIndex, int armorType);

    static const int damageReduceAmountArray[4];
    static const int maxDamageArray[4];

    int armorLevel;
    int armorType;
    int damageReduceAmount;
    int renderIndex;
};
