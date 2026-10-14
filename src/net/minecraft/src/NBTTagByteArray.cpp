#include "NBTTagByteArray.h"

#include <istream>
#include <ostream>

#include "java/IOUtil.h"

NBTTagByteArray::NBTTagByteArray()
{
}

NBTTagByteArray::NBTTagByteArray(const std::vector<byte_t> &abyte0)
{
	byteArray = abyte0;
}

void NBTTagByteArray::writeTagContents(std::ostream &dataoutput)
{
	IOUtil::writeInt(dataoutput, (int_t)byteArray.size());
	if (!byteArray.empty())
		dataoutput.write(reinterpret_cast<const char *>(byteArray.data()), (std::streamsize)byteArray.size());
}

void NBTTagByteArray::readTagContents(std::istream &datainput)
{
	int_t i = IOUtil::readInt(datainput);
	byteArray.assign((size_t)i, 0);
	if (i > 0)
		datainput.read(reinterpret_cast<char *>(byteArray.data()), (std::streamsize)i);
}

byte_t NBTTagByteArray::getType()
{
	return 7;
}

jstring NBTTagByteArray::toString()
{
	return u"[" + String::toString((int_t)byteArray.size()) + u" bytes]";
}
