#include "ChatAllowedCharacters.h"

#include "java/Resource.h"

#include <memory>

const std::string ChatAllowedCharacters::allowedCharacters = ChatAllowedCharacters::getAllowedCharacters();

const char ChatAllowedCharacters::allowedCharactersArray[15] = {
	'/', '\n', '\r', '\t', '\0', '\f', '`', '?', '*', '\\',
	'<', '>', '|', '"', ':'
};

std::string ChatAllowedCharacters::getAllowedCharacters()
{
	std::unique_ptr<std::istream> bufferedreader(Resource::getResource(u"/font.txt"));
	std::string result;

	std::string line;
	while (std::getline(*bufferedreader, line))
	{
		if (line.empty() || line[0] == '#')
			continue;
		if (line.back() == '\r')
			line.pop_back();
		result += line;
	}
	return result;
}
