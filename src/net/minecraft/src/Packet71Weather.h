#pragma once

#include "Packet.h"

class NetHandler;
class Entity;

// net.minecraft.src.Packet71Weather
class Packet71Weather : public Packet
{
public:
	Packet71Weather() = default;
	Packet71Weather(Entity *entity);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId;       // nbtTagType
	int_t weatherType;    // nbtTagName
	int_t posX;           // nbtTagList
	int_t posY;           // nbtTagCompound
	int_t posZ;           // nbtTagValue
};
