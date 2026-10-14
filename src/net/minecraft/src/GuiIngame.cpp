#include "GuiIngame.h"
#include "ScaledResolution.h"
#include "EntityRenderer.h"
#include "EntityPlayerSP.h"
#include "InventoryPlayer.h"
#include "GameSettings.h"
#include "ItemStack.h"
#include "Block.h"
#include "BlockPortal.h"
#include "RenderEngine.h"
#include "PlayerController.h"
#include "Material.h"
#include "RenderHelper.h"
#include "RenderItem.h"
#include "FontRenderer.h"
#include "MathHelper.h"
#include "GuiChat.h"
#include "ChatLine.h"
#include "Tessellator.h"
#include "StringTranslate.h"
#include "Minecraft.h"
#include "java/Random.h"
#include "java/System.h"
#include "java/Runtime.h"
#include "pc/OpenGL.h"
#include <cmath>
#include <algorithm>

// HSB to RGB helper (Java Color.HSBtoRGB equivalent)
static int_t hsbToRgb(float_t hue, float_t sat, float_t bri)
{
	float_t r = bri, g = bri, b = bri;
	if (sat != 0.0f)
	{
		float_t h = (hue - std::floor(hue)) * 6.0f;
		float_t f = h - std::floor(h);
		float_t p = bri * (1.0f - sat);
		float_t q = bri * (1.0f - sat * f);
		float_t t = bri * (1.0f - (sat * (1.0f - f)));
		switch ((int)h)
		{
		case 0: r = bri; g = t;   b = p;   break;
		case 1: r = q;   g = bri; b = p;   break;
		case 2: r = p;   g = bri; b = t;   break;
		case 3: r = p;   g = q;   b = bri; break;
		case 4: r = t;   g = p;   b = bri; break;
		case 5: r = bri; g = p;   b = q;   break;
		}
	}
	int_t ri = (int_t)(r * 255.0f + 0.5f);
	int_t gi = (int_t)(g * 255.0f + 0.5f);
	int_t bi = (int_t)(b * 255.0f + 0.5f);
	return 0xff000000 | (ri << 16) | (gi << 8) | bi;
}

RenderItem *GuiIngame::itemRenderer = new RenderItem();

GuiIngame::GuiIngame(Minecraft *minecraft)
	: mc(minecraft)
	, rand(new Random())
	, field_933_a("")
	, updateCounter(0)
	, recordPlaying("")
	, recordPlayingUpFor(0)
	, field_22065_l(false)
	, damageGuiPartialTime(0.0f)
	, prevVignetteBrightness(1.0f)
{
}

void GuiIngame::renderGameOverlay(float_t partialTick, bool showDebug, int_t mouseX, int_t mouseY)
{
	ScaledResolution sr(mc->gameSettings, mc->displayWidth, mc->displayHeight);
	int_t sw = sr.getScaledWidth();
	int_t sh = sr.getScaledHeight();
	FontRenderer *fr = mc->fontRenderer;

	mc->entityRenderer->setupOverlayRendering();
	glEnable(GL_BLEND);

	if (Minecraft::isFancyGraphicsEnabled())
		renderVignette(mc->thePlayer->getEntityBrightness(partialTick), sw, sh);

	ItemStack *helmet = mc->thePlayer->inventory->armorItemInSlot(3);
	if (!mc->gameSettings->thirdPersonView && helmet != nullptr && helmet->itemID == Block::pumpkin->blockID)
		renderPumpkinBlur(sw, sh);

	float_t portalIntensity = mc->thePlayer->prevTimeInPortal
	    + (mc->thePlayer->timeInPortal - mc->thePlayer->prevTimeInPortal) * partialTick;
	if (portalIntensity > 0.0f)
		renderPortalOverlay(portalIntensity, sw, sh);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, mc->renderEngine->getTexture("/gui/gui.png"));
	InventoryPlayer *inv = mc->thePlayer->inventory;
	zLevel = -90.0f;
	drawTexturedModalRect(sw / 2 - 91, sh - 22, 0,  0, 182, 22);
	drawTexturedModalRect((sw / 2 - 91 - 1) + inv->currentItem * 20, sh - 22 - 1, 0, 22, 24, 22);

	glBindTexture(GL_TEXTURE_2D, mc->renderEngine->getTexture("/gui/icons.png"));
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
	drawTexturedModalRect(sw / 2 - 7, sh / 2 - 7, 0, 0, 16, 16);
	glDisable(GL_BLEND);

	bool flashHearts = (mc->thePlayer->heartsLife / 3) % 2 == 1;
	if (mc->thePlayer->heartsLife < 10) flashHearts = false;
	int_t health     = mc->thePlayer->health;
	int_t prevHealth = mc->thePlayer->prevHealth;
	rand->setSeed((long_t)updateCounter * 0x4c627L);

	if (mc->playerController->shouldDrawHUD())
	{
		int_t armor = mc->thePlayer->getPlayerArmorValue();
		for (int_t j2 = 0; j2 < 10; j2++)
		{
			int_t k3 = sh - 32;
			if (armor > 0)
			{
				int_t armorX = (sw / 2 + 91) - j2 * 8 - 9;
				if (j2 * 2 + 1 < armor)  drawTexturedModalRect(armorX, k3, 34, 9, 9, 9);
				if (j2 * 2 + 1 == armor) drawTexturedModalRect(armorX, k3, 25, 9, 9, 9);
				if (j2 * 2 + 1 > armor)  drawTexturedModalRect(armorX, k3, 16, 9, 9, 9);
			}
			int_t flash = flashHearts ? 1 : 0;
			int_t hx = (sw / 2 - 91) + j2 * 8;
			if (health <= 4) k3 += rand->nextInt(2);
			drawTexturedModalRect(hx, k3, 16 + flash * 9, 0, 9, 9);
			if (flashHearts)
			{
				if (j2 * 2 + 1 < prevHealth)  drawTexturedModalRect(hx, k3, 70, 0, 9, 9);
				if (j2 * 2 + 1 == prevHealth) drawTexturedModalRect(hx, k3, 79, 0, 9, 9);
			}
			if (j2 * 2 + 1 < health)  drawTexturedModalRect(hx, k3, 52, 0, 9, 9);
			if (j2 * 2 + 1 == health) drawTexturedModalRect(hx, k3, 61, 0, 9, 9);
		}

		if (mc->thePlayer->isInsideOfMaterial(Material::water))
		{
			int_t full = (int_t)std::ceil(((double)(mc->thePlayer->air - 2) * 10.0) / 300.0);
			int_t empty = (int_t)std::ceil(((double)mc->thePlayer->air * 10.0) / 300.0) - full;
			for (int_t l5 = 0; l5 < full + empty; l5++)
			{
				if (l5 < full)
					drawTexturedModalRect((sw / 2 - 91) + l5 * 8, sh - 32 - 9, 16, 18, 9, 9);
				else
					drawTexturedModalRect((sw / 2 - 91) + l5 * 8, sh - 32 - 9, 25, 18, 9, 9);
			}
		}
	}

	glDisable(GL_BLEND);
	glEnable(GL_RESCALE_NORMAL);
	glPushMatrix();
	glRotatef(120.0f, 1.0f, 0.0f, 0.0f);
	RenderHelper::enableStandardItemLighting();
	glPopMatrix();
	for (int_t l1 = 0; l1 < 9; l1++)
	{
		int_t ix = (sw / 2 - 90) + l1 * 20 + 2;
		int_t iy = sh - 16 - 3;
		renderInventorySlot(l1, ix, iy, partialTick);
	}
	RenderHelper::disableStandardItemLighting();
	glDisable(GL_RESCALE_NORMAL);

	if (mc->thePlayer->getSleepTimer() > 0)
	{
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_ALPHA_TEST);
		int_t sleepT = mc->thePlayer->getSleepTimer();
		float_t f3 = (float_t)sleepT / 100.0f;
		if (f3 > 1.0f) f3 = 1.0f - (float_t)(sleepT - 100) / 10.0f;
		int_t sleepColor = (int_t)(220.0f * f3) << 24 | 0x101020;
		drawRect(0, 0, sw, sh, sleepColor);
		glEnable(GL_ALPHA_TEST);
		glEnable(GL_DEPTH_TEST);
	}

	if (mc->gameSettings->showDebugInfo)
	{
		glPushMatrix();
		if (Minecraft::hasPaidCheckTime > 0LL)
			glTranslatef(0.0f, 32.0f, 0.0f);
		fr->drawStringWithShadow("iBetaPlusPlus (" + mc->debug + ")", 2, 2, 0xffffff);
		fr->drawStringWithShadow(mc->getDebugLine1(), 2, 12, 0xffffff);
		fr->drawStringWithShadow(mc->getDebugLine2(), 2, 22, 0xffffff);
		fr->drawStringWithShadow(mc->getDebugLine3(), 2, 32, 0xffffff);
		fr->drawStringWithShadow(mc->getDebugLine4(), 2, 42, 0xffffff);
		Runtime &rt = Runtime::getRuntime();
		long_t maxMem   = rt.maxMemory();
		long_t totalMem = rt.totalMemory();
		long_t freeMem  = rt.freeMemory();
		long_t usedMem  = totalMem - freeMem;
		std::string memUsed = "Used memory: " + std::to_string((usedMem * 100LL) / maxMem) + "% ("
		    + std::to_string(usedMem / 1024LL / 1024LL) + "MB) of "
		    + std::to_string(maxMem / 1024LL / 1024LL) + "MB";
		drawString(fr, memUsed, sw - fr->getStringWidth(memUsed) - 2, 2, 0xe0e0e0);
		std::string memAlloc = "Allocated memory: " + std::to_string((totalMem * 100LL) / maxMem) + "% ("
		    + std::to_string(totalMem / 1024LL / 1024LL) + "MB)";
		drawString(fr, memAlloc, sw - fr->getStringWidth(memAlloc) - 2, 12, 0xe0e0e0);
		drawString(fr, "x: " + std::to_string(mc->thePlayer->posX), 2, 64, 0xe0e0e0);
		drawString(fr, "y: " + std::to_string(mc->thePlayer->posY), 2, 72, 0xe0e0e0);
		drawString(fr, "z: " + std::to_string(mc->thePlayer->posZ), 2, 80, 0xe0e0e0);
		drawString(fr, "f: " + std::to_string(MathHelper::floor_double((double)((mc->thePlayer->rotationYaw * 4.0f) / 360.0f) + 0.5) & 3), 2, 88, 0xe0e0e0);
		glPopMatrix();
	}

	if (recordPlayingUpFor > 0)
	{
		float_t f2 = (float_t)recordPlayingUpFor - partialTick;
		int_t alpha = (int_t)((f2 * 256.0f) / 20.0f);
		if (alpha > 255) alpha = 255;
		if (alpha > 0)
		{
			glPushMatrix();
			glTranslatef((float_t)(sw / 2), (float_t)(sh - 48), 0.0f);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			int_t color = 0xffffff;
			if (field_22065_l)
				color = hsbToRgb(f2 / 50.0f, 0.7f, 0.6f) & 0xffffff;
			fr->drawString(recordPlaying, -fr->getStringWidth(recordPlaying) / 2, -4, color + (alpha << 24));
			glDisable(GL_BLEND);
			glPopMatrix();
		}
	}

	int_t chatLines = 10;
	bool chatOpen = false;
	if (dynamic_cast<GuiChat *>(mc->currentScreen) != nullptr)
	{
		chatLines = 20;
		chatOpen = true;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_ALPHA_TEST);
	glPushMatrix();
	glTranslatef(0.0f, (float_t)(sh - 48), 0.0f);
	for (int_t i5 = 0; i5 < (int_t)chatMessageList.size() && i5 < chatLines; i5++)
	{
		ChatLine *line = chatMessageList[i5];
		if (line->updateCounter >= 200 && !chatOpen) continue;
		double d = (double)line->updateCounter / 200.0;
		d = 1.0 - d;
		d *= 10.0;
		if (d < 0.0) d = 0.0;
		if (d > 1.0) d = 1.0;
		d *= d;
		int_t msgAlpha = (int_t)(255.0 * d);
		if (chatOpen) msgAlpha = 255;
		if (msgAlpha > 0)
		{
			int_t cx = 2;
			int_t cy = -i5 * 9;
			drawRect(cx, cy - 1, cx + 320, cy + 8, msgAlpha / 2 << 24);
			glEnable(GL_BLEND);
			fr->drawStringWithShadow(line->message, cx, cy, 0xffffff + (msgAlpha << 24));
		}
	}
	glPopMatrix();
	glEnable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
}

void GuiIngame::renderPumpkinBlur(int_t w, int_t h)
{
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_ALPHA_TEST);
	glBindTexture(GL_TEXTURE_2D, mc->renderEngine->getTexture("%blur%/misc/pumpkinblur.png"));
	Tessellator *tess = &Tessellator::instance;
	tess->startDrawingQuads();
	tess->addVertexWithUV(0, h, -90.0, 0.0, 1.0);
	tess->addVertexWithUV(w, h, -90.0, 1.0, 1.0);
	tess->addVertexWithUV(w, 0, -90.0, 1.0, 0.0);
	tess->addVertexWithUV(0, 0, -90.0, 0.0, 0.0);
	tess->draw();
	glDepthMask(true);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void GuiIngame::renderVignette(float_t brightness, int_t w, int_t h)
{
	float_t f = 1.0f - brightness;
	if (f < 0.0f) f = 0.0f;
	if (f > 1.0f) f = 1.0f;
	prevVignetteBrightness += (double)(f - prevVignetteBrightness) * 0.01;
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);
	glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
	glColor4f(prevVignetteBrightness, prevVignetteBrightness, prevVignetteBrightness, 1.0f);
	glBindTexture(GL_TEXTURE_2D, mc->renderEngine->getTexture("%blur%/misc/vignette.png"));
	Tessellator *tess = &Tessellator::instance;
	tess->startDrawingQuads();
	tess->addVertexWithUV(0, h, -90.0, 0.0, 1.0);
	tess->addVertexWithUV(w, h, -90.0, 1.0, 1.0);
	tess->addVertexWithUV(w, 0, -90.0, 1.0, 0.0);
	tess->addVertexWithUV(0, 0, -90.0, 0.0, 0.0);
	tess->draw();
	glDepthMask(true);
	glEnable(GL_DEPTH_TEST);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void GuiIngame::renderPortalOverlay(float_t intensity, int_t w, int_t h)
{
	if (intensity < 1.0f)
	{
		intensity *= intensity;
		intensity *= intensity;
		intensity = intensity * 0.8f + 0.2f;
	}
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0f, 1.0f, 1.0f, intensity);
	glBindTexture(GL_TEXTURE_2D, mc->renderEngine->getTexture("/terrain.png"));
	float_t u1 = (float_t)(Block::portal->blockIndexInTexture % 16)       / 16.0f;
	float_t v1 = (float_t)(Block::portal->blockIndexInTexture / 16)       / 16.0f;
	float_t u2 = (float_t)(Block::portal->blockIndexInTexture % 16 + 1)   / 16.0f;
	float_t v2 = (float_t)(Block::portal->blockIndexInTexture / 16 + 1)   / 16.0f;
	Tessellator *tess = &Tessellator::instance;
	tess->startDrawingQuads();
	tess->addVertexWithUV(0, h, -90.0, u1, v2);
	tess->addVertexWithUV(w, h, -90.0, u2, v2);
	tess->addVertexWithUV(w, 0, -90.0, u2, v1);
	tess->addVertexWithUV(0, 0, -90.0, u1, v1);
	tess->draw();
	glDepthMask(true);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void GuiIngame::renderInventorySlot(int_t slot, int_t x, int_t y, float_t partialTick)
{
	ItemStack *stack = mc->thePlayer->inventory->mainInventory[slot];
	if (stack == nullptr) return;

	float_t animF = (float_t)stack->animationsToGo - partialTick;
	if (animF > 0.0f)
	{
		glPushMatrix();
		float_t scale = 1.0f + animF / 5.0f;
		glTranslatef((float_t)(x + 8), (float_t)(y + 12), 0.0f);
		glScalef(1.0f / scale, (scale + 1.0f) / 2.0f, 1.0f);
		glTranslatef(-(float_t)(x + 8), -(float_t)(y + 12), 0.0f);
	}
	itemRenderer->renderItemIntoGUI(mc->fontRenderer, mc->renderEngine, stack, x, y);
	if (animF > 0.0f) glPopMatrix();
	itemRenderer->renderItemOverlayIntoGUI(mc->fontRenderer, mc->renderEngine, stack, x, y);
}

void GuiIngame::updateTick()
{
	if (recordPlayingUpFor > 0) recordPlayingUpFor--;
	updateCounter++;
	for (int_t i = 0; i < (int_t)chatMessageList.size(); i++)
		chatMessageList[i]->updateCounter++;
}

void GuiIngame::clearChatMessages()
{
	chatMessageList.clear();
}

void GuiIngame::addChatMessage(const std::string &msg)
{
	std::string s = msg;
	while (mc->fontRenderer->getStringWidth(s) > 320)
	{
		int_t i = 1;
		while (i < (int_t)s.length() && mc->fontRenderer->getStringWidth(s.substr(0, i + 1)) <= 320)
			i++;
		addChatMessage(s.substr(0, i));
		s = s.substr(i);
	}
	chatMessageList.insert(chatMessageList.begin(), new ChatLine(s));
	while ((int_t)chatMessageList.size() > 50)
		chatMessageList.pop_back();
}

void GuiIngame::setRecordPlayingMessage(const std::string &record)
{
	recordPlaying = "Now playing: " + record;
	recordPlayingUpFor = 60;
	field_22065_l = true;
}

void GuiIngame::addChatMessageTranslate(const std::string &key)
{
	StringTranslate *tr = StringTranslate::getInstance();
	addChatMessage(tr->translateKey(key));
}
