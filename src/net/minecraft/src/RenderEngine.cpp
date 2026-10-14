#include "RenderEngine.h"

#include <algorithm>
#include <cstring>
#include <iostream>

#include "GameSettings.h"
#include "GLAllocation.h"
#include "TextureFX.h"
#include "TexturePackBase.h"
#include "TexturePackList.h"
#include "ThreadDownloadImageData.h"
#include "pc/OpenGL.h"

#include <SDL2/SDL.h>

bool RenderEngine::useMipmaps = false;

RenderEngine::RenderEngine(TexturePackList *texturepacklist, GameSettings *gamesettings)
	: options(gamesettings)
	, clampTexture(false)
	, blurTexture(false)
	, texturePack(texturepacklist)
	, missingTextureImage(createMissingTexture())
{
}

std::vector<int_t> RenderEngine::func_28149_a(const std::string &s)
{
	auto it = field_28151_c.find(s);
	if (it != field_28151_c.end())
		return it->second;

	try
	{
		std::unique_ptr<BufferedImage> image;
		TexturePackBase *texturepackbase = texturePack != nullptr ? texturePack->getSelectedTexturePack() : nullptr;
		if (s.rfind("##", 0) == 0)
		{
			image = unwrapImageByColumns(readTextureImage(texturepackbase->getResourceAsStream(s.substr(2))).get());
		}
		else if (s.rfind("%clamp%", 0) == 0)
		{
			clampTexture = true;
			image = readTextureImage(texturepackbase->getResourceAsStream(s.substr(7)));
			clampTexture = false;
		}
		else if (s.rfind("%blur%", 0) == 0)
		{
			blurTexture = true;
			image = readTextureImage(texturepackbase->getResourceAsStream(s.substr(6)));
			blurTexture = false;
		}
		else if (texturepackbase != nullptr)
		{
			image = readTextureImage(texturepackbase->getResourceAsStream(s));
		}

		std::vector<int_t> pixels = func_28148_b(image ? image.get() : missingTextureImage.get());
		field_28151_c[s] = pixels;
		return pixels;
	}
	catch (...)
	{
		std::vector<int_t> pixels = func_28148_b(missingTextureImage.get());
		field_28151_c[s] = pixels;
		return pixels;
	}
}

std::vector<int_t> RenderEngine::func_28148_b(BufferedImage *bufferedimage)
{
	if (bufferedimage == nullptr)
		bufferedimage = missingTextureImage.get();

	int_t width = bufferedimage->getWidth();
	int_t height = bufferedimage->getHeight();
	std::vector<int_t> pixels(width * height);
	const unsigned char *raw = bufferedimage->getRawPixels();
	for (int_t i = 0; i < width * height; i++)
	{
		int_t r = raw[i * 4 + 0] & 0xff;
		int_t g = raw[i * 4 + 1] & 0xff;
		int_t b = raw[i * 4 + 2] & 0xff;
		int_t a = raw[i * 4 + 3] & 0xff;
		pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
	}
	return pixels;
}

void RenderEngine::func_28147_a(BufferedImage *bufferedimage, std::vector<int_t> &ai)
{
	ai = func_28148_b(bufferedimage);
}

int_t RenderEngine::getTexture(const std::string &s)
{
	auto it = textureMap.find(s);
	if (it != textureMap.end())
		return it->second;

	std::vector<int_t> single(1);
	GLAllocation::generateTextureNames(single);
	int_t texture = single[0];

	try
	{
		std::unique_ptr<BufferedImage> image;
		TexturePackBase *texturepackbase = texturePack != nullptr ? texturePack->getSelectedTexturePack() : nullptr;
		if (s.rfind("##", 0) == 0)
		{
			image = unwrapImageByColumns(readTextureImage(texturepackbase->getResourceAsStream(s.substr(2))).get());
		}
		else if (s.rfind("%clamp%", 0) == 0)
		{
			clampTexture = true;
			image = readTextureImage(texturepackbase->getResourceAsStream(s.substr(7)));
		}
		else if (s.rfind("%blur%", 0) == 0)
		{
			blurTexture = true;
			image = readTextureImage(texturepackbase->getResourceAsStream(s.substr(6)));
		}
		else if (texturepackbase != nullptr)
		{
			image = readTextureImage(texturepackbase->getResourceAsStream(s));
		}
		// setupTexture() reads clampTexture/blurTexture to pick GL_CLAMP/GL_LINEAR, so it MUST run
		// while the flags are still set; reset them AFTER (as Java does), not inside the branch above.
		setupTexture(image ? image.get() : missingTextureImage.get(), texture);
	}
	catch (...)
	{
		setupTexture(missingTextureImage.get(), texture);
	}

	clampTexture = false;
	blurTexture = false;
	textureMap[s] = texture;
	return texture;
}

std::unique_ptr<BufferedImage> RenderEngine::unwrapImageByColumns(BufferedImage *bufferedimage)
{
	if (bufferedimage == nullptr)
		return createMissingTexture();

	int_t columns = bufferedimage->getWidth() / 16;
	std::unique_ptr<BufferedImage> image(new BufferedImage(16, bufferedimage->getHeight() * columns));
	const unsigned char *src = bufferedimage->getRawPixels();
	std::vector<unsigned char> column(16 * bufferedimage->getHeight() * 4);
	for (int_t col = 0; col < columns; col++)
	{
		for (int_t y = 0; y < bufferedimage->getHeight(); y++)
		{
			for (int_t x = 0; x < 16; x++)
			{
				int_t srcIndex = (y * bufferedimage->getWidth() + col * 16 + x) * 4;
				int_t dstIndex = (y * 16 + x) * 4;
				std::memcpy(&column[dstIndex], &src[srcIndex], 4);
			}
		}
		image->setRGB(0, col * bufferedimage->getHeight(), 16, bufferedimage->getHeight(), column.data());
	}
	return image;
}

int_t RenderEngine::allocateAndSetupTexture(BufferedImage *bufferedimage)
{
	std::vector<int_t> single(1);
	GLAllocation::generateTextureNames(single);
	int_t texture = single[0];
	setupTexture(bufferedimage != nullptr ? bufferedimage : missingTextureImage.get(), texture);
	if (bufferedimage != nullptr)
	{
		int_t width = bufferedimage->getWidth();
		int_t height = bufferedimage->getHeight();
		std::unique_ptr<unsigned char[]> copy(new unsigned char[width * height * 4]);
		std::memcpy(copy.get(), bufferedimage->getRawPixels(), width * height * 4);
		textureNameToImageMap[texture] = std::shared_ptr<BufferedImage>(new BufferedImage(width, height, std::move(copy)));
	}
	return texture;
}

int_t RenderEngine::allocateAndSetupTexture(SDL_Surface *surface)
{
	if (surface == nullptr)
		return allocateAndSetupTexture((BufferedImage *)nullptr);

	SDL_Surface *converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
	if (converted == nullptr)
		return allocateAndSetupTexture((BufferedImage *)nullptr);

	std::unique_ptr<unsigned char[]> data(new unsigned char[converted->w * converted->h * 4]);
	std::memcpy(data.get(), converted->pixels, converted->w * converted->h * 4);
	BufferedImage image(converted->w, converted->h, std::move(data));
	SDL_FreeSurface(converted);
	return allocateAndSetupTexture(&image);
}

void RenderEngine::setupTexture(BufferedImage *bufferedimage, int_t texture)
{
	if (bufferedimage == nullptr)
		bufferedimage = missingTextureImage.get();

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, blurTexture ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, blurTexture ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clampTexture ? GL_CLAMP : GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clampTexture ? GL_CLAMP : GL_REPEAT);

	int_t width = bufferedimage->getWidth();
	int_t height = bufferedimage->getHeight();
	std::vector<unsigned char> pixels(width * height * 4);
	const unsigned char *raw = bufferedimage->getRawPixels();
	for (int_t i = 0; i < width * height; i++)
	{
		int_t r = raw[i * 4 + 0] & 0xff;
		int_t g = raw[i * 4 + 1] & 0xff;
		int_t b = raw[i * 4 + 2] & 0xff;
		int_t a = raw[i * 4 + 3] & 0xff;
		if (options != nullptr && options->anaglyph)
		{
			int_t nr = (r * 30 + g * 59 + b * 11) / 100;
			int_t ng = (r * 30 + g * 70) / 100;
			int_t nb = (r * 30 + b * 70) / 100;
			r = nr;
			g = ng;
			b = nb;
		}
		pixels[i * 4 + 0] = (unsigned char)r;
		pixels[i * 4 + 1] = (unsigned char)g;
		pixels[i * 4 + 2] = (unsigned char)b;
		pixels[i * 4 + 3] = (unsigned char)a;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}

void RenderEngine::func_28150_a(const std::vector<int_t> &ai, int_t width, int_t height, int_t texture)
{
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, blurTexture ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, blurTexture ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clampTexture ? GL_CLAMP : GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clampTexture ? GL_CLAMP : GL_REPEAT);

	std::vector<unsigned char> pixels(width * height * 4);
	for (int_t i = 0; i < (int_t)ai.size(); i++)
	{
		int_t a = (ai[i] >> 24) & 0xff;
		int_t r = (ai[i] >> 16) & 0xff;
		int_t g = (ai[i] >> 8) & 0xff;
		int_t b = ai[i] & 0xff;
		if (options != nullptr && options->anaglyph)
		{
			int_t nr = (r * 30 + g * 59 + b * 11) / 100;
			int_t ng = (r * 30 + g * 70) / 100;
			int_t nb = (r * 30 + b * 70) / 100;
			r = nr;
			g = ng;
			b = nb;
		}
		pixels[i * 4 + 0] = (unsigned char)r;
		pixels[i * 4 + 1] = (unsigned char)g;
		pixels[i * 4 + 2] = (unsigned char)b;
		pixels[i * 4 + 3] = (unsigned char)a;
	}
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
}

void RenderEngine::deleteTexture(int_t i)
{
	textureNameToImageMap.erase(i);
	GLuint texture = (GLuint)i;
	glDeleteTextures(1, &texture);
}

int_t RenderEngine::getTextureForDownloadableImage(const std::string &s, const std::string &fallback)
{
	ThreadDownloadImageData *threaddownloadimagedata = nullptr;
	auto it = urlToImageDataMap.find(s);
	if (it != urlToImageDataMap.end())
		threaddownloadimagedata = it->second;
	if (threaddownloadimagedata != nullptr && threaddownloadimagedata->image != nullptr && !threaddownloadimagedata->textureSetupComplete)
	{
		if (threaddownloadimagedata->textureName < 0)
			threaddownloadimagedata->textureName = allocateAndSetupTexture(threaddownloadimagedata->image);
		else
			setupTexture(threaddownloadimagedata->image, threaddownloadimagedata->textureName);
		threaddownloadimagedata->textureSetupComplete = true;
	}
	if (threaddownloadimagedata == nullptr || threaddownloadimagedata->textureName < 0)
		return fallback.empty() ? -1 : getTexture(fallback);
	return threaddownloadimagedata->textureName;
}

ThreadDownloadImageData *RenderEngine::obtainImageData(const std::string &s, ImageBuffer *imagebuffer)
{
	auto it = urlToImageDataMap.find(s);
	if (it == urlToImageDataMap.end())
	{
		ThreadDownloadImageData *data = new ThreadDownloadImageData(s, imagebuffer);
		urlToImageDataMap[s] = data;
		return data;
	}
	it->second->referenceCount++;
	return it->second;
}

void RenderEngine::releaseImageData(const std::string &s)
{
	auto it = urlToImageDataMap.find(s);
	if (it == urlToImageDataMap.end())
		return;
	ThreadDownloadImageData *data = it->second;
	data->referenceCount--;
	if (data->referenceCount == 0)
	{
		if (data->textureName >= 0)
			deleteTexture(data->textureName);
		delete data;
		urlToImageDataMap.erase(it);
	}
}

void RenderEngine::registerTextureFX(TextureFX *texturefx)
{
	textureList.push_back(texturefx);
	texturefx->onTick();
}

void RenderEngine::updateDynamicTextures()
{
	for (TextureFX *texturefx : textureList)
	{
		texturefx->anaglyphEnabled = options != nullptr && options->anaglyph;
		texturefx->onTick();
		texturefx->bindImage(this);
		glTexSubImage2D(GL_TEXTURE_2D, 0, (texturefx->iconIndex % 16) * 16, (texturefx->iconIndex / 16) * 16,
			16, 16, GL_RGBA, GL_UNSIGNED_BYTE, texturefx->imageData);
	}
}

void RenderEngine::refreshTextures()
{
	for (auto &entry : textureNameToImageMap)
		setupTexture(entry.second.get(), entry.first);

	TexturePackBase *texturepackbase = texturePack != nullptr ? texturePack->getSelectedTexturePack() : nullptr;
	for (auto &entry : textureMap)
	{
		try
		{
			std::unique_ptr<BufferedImage> image;
			const std::string &s = entry.first;
			if (s.rfind("##", 0) == 0)
			{
				image = unwrapImageByColumns(readTextureImage(texturepackbase->getResourceAsStream(s.substr(2))).get());
			}
			else if (s.rfind("%clamp%", 0) == 0)
			{
				clampTexture = true;
				image = readTextureImage(texturepackbase->getResourceAsStream(s.substr(7)));
			}
			else if (s.rfind("%blur%", 0) == 0)
			{
				blurTexture = true;
				image = readTextureImage(texturepackbase->getResourceAsStream(s.substr(6)));
			}
			else if (texturepackbase != nullptr)
			{
				image = readTextureImage(texturepackbase->getResourceAsStream(s));
			}
			// flags must survive until setupTexture() consumes them; reset only afterwards
			setupTexture(image ? image.get() : missingTextureImage.get(), entry.second);
			blurTexture = false;
			clampTexture = false;
		}
		catch (...)
		{
			setupTexture(missingTextureImage.get(), entry.second);
			blurTexture = false;
			clampTexture = false;
		}
	}
	field_28151_c.clear();
}

void RenderEngine::bindTexture(int_t i)
{
	if (i >= 0)
		glBindTexture(GL_TEXTURE_2D, i);
}

std::unique_ptr<BufferedImage> RenderEngine::readTextureImage(std::istream *inputstream)
{
	if (inputstream == nullptr || !*inputstream)
		return nullptr;
	std::unique_ptr<std::istream> guard(inputstream);
	return std::unique_ptr<BufferedImage>(new BufferedImage(BufferedImage::ImageIO_read(*inputstream)));
}

std::unique_ptr<BufferedImage> RenderEngine::createMissingTexture()
{
	std::unique_ptr<BufferedImage> image(new BufferedImage(64, 64));
	std::vector<unsigned char> pixels(64 * 64 * 4);
	for (int_t y = 0; y < 64; y++)
	{
		for (int_t x = 0; x < 64; x++)
		{
			bool black = ((x / 8) + (y / 8)) % 2 == 0;
			int_t i = (y * 64 + x) * 4;
			pixels[i + 0] = black ? 0 : 255;
			pixels[i + 1] = black ? 0 : 255;
			pixels[i + 2] = black ? 0 : 255;
			pixels[i + 3] = 255;
		}
	}
	image->setRGB(0, 0, 64, 64, pixels.data());
	return image;
}

int_t RenderEngine::averageColor(int_t i, int_t j)
{
	int_t k = (i & 0xff000000) >> 24 & 0xff;
	int_t l = (j & 0xff000000) >> 24 & 0xff;
	return ((k + l >> 1) << 24) + ((i & 0xfefefe) + (j & 0xfefefe) >> 1);
}

int_t RenderEngine::weightedAverageColor(int_t i, int_t j)
{
	int_t k = (i & 0xff000000) >> 24 & 0xff;
	int_t l = (j & 0xff000000) >> 24 & 0xff;
	int_t c = 255;
	if (k + l == 0)
	{
		k = 1;
		l = 1;
		c = 0;
	}
	int_t i1 = (i >> 16 & 0xff) * k;
	int_t j1 = (i >> 8 & 0xff) * k;
	int_t k1 = (i & 0xff) * k;
	int_t l1 = (j >> 16 & 0xff) * l;
	int_t i2 = (j >> 8 & 0xff) * l;
	int_t j2 = (j & 0xff) * l;
	int_t k2 = (i1 + l1) / (k + l);
	int_t l2 = (j1 + i2) / (k + l);
	int_t i3 = (k1 + j2) / (k + l);
	return c << 24 | k2 << 16 | l2 << 8 | i3;
}
