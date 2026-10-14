#include "Packet1Login.h"

#include "NetHandler.h"

Packet1Login::Packet1Login(const jstring &s, int_t i) :
	protocolVersion(i),
	username(s),
	mapSeed(0),
	dimension(0)
{
}

void Packet1Login::readPacketData(std::istream &is)
{
	protocolVersion = IOUtil::readInt(is);
	username = Packet::readString(is, 16);
	mapSeed = IOUtil::readLong(is);
	dimension = IOUtil::readByte(is);
}

void Packet1Login::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, protocolVersion);
	Packet::writeString(username, os);
	IOUtil::writeLong(os, mapSeed);
	IOUtil::writeByte(os, dimension);
}

void Packet1Login::processPacket(NetHandler &nethandler)
{
	nethandler.handleLogin(*this);
}

int_t Packet1Login::getPacketSize()
{
	return 4 + (int_t)username.length() * 2 + 4 + 5;
}
