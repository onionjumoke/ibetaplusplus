#pragma once

#include "NBTBase.h"

// net.minecraft.src.NBTTagDouble
class NBTTagDouble : public NBTBase
{
public:
	NBTTagDouble();
	NBTTagDouble(double d);

	void writeTagContents(std::ostream &dataoutput) override;
	void readTagContents(std::istream &datainput) override;
	byte_t getType() override;
	jstring toString() override;

	double doubleValue = 0.0;
};
