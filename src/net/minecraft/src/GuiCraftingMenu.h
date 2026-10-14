#pragma once

#include "GuiScreen.h"

#include <vector>

class EntityPlayer;
class IRecipe;

// net.minecraft.src.GuiCraftingMenu — MCPE-style touch crafting:
//   * scrollable recipe list (Up/Down buttons page the list)
//   * tap a row to craft immediately (on press, no drag/press-and-hold)
//   * category tabs: All / Blocks / Items / Misc
//   * optional "craftable only" filter
//   * flat 2D item/block sprite icons (never the 3D block model)
//   * auto-consumes the ingredients from the player's inventory
class GuiCraftingMenu : public GuiScreen
{
public:
	GuiCraftingMenu(GuiScreen *parent, EntityPlayer *player, bool allow3x3);

	void initGui() override;
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
	void mouseClicked(int_t x, int_t y, int_t button) override;
	void keyTyped(char_t c, int_t key) override;
	void actionPerformed(GuiButton *button) override;

private:
	void rebuildBaseList();
	int_t categoryOf(IRecipe *recipe) const; // 0 blocks, 1 items, 2 misc
	void rebuildView();
	bool ingredientsAvailable(IRecipe *recipe);
	bool consumeIngredients(IRecipe *recipe);
	bool craftRecipe(IRecipe *recipe);
	int_t rowHeight() const;
	int_t listTop() const;
	int_t listBottom() const;

	GuiScreen *parentScreen;
	EntityPlayer *thePlayer;
	bool allow3x3;

	std::vector<IRecipe *> baseRecipes;
	std::vector<IRecipe *> view;        // filtered by tab + craftable-only

	int_t tab;                          // 0 all, 1 blocks, 2 items, 3 misc
	bool craftableOnly;

	float_t scroll;
	int_t flashRow;
	long_t flashUntilMs;
};
