#pragma once

#include "GuiContainer.h"

class EntityPlayer;

// net.minecraft.src.GuiTouchInventory — finger-sized re-layout of the survival
// inventory: 32x32 slots on a 400x300 panel, large item icons and on-screen
// action buttons instead of hidden desktop gestures.
class GuiTouchInventory : public GuiContainer
{
public:
	explicit GuiTouchInventory(EntityPlayer *player);

	void initGui() override;

protected:
	void drawGuiContainerForegroundLayer() override;
	void drawGuiContainerBackgroundLayer(float_t partialTick) override;
	void drawSlotInventory(Slot *slot) override;
	void actionPerformed(GuiButton *button) override;

private:
	int_t lastSlotX;
	int_t lastSlotY;
};
