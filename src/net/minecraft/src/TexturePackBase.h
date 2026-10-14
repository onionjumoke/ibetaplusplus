#pragma once

#include <string>
#include "java/String.h"

class Minecraft;

// net.minecraft.src.TexturePackBase
class TexturePackBase
{
public:
	TexturePackBase();
	virtual ~TexturePackBase() = default;

	virtual void loadTexturePack(); // openTexturePackFile
	virtual void closeTexturePackFile();
	virtual void getTexturePackFolder(Minecraft *minecraft); // loadTexturePack
	virtual void getResourceAsStream(Minecraft *minecraft); // unloadTexturePack
	virtual void bindThumbnailTexture(Minecraft *minecraft);
	virtual std::istream* getResourceAsStream(const std::string &s);

	jstring texturePackFileName;
	jstring firstDescriptionLine;
	jstring secondDescriptionLine;
	jstring texturePackFolder; // texturePackId / cacheKey
};
