#include "GuiEditSign.h"
#include "GuiButton.h"
#include "TileEntitySign.h"
#include "TileEntityRenderer.h"
#include "Block.h"
#include "World.h"
#include "NetClientHandler.h"
#include "Packet130UpdateSign.h"
#include "ChatAllowedCharacters.h"
#include "Minecraft.h"
#include "java/String.h"
#include "pc/lwjgl/Keyboard.h"
#include "pc/OpenGL.h"

GuiEditSign::GuiEditSign(TileEntitySign *sign)
	: screenTitle("Edit sign message:")
	, editLine(0)
	, entitySign(sign)
	, updateCounter(0)
{
}

void GuiEditSign::initGui()
{
	controlList.clear();
	lwjgl::Keyboard::enableRepeatEvents(true);
	controlList.push_back(new GuiButton(0, width / 2 - 100, height / 4 + 120, "Done"));
}

void GuiEditSign::onGuiClosed()
{
	lwjgl::Keyboard::enableRepeatEvents(false);
	if (mc->theWorld->multiplayerWorld)
	{
		Packet130UpdateSign *pkt = new Packet130UpdateSign();
		pkt->xPosition = entitySign->xCoord;
		pkt->yPosition = entitySign->yCoord;
		pkt->zPosition = entitySign->zCoord;
		for (int i = 0; i < 4; i++) pkt->signLines[i] = String::fromUTF8(entitySign->signText[i]);
		mc->getSendQueue()->addToSendQueue(pkt);
	}
}

void GuiEditSign::updateScreen()
{
	updateCounter++;
}

void GuiEditSign::actionPerformed(GuiButton *button)
{
	if (!button->enabled) return;
	if (button->id == 0)
	{
		entitySign->onInventoryChanged();
		mc->displayGuiScreen(nullptr);
	}
}

void GuiEditSign::keyTyped(char_t c, int_t key)
{
	if (key == 200) // Up
		editLine = (editLine - 1) & 3;
	if (key == 208 || key == 28) // Down or Enter
		editLine = (editLine + 1) & 3;
	if (key == 14 && !entitySign->signText[editLine].empty()) // Backspace
		entitySign->signText[editLine] = entitySign->signText[editLine].substr(0, entitySign->signText[editLine].length() - 1);
	if (ChatAllowedCharacters::allowedCharacters.find(c) != std::string::npos && (int_t)entitySign->signText[editLine].length() < 15)
		entitySign->signText[editLine] += c;
}

void GuiEditSign::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	drawDefaultBackground();
	drawCenteredString(fontRenderer, screenTitle, width / 2, 40, 0xffffff);

	glPushMatrix();
	glTranslatef((float_t)(width / 2), 0.0f, 50.0f);
	float_t s = 93.75f;
	glScalef(-s, -s, -s);
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);

	Block *block = entitySign->getBlockType();
	if (block == Block::signPost)
	{
		float_t angle = (float_t)(entitySign->getBlockMetadata() * 360) / 16.0f;
		glRotatef(angle, 0.0f, 1.0f, 0.0f);
		glTranslatef(0.0f, -1.0625f, 0.0f);
	}
	else
	{
		int_t meta = entitySign->getBlockMetadata();
		float_t angle = 0.0f;
		if (meta == 2) angle = 180.0f;
		if (meta == 4) angle =  90.0f;
		if (meta == 5) angle = -90.0f;
		glRotatef(angle, 0.0f, 1.0f, 0.0f);
		glTranslatef(0.0f, -1.0625f, 0.0f);
	}

	if ((updateCounter / 6) % 2 == 0)
		entitySign->lineBeingEdited = editLine;
	TileEntityRenderer::instance.renderTileEntityAt(entitySign, -0.5, -0.75, -0.5, 0.0f);
	entitySign->lineBeingEdited = -1;
	glPopMatrix();

	GuiScreen::drawScreen(mouseX, mouseY, partialTick);
}
