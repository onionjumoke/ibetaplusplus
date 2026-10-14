#pragma once

#include <map>
#include "java/Type.h"

class ItemStack;

// net.minecraft.src.FurnaceRecipes
class FurnaceRecipes
{
private:
    FurnaceRecipes();

public:
    static FurnaceRecipes* smelting();

    void addSmelting(int_t itemId, ItemStack *output);
    ItemStack* getSmeltingResult(int_t itemId);
    std::map<int_t, ItemStack*>& getSmeltingList();

private:
    static FurnaceRecipes *smeltingBase;
    std::map<int_t, ItemStack*> smeltingList;
};
