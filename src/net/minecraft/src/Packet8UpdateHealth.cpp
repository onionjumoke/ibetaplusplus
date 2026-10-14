#include "Packet8UpdateHealth.h"

#include "NetHandler.h"

void Packet8UpdateHealth::readPacketData(std::istream &is)
{
	healthMP = IOUtil::readShort(is);
}

void Packet8UpdateHealth::writePacketData(std::ostream &os)
{
	IOUtil::writeShort(os, healthMP);
}

void Packet8UpdateHealth::processPacket(NetHandler &nethandler)
{
	nethandler.handleHealth(*this);
}

int_t Packet8UpdateHealth::getPacketSize()
{
	return 2;
}
