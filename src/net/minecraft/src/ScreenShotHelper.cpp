#include "ScreenShotHelper.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <vector>

#include "pc/OpenGL.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace fs = std::filesystem;

std::string ScreenShotHelper::saveScreenshot(const std::string &file, int_t i, int_t j)
{
	try
	{
		fs::path screenshots = fs::path(file) / "screenshots";
		fs::create_directories(screenshots);

		std::vector<unsigned char> pixelData(i * j * 3);
		std::vector<unsigned char> imageData(i * j * 3);

		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glReadPixels(0, 0, i, j, GL_RGB, GL_UNSIGNED_BYTE, pixelData.data());

		std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::tm tmValue{};
#ifdef _WIN32
		localtime_s(&tmValue, &now);
#else
		localtime_r(&now, &tmValue);
#endif

		std::ostringstream nameStream;
		nameStream << std::put_time(&tmValue, "%Y-%m-%d_%H.%M.%S");
		std::string baseName = nameStream.str();

		fs::path output;
		for (int_t k = 1;; k++)
		{
			std::string suffix = k == 1 ? "" : "_" + std::to_string(k);
			output = screenshots / (baseName + suffix + ".png");
			if (!fs::exists(output))
				break;
		}

		for (int_t x = 0; x < i; x++)
		{
			for (int_t y = 0; y < j; y++)
			{
				int_t source = (x + (j - y - 1) * i) * 3;
				int_t dest = (x + y * i) * 3;
				imageData[dest + 0] = pixelData[source + 0];
				imageData[dest + 1] = pixelData[source + 1];
				imageData[dest + 2] = pixelData[source + 2];
			}
		}

		if (!stbi_write_png(output.string().c_str(), i, j, 3, imageData.data(), i * 3))
			return "Failed to save: could not write png";

		return "Saved screenshot as " + output.filename().string();
	}
	catch (const std::exception &exception)
	{
		return std::string("Failed to save: ") + exception.what();
	}
}
