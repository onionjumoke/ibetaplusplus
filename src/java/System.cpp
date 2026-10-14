#include "System.h"

#include <chrono>
#include <SDL.h>

namespace System
{

long_t currentTimeMillis()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

long_t nanoTime()
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

bool openURL(const std::string &url)
{
	return SDL_OpenURL(url.c_str()) == 0;
}

}
