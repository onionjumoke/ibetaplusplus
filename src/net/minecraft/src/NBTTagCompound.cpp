#include "NBTTagCompound.h"

#include <istream>
#include <ostream>

#include "java/IOUtil.h"

#include "NBTTagByte.h"
#include "NBTTagShort.h"
#include "NBTTagInt.h"
#include "NBTTagLong.h"
#include "NBTTagFloat.h"
#include "NBTTagDouble.h"
#include "NBTTagString.h"
#include "NBTTagByteArray.h"
#include "NBTTagList.h"

NBTTagCompound::NBTTagCompound()
{
}

void NBTTagCompound::writeTagContents(std::ostream &dataoutput)
{
	for (auto &entry : tagMap)
		NBTBase::writeTag(entry.second, dataoutput);
	IOUtil::writeByte(dataoutput, 0);
}

void NBTTagCompound::readTagContents(std::istream &datainput)
{
	tagMap.clear();
	NBTBase *nbtbase;
	while ((nbtbase = NBTBase::readTag(datainput))->getType() != 0)
		tagMap[nbtbase->getKey()] = nbtbase;
}

std::vector<NBTBase *> NBTTagCompound::getTags()
{
	std::vector<NBTBase *> values;
	for (auto &entry : tagMap)
		values.push_back(entry.second);
	return values;
}

byte_t NBTTagCompound::getType()
{
	return 10;
}

void NBTTagCompound::setTag(const jstring &s, NBTBase *nbtbase)
{
	tagMap[s] = nbtbase->setKey(s);
}

void NBTTagCompound::setByte(const jstring &s, byte_t byte0)
{
	tagMap[s] = (new NBTTagByte(byte0))->setKey(s);
}

void NBTTagCompound::setShort(const jstring &s, short_t word0)
{
	tagMap[s] = (new NBTTagShort(word0))->setKey(s);
}

void NBTTagCompound::setInteger(const jstring &s, int_t i)
{
	tagMap[s] = (new NBTTagInt(i))->setKey(s);
}

void NBTTagCompound::setLong(const jstring &s, long_t l)
{
	tagMap[s] = (new NBTTagLong(l))->setKey(s);
}

void NBTTagCompound::setFloat(const jstring &s, float f)
{
	tagMap[s] = (new NBTTagFloat(f))->setKey(s);
}

void NBTTagCompound::setDouble(const jstring &s, double d)
{
	tagMap[s] = (new NBTTagDouble(d))->setKey(s);
}

void NBTTagCompound::setString(const jstring &s, const jstring &s1)
{
	tagMap[s] = (new NBTTagString(s1))->setKey(s);
}

void NBTTagCompound::setByteArray(const jstring &s, const std::vector<byte_t> &abyte0)
{
	tagMap[s] = (new NBTTagByteArray(abyte0))->setKey(s);
}

void NBTTagCompound::setCompoundTag(const jstring &s, NBTTagCompound *nbttagcompound)
{
	tagMap[s] = nbttagcompound->setKey(s);
}

void NBTTagCompound::setBoolean(const jstring &s, bool flag)
{
	setByte(s, (byte_t)(flag ? 1 : 0));
}

bool NBTTagCompound::hasKey(const jstring &s)
{
	return tagMap.count(s) != 0;
}

byte_t NBTTagCompound::getByte(const jstring &s)
{
	if (tagMap.count(s) == 0)
		return 0;
	else
		return static_cast<NBTTagByte *>(tagMap[s])->byteValue;
}

short_t NBTTagCompound::getShort(const jstring &s)
{
	if (tagMap.count(s) == 0)
		return 0;
	else
		return static_cast<NBTTagShort *>(tagMap[s])->shortValue;
}

int_t NBTTagCompound::getInteger(const jstring &s)
{
	if (tagMap.count(s) == 0)
		return 0;
	else
		return static_cast<NBTTagInt *>(tagMap[s])->intValue;
}

long_t NBTTagCompound::getLong(const jstring &s)
{
	if (tagMap.count(s) == 0)
		return 0LL;
	else
		return static_cast<NBTTagLong *>(tagMap[s])->longValue;
}

float NBTTagCompound::getFloat(const jstring &s)
{
	if (tagMap.count(s) == 0)
		return 0.0f;
	else
		return static_cast<NBTTagFloat *>(tagMap[s])->floatValue;
}

double NBTTagCompound::getDouble(const jstring &s)
{
	if (tagMap.count(s) == 0)
		return 0.0;
	else
		return static_cast<NBTTagDouble *>(tagMap[s])->doubleValue;
}

jstring NBTTagCompound::getString(const jstring &s)
{
	if (tagMap.count(s) == 0)
		return u"";
	else
		return static_cast<NBTTagString *>(tagMap[s])->stringValue;
}

std::vector<byte_t> NBTTagCompound::getByteArray(const jstring &s)
{
	if (tagMap.count(s) == 0)
		return std::vector<byte_t>();
	else
		return static_cast<NBTTagByteArray *>(tagMap[s])->byteArray;
}

NBTTagCompound *NBTTagCompound::getCompoundTag(const jstring &s)
{
	if (tagMap.count(s) == 0)
		return new NBTTagCompound();
	else
		return static_cast<NBTTagCompound *>(tagMap[s]);
}

NBTTagList *NBTTagCompound::getTagList(const jstring &s)
{
	if (tagMap.count(s) == 0)
		return new NBTTagList();
	else
		return static_cast<NBTTagList *>(tagMap[s]);
}

bool NBTTagCompound::getBoolean(const jstring &s)
{
	return getByte(s) != 0;
}

jstring NBTTagCompound::toString()
{
	return String::toString((int_t)tagMap.size()) + u" entries";
}
