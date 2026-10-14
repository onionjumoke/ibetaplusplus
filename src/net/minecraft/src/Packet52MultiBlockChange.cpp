#include "Packet52MultiBlockChange.h"

#include "NetHandler.h"

Packet52MultiBlockChange::Packet52MultiBlockChange() :
	xPosition(0),
	zPosition(0),
	size(0)
{
	isChunkDataPacket = true;
}

void Packet52MultiBlockChange::readPacketData(std::istream &is)
{
	xPosition = IOUtil::readInt(is);
	zPosition = IOUtil::readInt(is);
	short_t s = IOUtil::readShort(is);
	size = s & 0xffff;
	coordinateArray.resize(size);
	typeArray.resize(size);
	metadataArray.resize(size);

	for (int_t i = 0; i < size; i++)
	{
		coordinateArray[i] = IOUtil::readShort(is);
	}
	is.read(reinterpret_cast<char*>(typeArray.data()), size);
	is.read(reinterpret_cast<char*>(metadataArray.data()), size);
}

void Packet52MultiBlockChange::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, xPosition);
	IOUtil::writeInt(os, zPosition);
	IOUtil::writeShort(os, (short_t)size);
	for (int_t i = 0; i < size; i++)
	{
		IOUtil::writeShort(os, coordinateArray[i]);
	}
	os.write(reinterpret_cast<const char*>(typeArray.data()), size);
	os.write(reinterpret_cast<const char*>(metadataArray.data()), size);
}

void Packet52MultiBlockChange::processPacket(NetHandler &nethandler)
{
	nethandler.handleMultiBlockChange(*this);
}

int_t Packet52MultiBlockChange::getPacketSize()
{
	return 10 + size * 4;
}
