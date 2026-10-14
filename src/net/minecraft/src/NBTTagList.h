#pragma once

#include <vector>

#include "NBTBase.h"

// net.minecraft.src.NBTTagList
class NBTTagList : public NBTBase
{
public:
	NBTTagList();

	void writeTagContents(std::ostream &dataoutput) override;
	void readTagContents(std::istream &datainput) override;
	byte_t getType() override;
	jstring toString() override;

	void setTag(NBTBase *nbtbase);
	void appendTag(NBTBase *nbtbase) { setTag(nbtbase); }
	NBTBase *tagAt(int_t i);
	int_t tagCount();

private:
	std::vector<NBTBase *> tagList;
	byte_t tagType = 0;
};
