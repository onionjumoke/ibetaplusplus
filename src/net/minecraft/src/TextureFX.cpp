#include "TextureFX.h"

#include "RenderEngine.h"
#include <SDL2/SDL_opengl.h>

TextureFX::TextureFX(int_t iconIndex) :
	iconIndex(iconIndex),
	anaglyphEnabled(false),
	textureId(0),
	tileSize(1),
	tileImage(0)
{
	for (int_t i = 0; i < 1024; i++)
		imageData[i] = 0;
}

void TextureFX::onTick()
{
}

void TextureFX::bindImage(RenderEngine *renderengine)
{
	if (tileImage == 0)
	{
		glBindTexture(GL_TEXTURE_2D, renderengine->getTexture("/terrain.png"));
	}
	else if (tileImage == 1)
	{
		glBindTexture(GL_TEXTURE_2D, renderengine->getTexture("/gui/items.png"));
	}
}
