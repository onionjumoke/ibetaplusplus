#include "NBTTagString.h"

#include <istream>
#include <ostream>

#include "java/IOUtil.h"

NBTTagString::NBTTagString()
{
}

NBTTagString::NBTTagString(const jstring &s)
{
	stringValue = s;
}

void NBTTagString::writeTagContents(std::ostream &dataoutput)
{
	IOUtil::writeUTF(dataoutput, stringValue);
}

void NBTTagString::readTagContents(std::istream &datainput)
{
	stringValue = IOUtil::readUTF(datainput);
}

byte_t NBTTagString::getType()
{
	return 8;
}

jstring NBTTagString::toString()
{
	return stringValue;
}
