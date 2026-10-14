#include "NBTTagByte.h"

#include <istream>
#include <ostream>

#include "java/IOUtil.h"

NBTTagByte::NBTTagByte()
{
}

NBTTagByte::NBTTagByte(byte_t byte0)
{
	byteValue = byte0;
}

void NBTTagByte::writeTagContents(std::ostream &dataoutput)
{
	IOUtil::writeByte(dataoutput, byteValue);
}

void NBTTagByte::readTagContents(std::istream &datainput)
{
	byteValue = IOUtil::readByte(datainput);
}

byte_t NBTTagByte::getType()
{
	return 1;
}

jstring NBTTagByte::toString()
{
	return String::toString((int_t)byteValue);
}
