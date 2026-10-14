#include "NBTBase.h"

#include <istream>
#include <ostream>

#include "java/IOUtil.h"

#include "NBTTagEnd.h"
#include "NBTTagByte.h"
#include "NBTTagShort.h"
#include "NBTTagInt.h"
#include "NBTTagLong.h"
#include "NBTTagFloat.h"
#include "NBTTagDouble.h"
#include "NBTTagByteArray.h"
#include "NBTTagString.h"
#include "NBTTagList.h"
#include "NBTTagCompound.h"

NBTBase::NBTBase()
{
}

jstring NBTBase::toString()
{
	return u"";
}

jstring NBTBase::getKey()
{
	if (key.empty())
		return u"";
	else
		return key;
}

NBTBase *NBTBase::setKey(const jstring &s)
{
	key = s;
	return this;
}

NBTBase *NBTBase::readTag(std::istream &datainput)
{
	byte_t byte0 = IOUtil::readByte(datainput);
	if (byte0 == 0)
	{
		return new NBTTagEnd();
	}
	else
	{
		NBTBase *nbtbase = createTagOfType(byte0);
		nbtbase->key = IOUtil::readUTF(datainput);
		nbtbase->readTagContents(datainput);
		return nbtbase;
	}
}

void NBTBase::writeTag(NBTBase *nbtbase, std::ostream &dataoutput)
{
	IOUtil::writeByte(dataoutput, nbtbase->getType());
	if (nbtbase->getType() == 0)
	{
		return;
	}
	else
	{
		IOUtil::writeUTF(dataoutput, nbtbase->getKey());
		nbtbase->writeTagContents(dataoutput);
		return;
	}
}

NBTBase *NBTBase::createTagOfType(byte_t byte0)
{
	switch (byte0)
	{
	case 0:
		return new NBTTagEnd();
	case 1:
		return new NBTTagByte();
	case 2:
		return new NBTTagShort();
	case 3:
		return new NBTTagInt();
	case 4:
		return new NBTTagLong();
	case 5:
		return new NBTTagFloat();
	case 6:
		return new NBTTagDouble();
	case 7:
		return new NBTTagByteArray();
	case 8:
		return new NBTTagString();
	case 9:
		return new NBTTagList();
	case 10:
		return new NBTTagCompound();
	}
	return nullptr;
}

jstring NBTBase::getTagName(byte_t byte0)
{
	switch (byte0)
	{
	case 0:
		return u"TAG_End";
	case 1:
		return u"TAG_Byte";
	case 2:
		return u"TAG_Short";
	case 3:
		return u"TAG_Int";
	case 4:
		return u"TAG_Long";
	case 5:
		return u"TAG_Float";
	case 6:
		return u"TAG_Double";
	case 7:
		return u"TAG_Byte_Array";
	case 8:
		return u"TAG_String";
	case 9:
		return u"TAG_List";
	case 10:
		return u"TAG_Compound";
	}
	return u"UNKNOWN";
}
