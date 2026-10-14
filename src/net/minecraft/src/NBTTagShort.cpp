#include "NBTTagShort.h"

#include <istream>
#include <ostream>

#include "java/IOUtil.h"

NBTTagShort::NBTTagShort()
{
}

NBTTagShort::NBTTagShort(short_t word0)
{
	shortValue = word0;
}

void NBTTagShort::writeTagContents(std::ostream &dataoutput)
{
	IOUtil::writeShort(dataoutput, shortValue);
}

void NBTTagShort::readTagContents(std::istream &datainput)
{
	shortValue = IOUtil::readShort(datainput);
}

byte_t NBTTagShort::getType()
{
	return 2;
}

jstring NBTTagShort::toString()
{
	return String::toString((int_t)shortValue);
}
