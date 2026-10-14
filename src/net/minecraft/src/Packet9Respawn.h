#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet9Respawn
class Packet9Respawn : public Packet
{
public:
	Packet9Respawn() = default;
	Packet9Respawn(byte_t dimension);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	byte_t respawnDimension; // blockBounds
	byte_t &dimension = respawnDimension;
};
