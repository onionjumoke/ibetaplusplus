#include "Packet70Bed.h"

#include "NetHandler.h"

const jstring Packet70Bed::bedMessages[] = {
	u"tile.bed.notValid",
	u"",
	u""
};
const int_t Packet70Bed::bedMessageCount = 3;

void Packet70Bed::readPacketData(std::istream &is)
{
	bedState = IOUtil::readByte(is);
}

void Packet70Bed::writePacketData(std::ostream &os)
{
	IOUtil::writeByte(os, bedState);
}

void Packet70Bed::processPacket(NetHandler &nethandler)
{
	nethandler.handleBed(*this);
}

int_t Packet70Bed::getPacketSize()
{
	return 1;
}
