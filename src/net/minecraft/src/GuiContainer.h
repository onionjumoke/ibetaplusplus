#pragma once

#include "GuiScreen.h"

class Container;
class Slot;
class RenderItem;
class InventoryPlayer;

// net.minecraft.src.GuiContainer
class GuiContainer : public GuiScreen
{
public:
	GuiContainer(Container *container);
	virtual ~GuiContainer() = default;

	void initGui() override;
	void drawScreen(int_t mouseX, int_t mouseY, float_t partialTick) override;

protected:
	virtual void drawGuiContainerForegroundLayer();
	virtual void drawGuiContainerBackgroundLayer(float_t partialTick) = 0;

protected:
	// Virtual so touch layouts can scale the icon inside larger slots.
	virtual void drawSlotInventory(Slot *slot);

private:
	Slot *getSlotAtPosition(int_t mouseX, int_t mouseY);
	bool getIsMouseOverSlot(Slot *slot, int_t mouseX, int_t mouseY);

protected:
	void mouseClicked(int_t x, int_t y, int_t button) override;
	void mouseMovedOrUp(int_t x, int_t y, int_t button) override;
	void keyTyped(char_t c, int_t key) override;

public:
	void onGuiClosed() override;
	bool doesGuiPauseGame() override;
	void updateScreen() override;

private:
	static RenderItem *itemRenderer;

protected:
	int_t xSize;
	int_t ySize;

public:
	Container *inventorySlots;
};
