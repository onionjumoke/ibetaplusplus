#pragma once

#include <iosfwd>

#include "java/Type.h"
#include "java/String.h"

// net.minecraft.src.NBTBase
class NBTBase
{
public:
	NBTBase();
	virtual ~NBTBase() = default;

	virtual void writeTagContents(std::ostream &dataoutput) = 0;
	virtual void readTagContents(std::istream &datainput) = 0;
	virtual byte_t getType() = 0;
	virtual jstring toString();

	jstring getKey();
	NBTBase *setKey(const jstring &s);

	static NBTBase *readTag(std::istream &datainput);
	static void writeTag(NBTBase *nbtbase, std::ostream &dataoutput);
	static NBTBase *createTagOfType(byte_t byte0);
	static jstring getTagName(byte_t byte0);

private:
	jstring key;
};
