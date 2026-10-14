#pragma once

#include <map>
#include <string>
#include <vector>

// net.minecraft.src.StringTranslate
class StringTranslate
{
private:
	StringTranslate();

public:
	static StringTranslate *getInstance();

	std::string translateKey(const std::string &s);
	std::string translateKeyFormat(const std::string &s, const std::vector<std::string> &args);
	std::string translateKeyFormat(const std::string &s, const std::string &arg);
	std::string translateKeyFormat(const std::string &s, const char *arg);
	std::string translateNamedKey(const std::string &s);

private:
	void loadLanguageFile(const std::string &path);
	static std::string trim(const std::string &s);

	static StringTranslate *instance;
	std::map<std::string, std::string> translateTable;
};
