#pragma once

#include "ItemTool.h"

class ItemPickaxe : public ItemTool {
public:
    ItemPickaxe(int id, EnumToolMaterial material);

    bool canHarvestBlock(Block* block) override;

private:
    static Block* blocksEffectiveAgainst[];
    static int numBlocksEffectiveAgainst;
    static bool initialized;
};
