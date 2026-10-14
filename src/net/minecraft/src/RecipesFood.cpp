#include "RecipesFood.h"

#include "Block.h"
#include "BlockMushroom.h"
#include "CraftingManager.h"
#include "Item.h"
#include "ItemStack.h"

RecipesFood::RecipesFood()
{
}

void RecipesFood::addRecipes(CraftingManager *craftingmanager)
{
	craftingmanager->addRecipe(new ItemStack(Item::bowlSoup),
	                           { "Y", "X", "#", 'X', Block::mushroomBrown, 'Y', Block::mushroomRed, '#', Item::bowlEmpty });
	craftingmanager->addRecipe(new ItemStack(Item::bowlSoup),
	                           { "Y", "X", "#", 'X', Block::mushroomRed, 'Y', Block::mushroomBrown, '#', Item::bowlEmpty });
	craftingmanager->addRecipe(new ItemStack(Item::cookie, 8),
	                           { "#X#", 'X', new ItemStack(Item::dyePowder, 1, 3), '#', Item::wheat });
}
