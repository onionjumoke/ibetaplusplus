#pragma once

#include <regex>
#include <string>

// net.minecraft.src.ChunkFilePattern
// Matches chunk save-file names: c.<x36>.<z36>.dat
class ChunkFilePattern
{
private:
	ChunkFilePattern() = delete;

public:
	static bool accept(const std::string &filename)
	{
		return std::regex_match(filename, field_22189_a);
	}

	// c\.(-?[0-9a-z]+)\.(-?[0-9a-z]+)\.dat
	static const std::regex field_22189_a;
};
