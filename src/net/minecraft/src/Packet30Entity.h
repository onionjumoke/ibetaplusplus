#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet30Entity
class Packet30Entity : public Packet
{
public:
	Packet30Entity();

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId;
	byte_t xPosition;
	byte_t yPosition;
	byte_t zPosition;
	byte_t yaw;
	byte_t pitch;
	bool rotating;
};
