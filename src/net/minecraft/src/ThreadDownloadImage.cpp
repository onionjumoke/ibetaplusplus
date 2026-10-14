#include "ThreadDownloadImage.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "ImageBuffer.h"
#include "ThreadDownloadImageData.h"
#include "java/JavaNetwork.h"
#include "java/BufferedImage.h"

ThreadDownloadImage::ThreadDownloadImage(ThreadDownloadImageData *threaddownloadimagedata, const std::string &s, ImageBuffer *imagebuffer)
	: location(s)
	, buffer(imagebuffer)
	, imageData(threaddownloadimagedata)
{
}

ThreadDownloadImage::~ThreadDownloadImage()
{
	if (worker.joinable())
		worker.detach();
}

void ThreadDownloadImage::start()
{
	worker = std::thread(&ThreadDownloadImage::run, this);
	worker.detach();
}

void ThreadDownloadImage::run()
{
	try
	{
		std::vector<unsigned char> data;
		if (!JavaNetwork::readUrl(location, data))
			return;
		std::string bytes(reinterpret_cast<const char *>(data.data()), data.size());
		std::istringstream input(bytes, std::ios::binary);
		BufferedImage *image = new BufferedImage(BufferedImage::ImageIO_read(input));
		imageData->image = buffer == nullptr ? image : buffer->parseUserSkin(image);
		if (imageData->image != image)
			delete image;
	}
	catch (std::exception &exception)
	{
		std::cerr << exception.what() << std::endl;
	}
}
