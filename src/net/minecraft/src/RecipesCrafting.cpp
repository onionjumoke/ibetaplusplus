#include "RecipesCrafting.h"

#include "Block.h"
#include "CraftingManager.h"
#include "ItemStack.h"

RecipesCrafting::RecipesCrafting()
{
}

void RecipesCrafting::addRecipes(CraftingManager *craftingmanager)
{
	craftingmanager->addRecipe(new ItemStack(Block::chest),
	                           { "###", "# #", "###", '#', Block::planks });
	craftingmanager->addRecipe(new ItemStack(Block::stoneOvenIdle),
	                           { "###", "# #", "###", '#', Block::cobblestone });
	craftingmanager->addRecipe(new ItemStack(Block::workbench),
	                           { "##", "##", '#', Block::planks });
	craftingmanager->addRecipe(new ItemStack(Block::sandStone),
	                           { "##", "##", '#', Block::sand });
}
