#include "Packet130UpdateSign.h"

#include "NetHandler.h"

void Packet130UpdateSign::readPacketData(std::istream &is)
{
	xPosition = IOUtil::readInt(is);
	yPosition = IOUtil::readShort(is);
	zPosition = IOUtil::readInt(is);
	for (int_t i = 0; i < 4; i++)
	{
		signLines[i] = Packet::readString(is, 15);
	}
}

void Packet130UpdateSign::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, xPosition);
	IOUtil::writeShort(os, yPosition);
	IOUtil::writeInt(os, zPosition);
	for (int_t i = 0; i < 4; i++)
	{
		Packet::writeString(signLines[i], os);
	}
}

void Packet130UpdateSign::processPacket(NetHandler &nethandler)
{
	nethandler.handleUpdateSign(*this);
}

int_t Packet130UpdateSign::getPacketSize()
{
	int_t size = 10;
	for (int_t i = 0; i < 4; i++)
		size += 4 + (int_t)signLines[i].length() * 2;
	return size;
}
