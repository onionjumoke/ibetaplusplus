#pragma once

#include "TexturePackBase.h"

class Minecraft;

// net.minecraft.src.TexturePackDefault
class TexturePackDefault : public TexturePackBase
{
public:
	TexturePackDefault();

	void getResourceAsStream(Minecraft *minecraft) override;
	void bindThumbnailTexture(Minecraft *minecraft) override;
	std::istream* getResourceAsStream(const std::string &s) override;

private:
	int_t texturePackName;
	void *texturePackThumbnail; // SDL_Surface*
};
