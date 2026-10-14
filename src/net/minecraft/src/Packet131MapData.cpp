#include "Packet131MapData.h"

#include "NetHandler.h"

void Packet131MapData::readPacketData(std::istream &is)
{
	itemId = IOUtil::readShort(is);
	mapId = IOUtil::readShort(is);
	short_t len = IOUtil::readShort(is);
	data.resize(len);
	is.read(reinterpret_cast<char*>(data.data()), len);
}

void Packet131MapData::writePacketData(std::ostream &os)
{
	IOUtil::writeShort(os, itemId);
	IOUtil::writeShort(os, mapId);
	IOUtil::writeShort(os, (short_t)data.size());
	os.write(reinterpret_cast<const char*>(data.data()), data.size());
}

void Packet131MapData::processPacket(NetHandler &nethandler)
{
	nethandler.handleMapData(*this);
}

int_t Packet131MapData::getPacketSize()
{
	return 6 + (int_t)data.size();
}
