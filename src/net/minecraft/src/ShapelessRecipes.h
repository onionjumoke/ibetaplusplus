#pragma once

#include "IRecipe.h"
#include <vector>

class ItemStack;

// net.minecraft.src.ShapelessRecipes
class ShapelessRecipes : public IRecipe
{
public:
    ShapelessRecipes(ItemStack *output, std::vector<ItemStack*> items);

    ItemStack* getRecipeOutput() override;

    // Touch crafting UI: expose the ingredient list.
    const std::vector<ItemStack*> &getItems() const { return recipeItems; }
    bool matches(InventoryCrafting *inventorycrafting) override;
    ItemStack* getCraftingResult(InventoryCrafting *inventorycrafting) override;
    int_t getRecipeSize() override;

private:
    ItemStack *recipeOutput;
    std::vector<ItemStack*> recipeItems;
};
