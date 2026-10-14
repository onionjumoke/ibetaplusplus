#include "Packet14BlockDig.h"

#include "NetHandler.h"

Packet14BlockDig::Packet14BlockDig(int_t status, int_t x, int_t y, int_t z, int_t face) :
	status(status),
	xPosition(x),
	yPosition(y),
	zPosition(z),
	face(face)
{
}

void Packet14BlockDig::readPacketData(std::istream &is)
{
	status = is.get();
	xPosition = IOUtil::readInt(is);
	yPosition = is.get();
	zPosition = IOUtil::readInt(is);
	face = is.get();
}

void Packet14BlockDig::writePacketData(std::ostream &os)
{
	os.put(status);
	IOUtil::writeInt(os, xPosition);
	os.put(yPosition);
	IOUtil::writeInt(os, zPosition);
	os.put(face);
}

void Packet14BlockDig::processPacket(NetHandler &nethandler)
{
	nethandler.handleBlockDig(*this);
}

int_t Packet14BlockDig::getPacketSize()
{
	return 11;
}
