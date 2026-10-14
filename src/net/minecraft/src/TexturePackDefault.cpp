#include "TexturePackDefault.h"

#include <fstream>
#include <iostream>
#include <vector>
#include "GameResources.h"
#include "Minecraft.h"
#include "RenderEngine.h"
#include <SDL2/SDL_opengl.h>

TexturePackDefault::TexturePackDefault() :
	texturePackName(-1),
	texturePackThumbnail(nullptr)
{
	texturePackFileName = "Default";
	firstDescriptionLine = "The default look of Minecraft";
}

void TexturePackDefault::getResourceAsStream(Minecraft *minecraft)
{
	if (texturePackThumbnail != nullptr)
	{
		minecraft->renderEngine->deleteTexture(texturePackName);
	}
}

void TexturePackDefault::bindThumbnailTexture(Minecraft *minecraft)
{
	if (texturePackThumbnail != nullptr && texturePackName < 0)
	{
		texturePackName = minecraft->renderEngine->allocateAndSetupTexture((SDL_Surface*)texturePackThumbnail);
	}
	if (texturePackThumbnail != nullptr)
	{
		minecraft->renderEngine->bindTexture(texturePackName);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, minecraft->renderEngine->getTexture("/gui/unknown_pack.png"));
	}
}

std::istream* TexturePackDefault::getResourceAsStream(const std::string &s)
{
	std::string resolved = GameResources::resolve(s);
	if (resolved.empty())
		return nullptr;
	std::ifstream *input = new std::ifstream(resolved, std::ios::binary);
	if (*input)
		return input;
	delete input;
	return nullptr;
}
