#pragma once

#include "TexturePackBase.h"
#include <string>

#ifdef MCBETA_HAVE_LIBZIP
#include <zip.h>
#else
struct zip;
typedef struct zip zip_t;
#endif

class Minecraft;

// net.minecraft.src.TexturePackCustom
class TexturePackCustom : public TexturePackBase
{
public:
	TexturePackCustom(const std::string &file);

	void getTexturePackFolder(Minecraft *minecraft) override;
	void getResourceAsStream(Minecraft *minecraft) override;
	void bindThumbnailTexture(Minecraft *minecraft) override;
	void loadTexturePack() override;
	void closeTexturePackFile() override;
	std::istream* getResourceAsStream(const std::string &s) override;

private:
	std::string truncateString(const std::string &s);

	zip_t *texturePackZipFile;
	int_t texturePackName; // OpenGL texture ID
	void *texturePackThumbnail; // SDL_Surface* or raw image data
	std::string texturePackFile;
};
