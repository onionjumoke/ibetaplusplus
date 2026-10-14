#pragma once

#include <set>
#include <vector>

#include "java/Type.h"

class EntityPlayer;
class ICrafting;
class IInventory;
class InventoryPlayer;
class ItemStack;
class Slot;

// net.minecraft.src.Container
class Container
{
public:
	Container();
	virtual ~Container();

protected:
	void addSlot(Slot *slot);

public:
	virtual void updateCraftingResults();
	Slot *getSlot(int_t i);
	virtual ItemStack *getStackInSlot(int_t i);

	// Java: func_27280_a — handles a click on a slot (left/right + shift-click).
	ItemStack *slotClick(int_t slotIndex, int_t mouseButton, bool shiftClick, EntityPlayer *entityplayer);

	virtual void onCraftGuiClosed(EntityPlayer *entityplayer);
	virtual void onCraftMatrixChanged(IInventory *iinventory);
	void putStackInSlot(int_t i, ItemStack *itemstack);
	void putStacksInSlots(ItemStack **aitemstack, int_t length);

	// Java: func_20112_a — pushes a progress-bar value to the open container.
	virtual void updateProgressBar(int_t id, int_t value);

	// Java: func_20111_a — unique transaction id for InventoryPlayer actions.
	short_t getNextTransactionID(InventoryPlayer *inventoryplayer);

	// Java: func_20113_a / func_20110_b — transaction confirm/response (server side).
	virtual void updateTransaction(short_t transactionID);
	virtual void onTransactionResponse(short_t transactionID);

	virtual bool isUsableByPlayer(EntityPlayer *entityplayer) = 0;

protected:
	// Java: func_28125_a — merge itemstack into slots in [startSlot, endSlot).
	void mergeItemStack(ItemStack *itemstack, int_t startSlot, int_t endSlot, bool lastToFirst);

public:
	// Java field_20123_d — cached copies of slot contents for change detection.
	std::vector<ItemStack *> inventoryItemStacks;

	std::vector<Slot *> slots;
	int_t windowId = 0;

private:
	// Java field_20917_a — counter for generating unique transaction IDs.
	short_t transactionID = 0;

protected:
	// Java field_20121_g — list of ICrafting listeners (for GUI updates).
	std::vector<ICrafting *> crafters;

private:
	// Java field_20918_b — set of players who have this container open.
	std::set<int_t> playerList;
};
