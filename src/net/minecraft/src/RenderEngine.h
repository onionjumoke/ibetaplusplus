#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "java/BufferedImage.h"
#include "java/Type.h"

struct SDL_Surface;

class GameSettings;
class ImageBuffer;
class TextureFX;
class TexturePackList;
class ThreadDownloadImageData;

// net.minecraft.src.RenderEngine
class RenderEngine
{
public:
	RenderEngine(TexturePackList *texturepacklist, GameSettings *gamesettings);

	std::vector<int_t> func_28149_a(const std::string &s);
	int_t getTexture(const std::string &s);
	int_t allocateAndSetupTexture(BufferedImage *bufferedimage);
	int_t allocateAndSetupTexture(SDL_Surface *surface);
	void setupTexture(BufferedImage *bufferedimage, int_t i);
	void func_28150_a(const std::vector<int_t> &ai, int_t i, int_t j, int_t k);
	void deleteTexture(int_t i);
	int_t getTextureForDownloadableImage(const std::string &s, const std::string &s1);
	ThreadDownloadImageData *obtainImageData(const std::string &s, ImageBuffer *imagebuffer);
	void releaseImageData(const std::string &s);
	void registerTextureFX(TextureFX *texturefx);
	void updateDynamicTextures();
	void refreshTextures();
	void bindTexture(int_t i);

	static bool useMipmaps;

private:
	std::vector<int_t> func_28148_b(BufferedImage *bufferedimage);
	void func_28147_a(BufferedImage *bufferedimage, std::vector<int_t> &ai);
	std::unique_ptr<BufferedImage> readTextureImage(std::istream *inputstream);
	std::unique_ptr<BufferedImage> unwrapImageByColumns(BufferedImage *bufferedimage);
	std::unique_ptr<BufferedImage> createMissingTexture();
	int_t averageColor(int_t i, int_t j);
	int_t weightedAverageColor(int_t i, int_t j);

	std::map<std::string, int_t> textureMap;
	std::map<std::string, std::vector<int_t>> field_28151_c;
	std::map<int_t, std::shared_ptr<BufferedImage>> textureNameToImageMap;
	std::vector<TextureFX *> textureList;
	std::map<std::string, ThreadDownloadImageData *> urlToImageDataMap;
	GameSettings *options;
	bool clampTexture;
	bool blurTexture;
	TexturePackList *texturePack;
	std::unique_ptr<BufferedImage> missingTextureImage;
};
