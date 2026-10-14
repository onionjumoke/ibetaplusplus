#include "NBTTagFloat.h"

#include <istream>
#include <ostream>

#include "java/IOUtil.h"

NBTTagFloat::NBTTagFloat()
{
}

NBTTagFloat::NBTTagFloat(float f)
{
	floatValue = f;
}

void NBTTagFloat::writeTagContents(std::ostream &dataoutput)
{
	IOUtil::writeFloat(dataoutput, floatValue);
}

void NBTTagFloat::readTagContents(std::istream &datainput)
{
	floatValue = IOUtil::readFloat(datainput);
}

byte_t NBTTagFloat::getType()
{
	return 5;
}

jstring NBTTagFloat::toString()
{
	return String::toString(floatValue);
}
