#pragma once

#include <regex>
#include <string>

// net.minecraft.src.ChunkFolderPattern
// Matches 1- or 2-char base-36 folder names used by the chunk save layout.
class ChunkFolderPattern
{
private:
	ChunkFolderPattern() = delete;

public:
	static bool accept(const std::string &dirName)
	{
		return std::regex_match(dirName, field_22392_a);
	}

	// [0-9a-z]|([0-9a-z][0-9a-z])
	static const std::regex field_22392_a;
};
