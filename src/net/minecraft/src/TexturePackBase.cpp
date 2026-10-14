#include "TexturePackBase.h"

#include <fstream>

TexturePackBase::TexturePackBase()
{
}

void TexturePackBase::loadTexturePack()
{
}

void TexturePackBase::closeTexturePackFile()
{
}

void TexturePackBase::getTexturePackFolder(Minecraft *minecraft)
{
}

void TexturePackBase::getResourceAsStream(Minecraft *minecraft)
{
}

void TexturePackBase::bindThumbnailTexture(Minecraft *minecraft)
{
}

std::istream* TexturePackBase::getResourceAsStream(const std::string &s)
{
	// Default: load from filesystem
	return new std::ifstream(s, std::ios::binary);
}
