#include <algorithm>

#include "GuiAchievements.h"
#include "AchievementList.h"
#include "Achievement.h"
#include "StatFileWriter.h"
#include "StatCollector.h"
#include "GuiSmallButton.h"
#include "GuiButton.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "RenderItem.h"
#include "RenderHelper.h"
#include "MathHelper.h"
#include "Block.h"
#include "Minecraft.h"
#include "pc/OpenGL.h"
#include "KeyBinding.h"
#include "pc/lwjgl/Mouse.h"
#include "java/System.h"
#include "java/Random.h"
#include <cmath>

const int_t GuiAchievements::minScrollX = AchievementList::minDisplayColumn * 24 - 112;
const int_t GuiAchievements::minScrollY = AchievementList::minDisplayRow    * 24 - 112;
const int_t GuiAchievements::maxScrollX = AchievementList::maxDisplayColumn * 24 - 77;
const int_t GuiAchievements::maxScrollY = AchievementList::maxDisplayRow    * 24 - 77;

GuiAchievements::GuiAchievements(StatFileWriter *statfilewriter)
	: bgWidth(256)
	, bgHeight(202)
	, lastMouseX(0)
	, lastMouseY(0)
	, lastScrollX(0.0)
	, lastScrollY(0.0)
	, scrollX(0.0)
	, scrollY(0.0)
	, targetScrollX(0.0)
	, targetScrollY(0.0)
	, mouseState(0)
	, statFileWriter(statfilewriter)
{
	int_t c  = 141;  // '\215'
	int_t c1 = 141;
	targetScrollX = scrollX = lastScrollX = (double)(AchievementList::openInventory->displayColumn * 24 - c / 2 - 12);
	targetScrollY = scrollY = lastScrollY = (double)(AchievementList::openInventory->displayRow    * 24 - c1 / 2);
}

void GuiAchievements::initGui()
{
	controlList.clear();
	controlList.push_back(new GuiSmallButton(1, width / 2 + 24, height / 2 + 74, 80, 20, StatCollector::translateToLocal("gui.done")));
}

void GuiAchievements::actionPerformed(GuiButton *guibutton)
{
	if (guibutton->id == 1)
	{
		mc->displayGuiScreen(nullptr);
		mc->setIngameFocus();
	}
	GuiScreen::actionPerformed(guibutton);
}

void GuiAchievements::keyTyped(char_t c, int_t i)
{
	if (i == mc->gameSettings->keyBindInventory->keyCode)
	{
		mc->displayGuiScreen(nullptr);
		mc->setIngameFocus();
	}
	else
	{
		GuiScreen::keyTyped(c, i);
	}
}

void GuiAchievements::updateScreen()
{
	lastScrollX = scrollX;
	lastScrollY = scrollY;
	double d  = targetScrollX - scrollX;
	double d1 = targetScrollY - scrollY;
	if (d * d + d1 * d1 < 4.0)
	{
		scrollX += d;
		scrollY += d1;
	}
	else
	{
		scrollX += d * 0.85;
		scrollY += d1 * 0.85;
	}
}

void GuiAchievements::drawScreen(int_t i, int_t j, float_t f)
{
	if (lwjgl::Mouse::isButtonDown(0))
	{
		int_t k  = (width  - bgWidth)  / 2;
		int_t l  = (height - bgHeight) / 2;
		int_t i1 = k + 8;
		int_t j1 = l + 17;
		if ((mouseState == 0 || mouseState == 1) && i >= i1 && i < i1 + 224 && j >= j1 && j < j1 + 155)
		{
			if (mouseState == 0)
			{
				mouseState = 1;
			}
			else
			{
				targetScrollX -= i - lastMouseX;
				targetScrollY -= j - lastMouseY;
				scrollX = lastScrollX = targetScrollX;
				scrollY = lastScrollY = targetScrollY;
			}
			lastMouseX = i;
			lastMouseY = j;
		}
		if (targetScrollX < (double)minScrollX) targetScrollX = (double)minScrollX;
		if (targetScrollY < (double)minScrollY) targetScrollY = (double)minScrollY;
		if (targetScrollX >= (double)maxScrollX) targetScrollX = (double)(maxScrollX - 1);
		if (targetScrollY >= (double)maxScrollY) targetScrollY = (double)(maxScrollY - 1);
	}
	else
	{
		mouseState = 0;
	}
	drawDefaultBackground();
	drawAchievementsPanel(i, j, f);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	drawTitle();
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
}

void GuiAchievements::drawTitle()
{
	int_t i = (width  - bgWidth)  / 2;
	int_t j = (height - bgHeight) / 2;
	fontRenderer->drawString("Achievements", i + 15, j + 5, 0x404040);
}

void GuiAchievements::drawAchievementsPanel(int_t i, int_t j, float_t f)
{
	int_t k = MathHelper::floor_double(lastScrollX + (scrollX - lastScrollX) * (double)f);
	int_t l = MathHelper::floor_double(lastScrollY + (scrollY - lastScrollY) * (double)f);
	if (k < minScrollX) k = minScrollX;
	if (l < minScrollY) l = minScrollY;
	if (k >= maxScrollX) k = maxScrollX - 1;
	if (l >= maxScrollY) l = maxScrollY - 1;

	int_t terrainTex  = mc->renderEngine->getTexture("/terrain.png");
	int_t achieveBgTex = mc->renderEngine->getTexture("/achievement/bg.png");
	int_t k1 = (width  - bgWidth)  / 2;
	int_t l1 = (height - bgHeight) / 2;
	int_t i2 = k1 + 16;
	int_t j2 = l1 + 17;
	zLevel = 0.0f;
	glDepthFunc(GL_GEQUAL);  // Java: glDepthFunc(518 / GL_GEQUAL) — draws the map "behind" the bg.png window
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, -200.0f);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_COLOR_MATERIAL);
	mc->renderEngine->bindTexture(terrainTex);

	int_t k2 = (k + 288) >> 4;
	int_t i3 = (l + 288) >> 4;
	int_t j3 = (k + 288) % 16;
	int_t i4 = (l + 288) % 16;
	Random random;
	for (int_t l7 = 0; l7 * 16 - i4 < 155; l7++)
	{
		float_t f5 = 0.6f - ((float_t)(i3 + l7) / 25.0f) * 0.3f;
		glColor4f(f5, f5, f5, 1.0f);
		for (int_t i8 = 0; i8 * 16 - j3 < 224; i8++)
		{
			random.setSeed(1234 + k2 + i8);
			random.nextInt();
			int_t j8 = random.nextInt(1 + i3 + l7) + (i3 + l7) / 2;
			int_t k8 = Block::sand->blockIndexInTexture;
			if (j8 > 37 || i3 + l7 == 35)
			{
				k8 = Block::bedrock->blockIndexInTexture;
			}
			else if (j8 == 22)
			{
				if (random.nextInt(2) == 0) k8 = Block::oreDiamond->blockIndexInTexture;
				else                        k8 = Block::oreRedstone->blockIndexInTexture;
			}
			else if (j8 == 10) k8 = Block::oreIron->blockIndexInTexture;
			else if (j8 == 8)  k8 = Block::oreCoal->blockIndexInTexture;
			else if (j8 > 4)   k8 = Block::stone->blockIndexInTexture;
			else if (j8 > 0)   k8 = Block::dirt->blockIndexInTexture;
			drawTexturedModalRect((i2 + i8 * 16) - j3, (j2 + l7 * 16) - i4, (k8 % 16) << 4, (k8 >> 4) << 4, 16, 16);
		}
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);  // Java: glDepthFunc(515 / GL_LEQUAL)
	glDisable(GL_TEXTURE_2D);

	for (int_t l2 = 0; l2 < (int_t)AchievementList::achievementList.size(); l2++)
	{
		Achievement *achievement1 = AchievementList::achievementList[l2];
		if (achievement1->parentAchievement == nullptr) continue;
		int_t k3 = (achievement1->displayColumn * 24 - k) + 11 + i2;
		int_t j4 = (achievement1->displayRow    * 24 - l) + 11 + j2;
		int_t k4 = (achievement1->parentAchievement->displayColumn * 24 - k) + 11 + i2;
		int_t i5 = (achievement1->parentAchievement->displayRow    * 24 - l) + 11 + j2;
		int_t l5 = 0;
		bool flag  = statFileWriter->hasAchievementUnlocked(achievement1);
		bool flag1 = statFileWriter->canUnlockAchievement(achievement1);
		int_t c = (int_t)(std::sin(((double)(System::currentTimeMillis() % 600LL) / 600.0) * 3.14159265358979 * 2.0) <= 0.6 ? 130 : 255);
		if (flag)        l5 = 0xff707070;
		else if (flag1)  l5 = 65280 + (c << 24);
		else             l5 = 0xff000000;
		drawHorizontalLine(k4, k3, j4, l5);
		drawVerticalLine(k4, j4, i5, l5);
	}

	Achievement *hoveredAchievement = nullptr;
	RenderItem renderitem;
	glPushMatrix();
	glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
	RenderHelper::enableStandardItemLighting();
	glPopMatrix();
	glDisable(GL_LIGHTING);
	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_COLOR_MATERIAL);

	for (int_t l3 = 0; l3 < (int_t)AchievementList::achievementList.size(); l3++)
	{
		Achievement *achievement3 = AchievementList::achievementList[l3];
		int_t l4 = achievement3->displayColumn * 24 - k;
		int_t j5 = achievement3->displayRow    * 24 - l;
		if (l4 < -24 || j5 < -24 || l4 > 224 || j5 > 155) continue;

		if (statFileWriter->hasAchievementUnlocked(achievement3))
		{
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		}
		else if (statFileWriter->canUnlockAchievement(achievement3))
		{
			float_t f2 = (float_t)(std::sin(((double)(System::currentTimeMillis() % 600LL) / 600.0) * 3.14159265358979 * 2.0) >= 0.6 ? 0.8f : 0.6f);
			glColor4f(f2, f2, f2, 1.0f);
		}
		else
		{
			float_t f3 = 0.3f;
			glColor4f(f3, f3, f3, 1.0f);
		}
		mc->renderEngine->bindTexture(achieveBgTex);
		int_t i6 = i2 + l4;
		int_t k6 = j2 + j5;
		if (achievement3->getSpecial())
			drawTexturedModalRect(i6 - 2, k6 - 2, 26, 202, 26, 26);
		else
			drawTexturedModalRect(i6 - 2, k6 - 2,  0, 202, 26, 26);

		if (!statFileWriter->canUnlockAchievement(achievement3))
		{
			float_t f4 = 0.1f;
			glColor4f(f4, f4, f4, 1.0f);
			renderitem.field_27004_a = false;
		}
		glEnable(GL_LIGHTING);
		glEnable(GL_CULL_FACE);
		renderitem.renderItemIntoGUI(mc->fontRenderer, mc->renderEngine, achievement3->theItemStack, i6 + 3, k6 + 3);
		glDisable(GL_LIGHTING);
		if (!statFileWriter->canUnlockAchievement(achievement3))
		{
			renderitem.field_27004_a = true;
		}
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		if (i >= i2 && j >= j2 && i < i2 + 224 && j < j2 + 155
		 && i >= i6 && i <= i6 + 22 && j >= k6 && j <= k6 + 22)
		{
			hoveredAchievement = achievement3;
		}
	}

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	mc->renderEngine->bindTexture(achieveBgTex);
	drawTexturedModalRect(k1, l1, 0, 0, bgWidth, bgHeight);
	glPopMatrix();
	zLevel = 0.0f;
	glDepthFunc(GL_LEQUAL);  // Java: glDepthFunc(515 / GL_LEQUAL)
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	GuiScreen::drawScreen(i, j, f);

	if (hoveredAchievement != nullptr)
	{
		Achievement *achievement2 = hoveredAchievement;
		std::string s  = achievement2->statName;
		std::string s1 = achievement2->getDescription();
		int_t k5 = i + 12;
		int_t j6 = j - 4;
		if (statFileWriter->canUnlockAchievement(achievement2))
		{
			int_t l6 = std::max(fontRenderer->getStringWidth(s), 120);
			int_t j7 = fontRenderer->splitStringWidth(s1, l6);
			if (statFileWriter->hasAchievementUnlocked(achievement2)) j7 += 12;
			drawGradientRect(k5 - 3, j6 - 3, k5 + l6 + 3, j6 + j7 + 3 + 12, 0xc0000000, 0xc0000000);
			fontRenderer->drawSplitString(s1, k5, j6 + 12, l6, 0xffa0a0a0);
			if (statFileWriter->hasAchievementUnlocked(achievement2))
				fontRenderer->drawStringWithShadow(StatCollector::translateToLocal("achievement.taken"), k5, j6 + j7 + 4, 0xff9090ff);
		}
		else
		{
			int_t i7 = std::max(fontRenderer->getStringWidth(s), 120);
			std::string s2 = StatCollector::translateToLocalFormatted("achievement.requires", achievement2->parentAchievement->statName.c_str());
			int_t k7 = fontRenderer->splitStringWidth(s2, i7);
			drawGradientRect(k5 - 3, j6 - 3, k5 + i7 + 3, j6 + k7 + 12 + 3, 0xc0000000, 0xc0000000);
			fontRenderer->drawSplitString(s2, k5, j6 + 12, i7, 0xff705050);
		}
		int_t textColor = statFileWriter->canUnlockAchievement(achievement2)
			? (achievement2->getSpecial() ? -128 : -1)
			: (achievement2->getSpecial() ? 0xff808040 : 0xff808080);
		fontRenderer->drawStringWithShadow(s, k5, j6, textColor);
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	RenderHelper::disableStandardItemLighting();
}

bool GuiAchievements::doesGuiPauseGame()
{
	return true;
}
