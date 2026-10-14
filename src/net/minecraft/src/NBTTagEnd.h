#pragma once

#include "NBTBase.h"

// net.minecraft.src.NBTTagEnd
class NBTTagEnd : public NBTBase
{
public:
	NBTTagEnd();

	void readTagContents(std::istream &datainput) override;
	void writeTagContents(std::ostream &dataoutput) override;
	byte_t getType() override;
	jstring toString() override;
};
