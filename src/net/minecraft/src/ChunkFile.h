#pragma once

#include <regex>
#include <string>
#include "java/Type.h"
#include "java/File.h"
#include "ChunkFilePattern.h"

// net.minecraft.src.ChunkFile
// Wraps a chunk save file; parses x/z coords from the filename.
class ChunkFile
{
public:
	explicit ChunkFile(File *file)
		: field_22326_a(file)
	{
		std::string name = String::toUTF8(file->getName());
		std::smatch matcher;
		if (std::regex_match(name, matcher, ChunkFilePattern::field_22189_a))
		{
			field_22325_b = std::stoi(matcher[1].str(), nullptr, 36);
			field_22327_c = std::stoi(matcher[2].str(), nullptr, 36);
		}
		else
		{
			field_22325_b = 0;
			field_22327_c = 0;
		}
	}

	// Compare by region (>> 5) so nearby chunks sort together.
	int_t compareTo(const ChunkFile &other) const
	{
		int_t i = field_22325_b >> 5;
		int_t j = other.field_22325_b >> 5;
		if (i == j)
		{
			int_t k = field_22327_c >> 5;
			int_t l = other.field_22327_c >> 5;
			return k - l;
		}
		return i - j;
	}

	bool operator<(const ChunkFile &other) const { return compareTo(other) < 0; }

	File *getFile() const          { return field_22326_a; }   // func_22324_a
	int_t getChunkX() const        { return field_22325_b; }   // func_22323_b
	int_t getChunkZ() const        { return field_22327_c; }   // func_22321_c

private:
	File *field_22326_a;
	int_t field_22325_b;
	int_t field_22327_c;
};
