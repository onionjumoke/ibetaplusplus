#include "RecipeSorter.h"

#include "IRecipe.h"
#include <string>
#include <typeinfo>

static bool isRecipeType(IRecipe *recipe, const char *typeName)
{
	return recipe != nullptr && std::string(typeid(*recipe).name()).find(typeName) != std::string::npos;
}

RecipeSorter::RecipeSorter(CraftingManager *craftingmanager) :
	craftingManager(craftingmanager)
{
}

int_t RecipeSorter::compareRecipes(IRecipe *irecipe, IRecipe *irecipe1)
{
	if (isRecipeType(irecipe, "ShapelessRecipes") && isRecipeType(irecipe1, "ShapedRecipes"))
	{
		return 1;
	}
	if (isRecipeType(irecipe1, "ShapelessRecipes") && isRecipeType(irecipe, "ShapedRecipes"))
	{
		return -1;
	}
	if (irecipe1->getRecipeSize() < irecipe->getRecipeSize())
	{
		return -1;
	}
	return irecipe1->getRecipeSize() <= irecipe->getRecipeSize() ? 0 : 1;
}

int_t RecipeSorter::compare(IRecipe *obj, IRecipe *obj1)
{
	return compareRecipes(obj, obj1);
}

bool RecipeSorter::operator()(IRecipe *irecipe, IRecipe *irecipe1)
{
	return compareRecipes(irecipe, irecipe1) < 0;
}
