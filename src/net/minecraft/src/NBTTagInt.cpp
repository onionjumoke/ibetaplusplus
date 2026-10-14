#include "NBTTagInt.h"

#include <istream>
#include <ostream>

#include "java/IOUtil.h"

NBTTagInt::NBTTagInt()
{
}

NBTTagInt::NBTTagInt(int_t i)
{
	intValue = i;
}

void NBTTagInt::writeTagContents(std::ostream &dataoutput)
{
	IOUtil::writeInt(dataoutput, intValue);
}

void NBTTagInt::readTagContents(std::istream &datainput)
{
	intValue = IOUtil::readInt(datainput);
}

byte_t NBTTagInt::getType()
{
	return 3;
}

jstring NBTTagInt::toString()
{
	return String::toString(intValue);
}
