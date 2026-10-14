#include "GuiAchievement.h"
#include "Minecraft.h"
#include "Achievement.h"
#include "RenderItem.h"
#include "RenderHelper.h"
#include "RenderEngine.h"
#include "ScaledResolution.h"
#include "FontRenderer.h"
#include "StatCollector.h"
#include "pc/OpenGL.h"
#include "java/System.h"

GuiAchievement::GuiAchievement(Minecraft *minecraft)
	: theGame(minecraft)
	, achievementWindowWidth(0)
	, achievementWindowHeight(0)
	, theAchievement(nullptr)
	, field_25083_f(0LL)
	, itemRender(new RenderItem())
	, field_27103_i(false)
{
}

void GuiAchievement::queueTakenAchievement(Achievement *achievement)
{
	field_25085_d = StatCollector::translateToLocal("achievement.get");
	field_25084_e = achievement->statName;
	field_25083_f = System::currentTimeMillis();
	theAchievement = achievement;
	field_27103_i = false;
}

void GuiAchievement::queueAchievementInformation(Achievement *achievement)
{
	field_25085_d = achievement->statName;
	field_25084_e = achievement->getDescription();
	field_25083_f = System::currentTimeMillis() - 2500LL;
	theAchievement = achievement;
	field_27103_i = true;
}

void GuiAchievement::updateAchievementWindowScale()
{
	glViewport(0, 0, theGame->displayWidth, theGame->displayHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	achievementWindowWidth  = theGame->displayWidth;
	achievementWindowHeight = theGame->displayHeight;
	ScaledResolution scaledresolution(theGame->gameSettings, theGame->displayWidth, theGame->displayHeight);
	achievementWindowWidth  = scaledresolution.getScaledWidth();
	achievementWindowHeight = scaledresolution.getScaledHeight();
	glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, achievementWindowWidth, achievementWindowHeight, 0.0, 1000.0, 3000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -2000.0f);
}

void GuiAchievement::updateAchievementWindow()
{
	if (Minecraft::hasPaidCheckTime > 0LL)
	{
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		RenderHelper::disableStandardItemLighting();
		updateAchievementWindowScale();
		std::string s  = "Minecraft Beta 1.7.3   Unlicensed Copy :(";
		std::string s1 = "(Or logged in from another location)";
		std::string s2 = "Purchase at minecraft.net";
		theGame->fontRenderer->drawStringWithShadow(s,  2,  2, 0xffffff);
		theGame->fontRenderer->drawStringWithShadow(s1, 2, 11, 0xffffff);
		theGame->fontRenderer->drawStringWithShadow(s2, 2, 20, 0xffffff);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
	}
	if (theAchievement == nullptr || field_25083_f == 0LL)
	{
		return;
	}
	double d = (double)(System::currentTimeMillis() - field_25083_f) / 3000.0;
	if (!field_27103_i && (d < 0.0 || d > 1.0))
	{
		field_25083_f = 0LL;
		return;
	}
	updateAchievementWindowScale();
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	double d1 = d * 2.0;
	if (d1 > 1.0) d1 = 2.0 - d1;
	d1 *= 4.0;
	d1  = 1.0 - d1;
	if (d1 < 0.0) d1 = 0.0;
	d1 *= d1;
	d1 *= d1;
	int_t i = achievementWindowWidth - 160;
	int_t j = 0 - (int_t)(d1 * 36.0);
	int_t k = theGame->renderEngine->getTexture("/achievement/bg.png");
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, k);
	glDisable(GL_LIGHTING);
	drawTexturedModalRect(i, j, 96, 202, 160, 32);
	if (field_27103_i)
	{
		theGame->fontRenderer->drawSplitString(field_25084_e, i + 30, j + 7, 120, -1);
	}
	else
	{
		theGame->fontRenderer->drawString(field_25085_d, i + 30, j + 7, -256);
		theGame->fontRenderer->drawString(field_25084_e, i + 30, j + 18, -1);
	}
	glPushMatrix();
	glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
	RenderHelper::enableStandardItemLighting();
	glPopMatrix();
	glDisable(GL_LIGHTING);
	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	itemRender->renderItemIntoGUI(theGame->fontRenderer, theGame->renderEngine, theAchievement->theItemStack, i + 8, j + 8);
	glDisable(GL_LIGHTING);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}
