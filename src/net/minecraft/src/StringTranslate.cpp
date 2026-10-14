#include "StringTranslate.h"
#include "GameResources.h"

#include <fstream>
#include <sstream>

StringTranslate *StringTranslate::instance = nullptr;

StringTranslate::StringTranslate()
{
	loadLanguageFile("/lang/en_US.lang");
	loadLanguageFile("/lang/stats_US.lang");
}

StringTranslate *StringTranslate::getInstance()
{
	if (instance == nullptr)
		instance = new StringTranslate();
	return instance;
}

std::string StringTranslate::translateKey(const std::string &s)
{
	auto it = translateTable.find(s);
	return it != translateTable.end() ? it->second : s;
}

std::string StringTranslate::translateKeyFormat(const std::string &s, const std::vector<std::string> &args)
{
	std::string result = translateKey(s);
	// Java used String.format, which accepts both positional (%1$s) and
	// plain (%s) specifiers. Only handling %1$s left raw "%s" on screen
	// (e.g. disconnect reasons).
	for (std::size_t i = 0; i < args.size(); i++)
	{
		std::string token = "%" + std::to_string(i + 1) + "$s";
		for (std::size_t pos = result.find(token); pos != std::string::npos;
		     pos = result.find(token, pos))
			result.replace(pos, token.length(), args[i]);
	}
	std::size_t ai = 0;
	for (std::size_t pos = result.find("%s"); pos != std::string::npos && ai < args.size();
	     pos = result.find("%s", pos))
	{
		result.replace(pos, 2, args[ai]);
		pos += args[ai].length();
		ai++;
	}
	std::size_t di = 0;
	for (std::size_t pos = result.find("%d"); pos != std::string::npos && di < args.size();
	     pos = result.find("%d", pos))
	{
		result.replace(pos, 2, args[di]);
		pos += args[di].length();
		di++;
	}
	return result;
}

std::string StringTranslate::translateKeyFormat(const std::string &s, const std::string &arg)
{
	return translateKeyFormat(s, std::vector<std::string>{ arg });
}

std::string StringTranslate::translateKeyFormat(const std::string &s, const char *arg)
{
	return translateKeyFormat(s, std::vector<std::string>{ arg != nullptr ? arg : "" });
}

std::string StringTranslate::translateNamedKey(const std::string &s)
{
	auto it = translateTable.find(s + ".name");
	return it != translateTable.end() ? it->second : "";
}

void StringTranslate::loadLanguageFile(const std::string &path)
{
	std::string resolved = GameResources::resolve(path);
	if (resolved.empty())
		return;

	std::ifstream input(resolved);

	std::string line;
	while (std::getline(input, line))
	{
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		if (line.empty() || line[0] == '#')
			continue;
		std::size_t equals = line.find('=');
		if (equals == std::string::npos)
			continue;
		translateTable[trim(line.substr(0, equals))] = trim(line.substr(equals + 1));
	}
}

std::string StringTranslate::trim(const std::string &s)
{
	std::size_t first = s.find_first_not_of(" \t\r\n");
	if (first == std::string::npos)
		return "";
	std::size_t last = s.find_last_not_of(" \t\r\n");
	return s.substr(first, last - first + 1);
}
