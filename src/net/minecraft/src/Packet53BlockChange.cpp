#include "Packet53BlockChange.h"

#include "NetHandler.h"

Packet53BlockChange::Packet53BlockChange() :
	xPosition(0),
	yPosition(0),
	zPosition(0),
	type(0),
	metadata(0)
{
	isChunkDataPacket = true;
}

void Packet53BlockChange::readPacketData(std::istream &is)
{
	xPosition = IOUtil::readInt(is);
	yPosition = is.get();
	zPosition = IOUtil::readInt(is);
	type = is.get();
	metadata = is.get();
}

void Packet53BlockChange::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, xPosition);
	os.put(yPosition);
	IOUtil::writeInt(os, zPosition);
	os.put(type);
	os.put(metadata);
}

void Packet53BlockChange::processPacket(NetHandler &nethandler)
{
	nethandler.handleBlockChange(*this);
}

int_t Packet53BlockChange::getPacketSize()
{
	return 11;
}
