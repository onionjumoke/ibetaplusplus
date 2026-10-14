#include "GuiInventory.h"
#include "EntityPlayer.h"
#include "EntityPlayerSP.h"
#include "AchievementList.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "RenderHelper.h"
#include "RenderManager.h"
#include "GuiAchievements.h"
#include "GuiStats.h"
#include "GuiCraftingMenu.h"
#include "GuiButton.h"
#include "Minecraft.h"
#include "pc/OpenGL.h"
#include <cmath>

#ifndef GL_RESCALE_NORMAL
#define GL_RESCALE_NORMAL 0x803A
#endif

GuiInventory::GuiInventory(EntityPlayer *player)
	: GuiContainer(player->inventorySlots)
	, xSize_lo(0.0f)
	, ySize_lo(0.0f)
{
	field_948_f = true;
	player->addStat(AchievementList::openInventory, 1);
}

void GuiInventory::initGui()
{
	controlList.clear();
#ifdef MC_IOS
	// Touch: open the MCPE-style 2x2 crafting list from the inventory.
	controlList.push_back(new GuiButton(3, width / 2 + 104, height / 2 - 20, 68, 20, "Craft"));
#endif
}

void GuiInventory::drawGuiContainerForegroundLayer()
{
	fontRenderer->drawString("Crafting", 86, 16, 0x404040);
}

void GuiInventory::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	GuiContainer::drawScreen(mouseX, mouseY, partialTick);
	xSize_lo = (float_t)mouseX;
	ySize_lo = (float_t)mouseY;
}

void GuiInventory::drawGuiContainerBackgroundLayer(float_t partialTick)
{
	int_t tex = mc->renderEngine->getTexture("/gui/inventory.png");
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	mc->renderEngine->bindTexture(tex);
	int_t guiX = (width  - xSize) / 2;
	int_t guiY = (height - ySize) / 2;
	drawTexturedModalRect(guiX, guiY, 0, 0, xSize, ySize);

	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_COLOR_MATERIAL);
	glPushMatrix();
	glTranslatef((float_t)(guiX + 51), (float_t)(guiY + 75), 50.0f);
	float_t scale = 30.0f;
	glScalef(-scale, scale, scale);
	glRotatef(180.0f, 0.0f, 0.0f, 1.0f);

	float_t savedYawOffset = mc->thePlayer->renderYawOffset;
	float_t savedYaw       = mc->thePlayer->rotationYaw;
	float_t savedPitch     = mc->thePlayer->rotationPitch;
	float_t f5 = (float_t)(guiX + 51) - xSize_lo;
	float_t f6 = (float_t)((guiY + 75) - 50) - ySize_lo;

	glRotatef(135.0f, 0.0f, 1.0f, 0.0f);
	RenderHelper::enableStandardItemLighting();
	glRotatef(-135.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(-(float_t)std::atan(f6 / 40.0f) * 20.0f, 1.0f, 0.0f, 0.0f);

	mc->thePlayer->renderYawOffset = (float_t)std::atan(f5 / 40.0f) * 20.0f;
	mc->thePlayer->rotationYaw     = (float_t)std::atan(f5 / 40.0f) * 40.0f;
	mc->thePlayer->rotationPitch   = -(float_t)std::atan(f6 / 40.0f) * 20.0f;
	mc->thePlayer->entityBrightness = 1.0f;

	glTranslatef(0.0f, mc->thePlayer->yOffset, 0.0f);
	RenderManager::instance->playerViewY = 180.0f;
	RenderManager::instance->renderEntityWithPosYaw(mc->thePlayer, 0.0, 0.0, 0.0, 0.0f, 1.0f);

	mc->thePlayer->entityBrightness = 0.0f;
	mc->thePlayer->renderYawOffset  = savedYawOffset;
	mc->thePlayer->rotationYaw      = savedYaw;
	mc->thePlayer->rotationPitch    = savedPitch;

	glPopMatrix();
	RenderHelper::disableStandardItemLighting();
	glDisable(GL_RESCALE_NORMAL);
}

void GuiInventory::actionPerformed(GuiButton *button)
{
	if (button->id == 0)
	{
		mc->displayGuiScreen(new GuiAchievements(mc->statFileWriter));
	}
	if (button->id == 1)
	{
		mc->displayGuiScreen(new GuiStats(this, mc->statFileWriter));
	}
	if (button->id == 3)
	{
		mc->displayGuiScreen(new GuiCraftingMenu(this, mc->thePlayer, false));
	}
}
