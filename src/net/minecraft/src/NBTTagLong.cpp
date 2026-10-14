#include "NBTTagLong.h"

#include <istream>
#include <ostream>

#include "java/IOUtil.h"

NBTTagLong::NBTTagLong()
{
}

NBTTagLong::NBTTagLong(long_t l)
{
	longValue = l;
}

void NBTTagLong::writeTagContents(std::ostream &dataoutput)
{
	IOUtil::writeLong(dataoutput, longValue);
}

void NBTTagLong::readTagContents(std::istream &datainput)
{
	longValue = IOUtil::readLong(datainput);
}

byte_t NBTTagLong::getType()
{
	return 4;
}

jstring NBTTagLong::toString()
{
	return String::toString(longValue);
}
