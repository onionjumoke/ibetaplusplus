#include "NBTTagList.h"

#include <istream>
#include <ostream>

#include "java/IOUtil.h"

NBTTagList::NBTTagList()
{
}

void NBTTagList::writeTagContents(std::ostream &dataoutput)
{
	if (tagList.size() > 0)
		tagType = tagList[0]->getType();
	else
		tagType = 1;
	IOUtil::writeByte(dataoutput, tagType);
	IOUtil::writeInt(dataoutput, (int_t)tagList.size());
	for (int_t i = 0; i < (int_t)tagList.size(); i++)
		tagList[i]->writeTagContents(dataoutput);
}

void NBTTagList::readTagContents(std::istream &datainput)
{
	tagType = IOUtil::readByte(datainput);
	int_t i = IOUtil::readInt(datainput);
	tagList.clear();
	for (int_t j = 0; j < i; j++)
	{
		NBTBase *nbtbase = NBTBase::createTagOfType(tagType);
		nbtbase->readTagContents(datainput);
		tagList.push_back(nbtbase);
	}
}

byte_t NBTTagList::getType()
{
	return 9;
}

jstring NBTTagList::toString()
{
	return String::toString((int_t)tagList.size()) + u" entries of type " + NBTBase::getTagName(tagType);
}

void NBTTagList::setTag(NBTBase *nbtbase)
{
	tagType = nbtbase->getType();
	tagList.push_back(nbtbase);
}

NBTBase *NBTTagList::tagAt(int_t i)
{
	return tagList[i];
}

int_t NBTTagList::tagCount()
{
	return (int_t)tagList.size();
}
