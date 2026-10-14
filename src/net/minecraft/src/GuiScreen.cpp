#include "GuiScreen.h"
#include "SoundManager.h"
#include "GuiButton.h"
#include "GuiParticle.h"
#include "Tessellator.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "pc/OpenGL.h"
#include "pc/lwjgl/Keyboard.h"
#include "pc/lwjgl/Mouse.h"

// Minecraft forward-included via header chain
class Minecraft;
#include "Minecraft.h"

GuiScreen::GuiScreen()
	: mc(nullptr)
	, width(0)
	, height(0)
	, field_948_f(false)
	, fontRenderer(nullptr)
	, guiParticles(nullptr)
	, selectedButton(nullptr)
{
}

GuiScreen::~GuiScreen()
{
	// Buttons and the particle system are owned solely by this screen.
	for (GuiButton *btn : controlList)
		delete btn;
	controlList.clear();
	delete guiParticles;
	guiParticles = nullptr;
}

void GuiScreen::drawScreen(int_t mouseX, int_t mouseY, float_t partialTick)
{
	for (int_t i = 0; i < (int_t)controlList.size(); i++)
	{
		controlList[i]->drawButton(mc, mouseX, mouseY);
	}
}

void GuiScreen::keyTyped(char_t c, int_t key)
{
	if (key == 1)
	{
		mc->displayGuiScreen(nullptr);
		mc->setIngameFocus();
	}
}

std::string GuiScreen::getClipboardString()
{
	// SDL clipboard
#if defined(SDL_VERSION)
	char *text = SDL_GetClipboardText();
	if (text)
	{
		std::string s(text);
		SDL_free(text);
		return s;
	}
#endif
	return "";
}

void GuiScreen::mouseClicked(int_t x, int_t y, int_t button)
{
	if (button == 0)
	{
		for (int_t i = 0; i < (int_t)controlList.size(); i++)
		{
			GuiButton *btn = controlList[i];
			if (btn->mousePressed(mc, x, y))
			{
				selectedButton = btn;
				mc->sndManager->playSoundFX("random.click", 1.0f, 1.0f);
				actionPerformed(btn);
			}
		}
	}
}

void GuiScreen::mouseMovedOrUp(int_t x, int_t y, int_t button)
{
	if (selectedButton != nullptr && button == 0)
	{
		selectedButton->mouseReleased(x, y);
		selectedButton = nullptr;
	}
}

void GuiScreen::actionPerformed(GuiButton *button)
{
}

void GuiScreen::setWorldAndResolution(Minecraft *minecraft, int_t w, int_t h)
{
	// Called again on every resize; free the previous frame's buttons/particles
	// instead of leaking them (Java relied on GC).
	delete guiParticles;
	guiParticles = new GuiParticle(minecraft);
	mc = minecraft;
	fontRenderer = minecraft->fontRenderer;
	width = w;
	height = h;
	// selectedButton siempre apunta a un boton de controlList; al destruirlos quedaria
	// colgante y mouseMovedOrUp haria use-after-free (crash al soltar el raton tras
	// una accion que reconstruye la pantalla, p.ej. togglear una opcion). Anularlo.
	selectedButton = nullptr;
	for (GuiButton *btn : controlList)
		delete btn;
	controlList.clear();
	initGui();
}

void GuiScreen::initGui()
{
}

void GuiScreen::handleInput()
{
	while (lwjgl::Mouse::next()) handleMouseInput();
	while (lwjgl::Keyboard::next()) handleKeyboardInput();
}

void GuiScreen::handleMouseInput()
{
	if (lwjgl::Mouse::getEventButtonState())
	{
		int_t x = (lwjgl::Mouse::getEventX() * width) / mc->displayWidth;
		int_t y = height - (lwjgl::Mouse::getEventY() * height) / mc->displayHeight - 1;
		mouseClicked(x, y, lwjgl::Mouse::getEventButton());
	}
	else
	{
		int_t x = (lwjgl::Mouse::getEventX() * width) / mc->displayWidth;
		int_t y = height - (lwjgl::Mouse::getEventY() * height) / mc->displayHeight - 1;
		mouseMovedOrUp(x, y, lwjgl::Mouse::getEventButton());
	}
}

void GuiScreen::handleKeyboardInput()
{
	if (lwjgl::Keyboard::getEventKeyState())
	{
		if (lwjgl::Keyboard::getEventKey() == 87)
		{
			mc->toggleFullscreen();
			return;
		}
		keyTyped(lwjgl::Keyboard::getEventCharacter(), lwjgl::Keyboard::getEventKey());
	}
}

void GuiScreen::updateScreen()
{
}

GuiTextField *GuiScreen::getFocusedTextField()
{
	return nullptr;
}

void GuiScreen::onGuiClosed()
{
}

void GuiScreen::drawDefaultBackground()
{
	drawWorldBackground(0);
}

void GuiScreen::drawWorldBackground(int_t ticks)
{
	if (mc->theWorld != nullptr)
	{
		drawGradientRect(0, 0, width, height, 0xc0101010, 0xd0101010);
	}
	else
	{
		drawBackground(ticks);
	}
}

void GuiScreen::drawBackground(int_t ticks)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);
	Tessellator *tess = &Tessellator::instance;
	glBindTexture(GL_TEXTURE_2D, mc->renderEngine->getTexture("/gui/background.png"));
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	float_t f = 32.0f;
	tess->startDrawingQuads();
	tess->setColorOpaque_I(0x404040);
	tess->addVertexWithUV(0,     height, 0.0, 0.0,              (float_t)height / f + (float_t)ticks);
	tess->addVertexWithUV(width, height, 0.0, (float_t)width / f, (float_t)height / f + (float_t)ticks);
	tess->addVertexWithUV(width, 0,      0.0, (float_t)width / f, 0 + ticks);
	tess->addVertexWithUV(0,     0,      0.0, 0.0,              0 + ticks);
	tess->draw();
}

bool GuiScreen::doesGuiPauseGame()
{
	return true;
}

void GuiScreen::deleteWorld(bool confirmed, int_t worldNum)
{
}

void GuiScreen::selectNextField()
{
}
