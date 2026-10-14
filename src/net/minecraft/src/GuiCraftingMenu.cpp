#include "GuiCraftingMenu.h"

#include <algorithm>
#include <cstdlib>

#include "CraftingManager.h"
#include "IRecipe.h"
#include "ShapedRecipes.h"
#include "ShapelessRecipes.h"
#include "ItemStack.h"
#include "Item.h"
#include "Block.h"
#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "GuiButton.h"
#include "RenderItem.h"
#include "RenderEngine.h"
#include "FontRenderer.h"
#include "Minecraft.h"
#include "StringTranslate.h"
#include "java/System.h"
#include "pc/OpenGL.h"

namespace
{
constexpr int_t kTabH = 18;
constexpr int_t kBottomH = 20;
// The iOS overlay keeps KB / CHAT / MENU as UIKit subviews in the top-right
// corner even while a game screen is open, so the top strip must stay clear.
constexpr int_t kTopReserve = 30;

void collectIngredients(IRecipe *r, std::vector<ItemStack *> &out)
{
	if (auto *sr = dynamic_cast<ShapedRecipes *>(r))
	{
		ItemStack **items = sr->getItems();
		int_t n = sr->getWidth() * sr->getHeight();
		for (int_t i = 0; i < n; i++)
			if (items[i] != nullptr)
				out.push_back(items[i]);
	}
	else if (auto *ul = dynamic_cast<ShapelessRecipes *>(r))
	{
		for (ItemStack *s : ul->getItems())
			if (s != nullptr)
				out.push_back(s);
	}
}

// Block/wood recipe ingredients are registered as `new ItemStack(block, 1, -1)`
// where damage -1 means "any variant". A plain isItemEqual() comparison treats
// -1 as a literal damage value, so logs/planks never matched their inputs and
// the whole craftable filter appeared broken.
bool stackMatches(ItemStack *have, ItemStack *need)
{
	if (have == nullptr || need == nullptr)
		return false;
	if (have->itemID != need->itemID)
		return false;
	if (need->getItemDamage() >= 0 && have->getItemDamage() != need->getItemDamage())
		return false;
	return true;
}
} // namespace

static RenderItem g_craftItemRenderer;

GuiCraftingMenu::GuiCraftingMenu(GuiScreen *parent, EntityPlayer *player, bool allow3x3In)
	: parentScreen(parent)
	, thePlayer(player)
	, allow3x3(allow3x3In)
	, tab(0)
	, craftableOnly(false)
	, scroll(0.0f)
	, flashRow(-1)
	, flashUntilMs(0)
{
	field_948_f = true;
	rebuildBaseList();
	rebuildView();
}

void GuiCraftingMenu::rebuildBaseList()
{
	baseRecipes.clear();
	const std::vector<IRecipe *> &all = CraftingManager::getInstance()->getRecipeList();
	for (IRecipe *r : all)
	{
		if (r == nullptr || r->getRecipeOutput() == nullptr)
			continue;
		// A 2x2 grid only fits shaped recipes up to 2x2 and shapeless up to 4
		// items. The old check used total size (e.g. a 3x1 slab recipe has size 3
		// and wrongly passed), so the inventory menu could craft slabs.
		if (!allow3x3)
		{
			if (auto *sr = dynamic_cast<ShapedRecipes *>(r))
			{
				if (sr->getWidth() > 2 || sr->getHeight() > 2)
					continue;
			}
			else if (auto *ul = dynamic_cast<ShapelessRecipes *>(r))
			{
				if ((int_t)ul->getItems().size() > 4)
					continue;
			}
			else if (r->getRecipeSize() > 4)
				continue;
		}
		baseRecipes.push_back(r);
	}
	std::sort(baseRecipes.begin(), baseRecipes.end(), [](IRecipe *a, IRecipe *b) {
		ItemStack *x = a->getRecipeOutput();
		ItemStack *y = b->getRecipeOutput();
		if (x->itemID != y->itemID)
			return x->itemID < y->itemID;
		return x->getItemDamage() < y->getItemDamage();
	});
}

int_t GuiCraftingMenu::categoryOf(IRecipe *recipe) const
{
	ItemStack *out = recipe->getRecipeOutput();
	if (out == nullptr)
		return 1;
	if (out->itemID < 256)
		return 0; // block
	Item *item = Item::itemsList[out->itemID];
	if (item != nullptr && item->getHasSubtypes())
		return 2; // misc (wool, dyes, coal, ...)
	return 1;     // item
}

void GuiCraftingMenu::rebuildView()
{
	view.clear();
	for (IRecipe *r : baseRecipes)
	{
		if (tab != 0 && categoryOf(r) != (tab - 1))
			continue;
		if (craftableOnly && !ingredientsAvailable(r))
			continue;
		view.push_back(r);
	}
	// craftable first so the useful ones are always in reach
	std::stable_sort(view.begin(), view.end(), [this](IRecipe *a, IRecipe *b) {
		bool ca = ingredientsAvailable(a);
		bool cb = ingredientsAvailable(b);
		return ca && !cb;
	});
	// NOTE: scroll is intentionally NOT reset here; callers that change the
	// tab/filter reset it explicitly. Resetting on every rebuild made the list
	// snap back to the top.
}

int_t GuiCraftingMenu::rowHeight() const
{
	int_t h = height / 7;
	if (h < 28)
		h = 28;
	if (h > 48)
		h = 48;
	return h;
}

int_t GuiCraftingMenu::listTop() const
{
	return kTopReserve + kTabH + 4;
}

int_t GuiCraftingMenu::listBottom() const
{
	return height - kBottomH - 8;
}

void GuiCraftingMenu::initGui()
{
	controlList.clear();

	int_t tabW = (width - 8) / 4;
	static const char *tabNames[4] = {"All", "Blocks", "Items", "Misc"};
	for (int_t i = 0; i < 4; i++)
	{
		GuiButton *tb = new GuiButton(10 + i, 4 + i * tabW, kTopReserve, tabW - 2, kTabH, tabNames[i]);
		tb->enabled = (tab != i); // active tab shown greyed/selected
		controlList.push_back(tb);
	}

	int_t by = height - kBottomH - 4;
	controlList.push_back(new GuiButton(1, 6, by, 40, kBottomH, "Up"));
	controlList.push_back(new GuiButton(2, 50, by, 40, kBottomH, "Down"));
	controlList.push_back(new GuiButton(20, 96, by, 120, kBottomH,
	                                    craftableOnly ? "Craftable: ON" : "Craftable: OFF"));
	controlList.push_back(new GuiButton(0, width - 76, by, 70, kBottomH, "Back"));

	// highlight the active tab
	for (int_t i = 0; i < 4; i++)
		controlList[i]->enabled = (tab != i);
}

bool GuiCraftingMenu::ingredientsAvailable(IRecipe *recipe)
{
	std::vector<ItemStack *> need;
	collectIngredients(recipe, need);
	InventoryPlayer *inv = thePlayer->inventory;

	std::vector<int_t> used(inv->getSizeInventory(), 0);
	for (size_t i = 0; i < need.size(); i++)
	{
		bool ok = false;
		for (int_t s = 0; s < inv->getSizeInventory(); s++)
		{
			ItemStack *st = inv->getStackInSlot(s);
			if (st == nullptr || st->stackSize - used[s] <= 0)
				continue;
			if (!stackMatches(st, need[i]))
				continue;
			used[s]++;
			ok = true;
			break;
		}
		if (!ok)
			return false;
	}
	return true;
}

bool GuiCraftingMenu::consumeIngredients(IRecipe *recipe)
{
	std::vector<ItemStack *> need;
	collectIngredients(recipe, need);
	InventoryPlayer *inv = thePlayer->inventory;

	std::vector<int_t> used(inv->getSizeInventory(), 0);
	for (size_t i = 0; i < need.size(); i++)
	{
		bool ok = false;
		for (int_t s = 0; s < inv->getSizeInventory(); s++)
		{
			ItemStack *st = inv->getStackInSlot(s);
			if (st == nullptr || st->stackSize - used[s] <= 0)
				continue;
			if (!stackMatches(st, need[i]))
				continue;
			used[s]++;
			ok = true;
			break;
		}
		if (!ok)
			return false;
	}
	for (int_t s = 0; s < inv->getSizeInventory(); s++)
		if (used[s] > 0)
			inv->decrStackSize(s, used[s]);
	return true;
}

bool GuiCraftingMenu::craftRecipe(IRecipe *recipe)
{
	if (!ingredientsAvailable(recipe))
		return false;
	if (!consumeIngredients(recipe))
		return false;

	ItemStack *out = recipe->getRecipeOutput();
	if (out != nullptr)
		thePlayer->inventory->addItemStackToInventory(out->copy());
	return true;
}

void GuiCraftingMenu::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawDefaultBackground();

	int_t top = listTop();
	int_t bottom = listBottom();
	int_t rh = rowHeight();
	int_t totalH = (int_t)view.size() * rh;
	float_t maxScroll = (float_t)std::max(0, totalH - (bottom - top));
	if (scroll < 0.0f)
		scroll = 0.0f;
	if (scroll > maxScroll)
		scroll = maxScroll;

	long_t now = System::currentTimeMillis();

	for (size_t i = 0; i < view.size(); i++)
	{
		int_t y = top + (int_t)i * rh - (int_t)scroll;
		if (y + rh <= top || y >= bottom)
			continue;

		int_t drawY = y;
		int_t drawH = rh;
		if (drawY < top)
		{
			drawH -= (top - drawY);
			drawY = top;
		}
		if (drawY + drawH > bottom)
			drawH = bottom - drawY;
		if (drawH <= 2)
			continue;

		IRecipe *recipe = view[i];
		bool can = ingredientsAvailable(recipe);
		bool flash = (flashRow == (int_t)i && now < flashUntilMs);

		if (flash)
			drawGradientRect(2, drawY, width - 2, drawY + drawH, 0xff50d050, 0xff209020);
		else if (can)
			drawGradientRect(2, drawY, width - 2, drawY + drawH, 0xff3a3a3a, 0xff222222);
		else
			drawGradientRect(2, drawY, width - 2, drawY + drawH, 0xff191919, 0xff111111);

		ItemStack *out = recipe->getRecipeOutput();

		// output icon (flat 2D sprite)
		int_t iconSz = rh - 8;
		if (iconSz > 40)
			iconSz = 40;
		if (iconSz < 16)
			iconSz = 16;
		int_t iconY = drawY + (drawH - iconSz) / 2;
		g_craftItemRenderer.renderItemIcon2D(mc->renderEngine, nullptr, out, 6, iconY,
		                                     (float_t)iconSz / 16.0f);

		// name
		std::string itemName = out->getItemName();
		StringTranslate *tr = StringTranslate::getInstance();
		std::string pretty = tr->translateNamedKey(itemName);
		if (pretty.empty())
			pretty = tr->translateKey(itemName);

		glDisable(GL_LIGHTING);
		int_t textX = 6 + iconSz + 6;
		int_t textY = drawY + (drawH - 8) / 2;
		fontRenderer->drawStringWithShadow(pretty, textX, textY, can ? 0xffffff : 0x909090);

		// ingredient sprites, right-aligned (this is what fills the row)
		std::vector<ItemStack *> need;
		collectIngredients(recipe, need);
		int_t ingSz = 12;
		int_t stateW = 34;
		int_t ingX = width - stateW - (int_t)need.size() * (ingSz + 2) - 6;
		int_t ingY = drawY + (drawH - ingSz) / 2;
		for (size_t k = 0; k < need.size(); k++)
			g_craftItemRenderer.renderItemIcon2D(mc->renderEngine, nullptr, need[k],
			                                     ingX + (int_t)k * (ingSz + 2), ingY,
			                                     (float_t)ingSz / 16.0f);

		// state
		glDisable(GL_LIGHTING);
		if (can)
			fontRenderer->drawStringWithShadow("Craft", width - stateW, textY, 0x60ff60);
		else
			fontRenderer->drawStringWithShadow("missing", width - stateW - 8, textY, 0xff8080);
	}

	// scrollbar
	if (maxScroll > 0.0f)
	{
		float_t frac = (float_t)(bottom - top) / (float_t)totalH;
		int_t barH = std::max(12, (int_t)((bottom - top) * frac));
		int_t barY = top + (int_t)((bottom - top - barH) * (scroll / maxScroll));
		drawGradientRect(width - 4, barY, width - 2, barY + barH, 0x80ffffff, 0x80ffffff);
	}

	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}

void GuiCraftingMenu::mouseClicked(int_t x, int_t y, int_t button)
{
	GuiScreen::mouseClicked(x, y, button);

	// Craft on press: a single, immediate action per tap. No drag handling, no
	// press-and-hold repeat — that ambiguity confused players.
	if (button != 0)
		return;

	int_t top = listTop();
	int_t bottom = listBottom();
	if (y < top || y > bottom)
		return;

	int_t rh = rowHeight();
	int_t row = (int_t)((y - top + (int_t)scroll) / rh);
	if (row >= 0 && row < (int_t)view.size() && craftRecipe(view[row]))
	{
		flashRow = row;
		flashUntilMs = System::currentTimeMillis() + 250;
	}
}

void GuiCraftingMenu::keyTyped(char_t c, int_t key)
{
	if (key == 1)
	{
		mc->displayGuiScreen(parentScreen);
		return;
	}
	GuiScreen::keyTyped(c, key);
}

void GuiCraftingMenu::actionPerformed(GuiButton *button)
{
	if (button == nullptr)
		return;

	int_t top = listTop();
	int_t bottom = listBottom();
	int_t rh = rowHeight();
	int_t page = std::max(rh, (bottom - top) - rh);

	if (button->id == 0)
	{
		mc->displayGuiScreen(parentScreen);
	}
	else if (button->id == 1)
	{
		scroll -= (float_t)page;
		if (scroll < 0.0f)
			scroll = 0.0f;
	}
	else if (button->id == 2)
	{
		int_t totalH = (int_t)view.size() * rh;
		float_t maxScroll = (float_t)std::max(0, totalH - (bottom - top));
		scroll += (float_t)page;
		if (scroll > maxScroll)
			scroll = maxScroll;
	}
	else if (button->id >= 10 && button->id <= 13)
	{
		if (tab != button->id - 10)
		{
			tab = button->id - 10;
			rebuildView();
			scroll = 0.0f;
			initGui();
		}
	}
	else if (button->id == 20)
	{
		craftableOnly = !craftableOnly;
		rebuildView();
		scroll = 0.0f;
		initGui();
	}
}
