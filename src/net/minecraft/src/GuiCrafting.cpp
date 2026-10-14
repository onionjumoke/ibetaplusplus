#include "GuiCrafting.h"
#include "ContainerWorkbench.h"
#include "InventoryPlayer.h"
#include "World.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "Minecraft.h"
#include "EntityPlayerSP.h"
#include "pc/OpenGL.h"

GuiCrafting::GuiCrafting(InventoryPlayer *player, World *world, int_t x, int_t y, int_t z)
	: GuiContainer(new ContainerWorkbench(player, world, x, y, z))
{
}

void GuiCrafting::onGuiClosed()
{
	GuiContainer::onGuiClosed();
	inventorySlots->onCraftGuiClosed(mc->thePlayer);
}

void GuiCrafting::drawGuiContainerForegroundLayer()
{
	fontRenderer->drawString("Crafting",  28, 6,             0x404040);
	fontRenderer->drawString("Inventory",  8, (ySize - 96) + 2, 0x404040);
}

void GuiCrafting::drawGuiContainerBackgroundLayer(float_t partialTick)
{
	int_t tex = mc->renderEngine->getTexture("/gui/crafting.png");
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	mc->renderEngine->bindTexture(tex);
	int_t guiX = (width  - xSize) / 2;
	int_t guiY = (height - ySize) / 2;
	drawTexturedModalRect(guiX, guiY, 0, 0, xSize, ySize);
}
