#include "ThreadDownloadImageData.h"

#include "ThreadDownloadImage.h"

ThreadDownloadImageData::ThreadDownloadImageData(const std::string &s, ImageBuffer *imagebuffer)
	: image(nullptr)
	, referenceCount(1)
	, textureName(-1)
	, textureSetupComplete(false)
{
	(new ThreadDownloadImage(this, s, imagebuffer))->start();
}
