#pragma once

#include "NBTBase.h"

// net.minecraft.src.NBTTagByte
class NBTTagByte : public NBTBase
{
public:
	NBTTagByte();
	NBTTagByte(byte_t byte0);

	void writeTagContents(std::ostream &dataoutput) override;
	void readTagContents(std::istream &datainput) override;
	byte_t getType() override;
	jstring toString() override;

	byte_t byteValue = 0;
};
