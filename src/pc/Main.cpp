#define SDL_MAIN_HANDLED
#include "SDL.h"

#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <iostream>

#include "client/Minecraft.h"
#include "net/minecraft/src/GameResources.h"

#include "external/SDLException.h"

#include "lwjgl/GLContext.h"

#ifdef MC_TERMINAL
#include "terminal/Terminal.h"
#endif

namespace
{
std::string trimUsername(std::string value)
{
	if (value.size() >= 3 &&
		static_cast<unsigned char>(value[0]) == 0xef &&
		static_cast<unsigned char>(value[1]) == 0xbb &&
		static_cast<unsigned char>(value[2]) == 0xbf)
	{
		value.erase(0, 3);
	}

	const auto first = value.find_first_not_of(" \t\r\n");
	if (first == std::string::npos)
		return "";
	const auto last = value.find_last_not_of(" \t\r\n");
	return value.substr(first, last - first + 1);
}

std::string loadUsername()
{
	const std::filesystem::path path =
		std::filesystem::path(GameResources::getExeDir()) / "username.txt";
	std::ifstream input(path);
	std::string username;
	if (input && std::getline(input, username))
		username = trimUsername(username);

	if (!username.empty())
		return username;

	username = "Player";
	std::ofstream output(path, std::ios::trunc);
	if (output)
		output << username << '\n';
	return username;
}
}

int main(int argc, char *argv[])
{
#ifdef MC_TERMINAL
	lwjgl::terminal::earlyInit();
#endif
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) < 0)
		throw SDLException();
	// Audio is optional: machines without a sound device (servers, headless
	// SSH, containers) must still boot. SoundManager handles a missing device
	// gracefully at runtime.
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
		std::cerr << "audio unavailable, continuing silently: " << SDL_GetError() << std::endl;
	lwjgl::GLContext::instantiate();

	jstring username = String::fromUTF8(loadUsername());
	if (argc >= 2 && std::strlen(argv[1]) > 0)
		username = String::fromUTF8(argv[1]);

	jstring auth = u"-";
	if (argc >= 3 && std::strlen(argv[2]) > 1)
		auth = String::fromUTF8(argv[2]);

	Minecraft::start(&username, &auth);

#ifdef MC_TERMINAL
	lwjgl::terminal::shutdown();
#endif
	return 0;
}
