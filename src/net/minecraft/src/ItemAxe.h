#pragma once

#include "ItemTool.h"

class ItemAxe : public ItemTool {
public:
    ItemAxe(int id, EnumToolMaterial material);

private:
    static Block* blocksEffectiveAgainst[];
    static int numBlocksEffectiveAgainst;
    static bool initialized;
};
