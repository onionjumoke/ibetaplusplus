#include "GuiDispenser.h"
#include "ContainerDispenser.h"
#include "InventoryPlayer.h"
#include "TileEntityDispenser.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "Minecraft.h"
#include "pc/OpenGL.h"

GuiDispenser::GuiDispenser(InventoryPlayer *player, TileEntityDispenser *dispenser)
	: GuiContainer(new ContainerDispenser(player, dispenser))
{
}

void GuiDispenser::drawGuiContainerForegroundLayer()
{
	fontRenderer->drawString("Dispenser", 60, 6,             0x404040);
	fontRenderer->drawString("Inventory",  8, (ySize - 96) + 2, 0x404040);
}

void GuiDispenser::drawGuiContainerBackgroundLayer(float_t partialTick)
{
	int_t tex = mc->renderEngine->getTexture("/gui/trap.png");
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	mc->renderEngine->bindTexture(tex);
	int_t guiX = (width  - xSize) / 2;
	int_t guiY = (height - ySize) / 2;
	drawTexturedModalRect(guiX, guiY, 0, 0, xSize, ySize);
}
