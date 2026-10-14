#include "GuiContainer.h"
#include "EntityPlayerSP.h"
#include "Container.h"
#include "Slot.h"
#include "RenderItem.h"
#include "RenderHelper.h"
#include "InventoryPlayer.h"
#include "StringTranslate.h"
#include "ItemStack.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "Minecraft.h"
#include "GameSettings.h"
#include "KeyBinding.h"
#include "PlayerController.h"
#include "pc/OpenGL.h"
#include "pc/lwjgl/Keyboard.h"

RenderItem *GuiContainer::itemRenderer = new RenderItem();

GuiContainer::GuiContainer(Container *container)
	: xSize(176)
	, ySize(166)
	, inventorySlots(container)
{
}

void GuiContainer::initGui()
{
	GuiScreen::initGui();
	mc->thePlayer->craftingInventory = inventorySlots;
}

void GuiContainer::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawDefaultBackground();
	int_t guiX = (width - xSize) / 2;
	int_t guiY = (height - ySize) / 2;
	drawGuiContainerBackgroundLayer(partialTick);

	glPushMatrix();
	glRotatef(120.0f, 1.0f, 0.0f, 0.0f);
	RenderHelper::enableStandardItemLighting();
	glPopMatrix();

	glPushMatrix();
	glTranslatef((float_t)guiX, (float_t)guiY, 0.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_RESCALE_NORMAL);

	Slot *hoveredSlot = nullptr;
	for (int_t i = 0; i < (int_t)inventorySlots->slots.size(); i++)
	{
		Slot *slot = inventorySlots->slots[i];
		drawSlotInventory(slot);
		if (getIsMouseOverSlot(slot, mouseX, mouseY))
		{
			hoveredSlot = slot;
			glDisable(GL_LIGHTING);
			glDisable(GL_DEPTH_TEST);
			int_t sx = slot->xDisplayPosition;
			int_t sy = slot->yDisplayPosition;
			drawGradientRect(sx, sy, sx + 16, sy + 16, 0x80ffffff, 0x80ffffff);
			glEnable(GL_LIGHTING);
			glEnable(GL_DEPTH_TEST);
		}
	}

	InventoryPlayer *inv = mc->thePlayer->inventory;
	if (inv->getItemStack() != nullptr)
	{
		glTranslatef(0.0f, 0.0f, 32.0f);
		itemRenderer->renderItemIntoGUI(fontRenderer, mc->renderEngine, inv->getItemStack(),
		                                mouseX - guiX - 8, mouseY - guiY - 8);
		itemRenderer->renderItemOverlayIntoGUI(fontRenderer, mc->renderEngine, inv->getItemStack(),
		                                       mouseX - guiX - 8, mouseY - guiY - 8);
	}

	glDisable(GL_RESCALE_NORMAL);
	RenderHelper::disableStandardItemLighting();
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	drawGuiContainerForegroundLayer();

	if (inv->getItemStack() == nullptr && hoveredSlot != nullptr && hoveredSlot->getHasStack())
	{
		std::string name = StringTranslate::getInstance()->translateNamedKey(hoveredSlot->getStack()->getItemName());
		// trim
		while (!name.empty() && name.front() == ' ') name.erase(name.begin());
		while (!name.empty() && name.back() == ' ')  name.pop_back();
		if (!name.empty())
		{
			int_t tx = (mouseX - guiX) + 12;
			int_t ty = mouseY - guiY - 12;
			int_t tw = fontRenderer->getStringWidth(name);
			drawGradientRect(tx - 3, ty - 3, tx + tw + 3, ty + 8 + 3, 0xc0000000, 0xc0000000);
			fontRenderer->drawStringWithShadow(name, tx, ty, -1);
		}
	}

	glPopMatrix();
	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
}

void GuiContainer::drawGuiContainerForegroundLayer()
{
}

void GuiContainer::drawSlotInventory(Slot *slot)
{
	int_t x = slot->xDisplayPosition;
	int_t y = slot->yDisplayPosition;
	ItemStack *stack = slot->getStack();
	if (stack == nullptr)
	{
		int_t icon = slot->getBackgroundIconIndex();
		if (icon >= 0)
		{
			glDisable(GL_LIGHTING);
			mc->renderEngine->bindTexture(mc->renderEngine->getTexture("/gui/items.png"));
			drawTexturedModalRect(x, y, (icon % 16) * 16, (icon / 16) * 16, 16, 16);
			glEnable(GL_LIGHTING);
			return;
		}
	}
	itemRenderer->renderItemIntoGUI(fontRenderer, mc->renderEngine, stack, x, y);
	itemRenderer->renderItemOverlayIntoGUI(fontRenderer, mc->renderEngine, stack, x, y);
}

Slot *GuiContainer::getSlotAtPosition(int_t mouseX, int_t mouseY)
{
	// Touch-friendly: return the NEAREST slot within a forgiving radius
	// instead of the first exact hit. Vanilla cells are 18x18; the centre is
	// at most ~12.7 px from a corner, so 13 px accepts the whole cell plus a
	// little slack for a fat finger. Nearest-wins also beats first-match when
	// two expanded bounds would overlap.
	int_t guiX = (width - xSize) / 2;
	int_t guiY = (height - ySize) / 2;
	int_t rx = mouseX - guiX;
	int_t ry = mouseY - guiY;

	Slot *best = nullptr;
	int_t bestD = 0x7fffffff;
	for (size_t i = 0; i < inventorySlots->slots.size(); i++)
	{
		Slot *slot = inventorySlots->slots[i];
		int_t dx = rx - (slot->xDisplayPosition + 8);
		int_t dy = ry - (slot->yDisplayPosition + 8);
		int_t d = dx * dx + dy * dy;
		if (d < bestD)
		{
			bestD = d;
			best = slot;
		}
	}
	if (best != nullptr && bestD <= 13 * 13)
		return best;
	return nullptr;
}

bool GuiContainer::getIsMouseOverSlot(Slot *slot, int_t mouseX, int_t mouseY)
{
	int_t guiX = (width - xSize) / 2;
	int_t guiY = (height - ySize) / 2;
	int_t rx = mouseX - guiX;
	int_t ry = mouseY - guiY;
	return rx >= slot->xDisplayPosition - 1 && rx < slot->xDisplayPosition + 16 + 1
	    && ry >= slot->yDisplayPosition - 1 && ry < slot->yDisplayPosition + 16 + 1;
}

void GuiContainer::mouseClicked(int_t x, int_t y, int_t button)
{
	GuiScreen::mouseClicked(x, y, button);
	if (button == 0 || button == 1)
	{
		Slot *slot = getSlotAtPosition(x, y);
		int_t guiX = (width - xSize) / 2;
		int_t guiY = (height - ySize) / 2;
		bool outsideGui = x < guiX || y < guiY || x >= guiX + xSize || y >= guiY + ySize;
		int_t slotId = -1;
		if (slot != nullptr) slotId = slot->slotNumber;
		if (outsideGui)      slotId = -999;
		if (slotId != -1)
		{
			bool shift = slotId != -999 && (lwjgl::Keyboard::isKeyDown(42) || lwjgl::Keyboard::isKeyDown(54));
			mc->playerController->windowClick(inventorySlots->windowId, slotId, button, shift, mc->thePlayer);
		}
	}
}

void GuiContainer::mouseMovedOrUp(int_t x, int_t y, int_t button)
{
	// no-op when button != 0
}

void GuiContainer::keyTyped(char_t c, int_t key)
{
	if (key == 1 || key == mc->gameSettings->keyBindInventory->keyCode)
	{
		mc->thePlayer->closeScreen();
	}
}

void GuiContainer::onGuiClosed()
{
	if (mc->thePlayer == nullptr) return;
	mc->playerController->onContainerClosed(inventorySlots->windowId, mc->thePlayer);
}

bool GuiContainer::doesGuiPauseGame()
{
	return false;
}

void GuiContainer::updateScreen()
{
	GuiScreen::updateScreen();
	if (!mc->thePlayer->isEntityAlive() || mc->thePlayer->isDead)
	{
		mc->thePlayer->closeScreen();
	}
}
