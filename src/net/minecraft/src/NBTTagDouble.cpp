#include "NBTTagDouble.h"

#include <istream>
#include <ostream>

#include "java/IOUtil.h"

NBTTagDouble::NBTTagDouble()
{
}

NBTTagDouble::NBTTagDouble(double d)
{
	doubleValue = d;
}

void NBTTagDouble::writeTagContents(std::ostream &dataoutput)
{
	IOUtil::writeDouble(dataoutput, doubleValue);
}

void NBTTagDouble::readTagContents(std::istream &datainput)
{
	doubleValue = IOUtil::readDouble(datainput);
}

byte_t NBTTagDouble::getType()
{
	return 6;
}

jstring NBTTagDouble::toString()
{
	return String::toString(doubleValue);
}
