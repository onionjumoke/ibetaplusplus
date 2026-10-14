#pragma once

#include <string>

class BufferedImage;
class ImageBuffer;

// net.minecraft.src.ThreadDownloadImageData
class ThreadDownloadImageData
{
public:
	ThreadDownloadImageData(const std::string &s, ImageBuffer *imagebuffer);

	BufferedImage *image;
	int referenceCount;
	int textureName;
	bool textureSetupComplete;
};
