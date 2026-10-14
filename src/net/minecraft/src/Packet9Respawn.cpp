#include "Packet9Respawn.h"

#include "NetHandler.h"

Packet9Respawn::Packet9Respawn(byte_t dimension) :
	respawnDimension(dimension)
{
}

void Packet9Respawn::readPacketData(std::istream &is)
{
	respawnDimension = IOUtil::readByte(is);
}

void Packet9Respawn::writePacketData(std::ostream &os)
{
	IOUtil::writeByte(os, respawnDimension);
}

void Packet9Respawn::processPacket(NetHandler &nethandler)
{
	nethandler.handleRespawn(*this);
}

int_t Packet9Respawn::getPacketSize()
{
	return 1;
}
