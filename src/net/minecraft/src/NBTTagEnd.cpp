#include "NBTTagEnd.h"

NBTTagEnd::NBTTagEnd()
{
}

void NBTTagEnd::readTagContents(std::istream &datainput)
{
}

void NBTTagEnd::writeTagContents(std::ostream &dataoutput)
{
}

byte_t NBTTagEnd::getType()
{
	return 0;
}

jstring NBTTagEnd::toString()
{
	return u"END";
}
