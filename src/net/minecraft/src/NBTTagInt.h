#pragma once

#include "NBTBase.h"

// net.minecraft.src.NBTTagInt
class NBTTagInt : public NBTBase
{
public:
	NBTTagInt();
	NBTTagInt(int_t i);

	void writeTagContents(std::ostream &dataoutput) override;
	void readTagContents(std::istream &datainput) override;
	byte_t getType() override;
	jstring toString() override;

	int_t intValue = 0;
};
