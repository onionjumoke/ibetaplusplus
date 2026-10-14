#pragma once

#include <string>

// net.minecraft.src.ChatAllowedCharacters
class ChatAllowedCharacters
{
public:
	static const std::string allowedCharacters;
	static const char allowedCharactersArray[15];

private:
	static std::string getAllowedCharacters();
};
