#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet53BlockChange
class Packet53BlockChange : public Packet
{
public:
	Packet53BlockChange();

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t xPosition;
	int_t yPosition;
	int_t zPosition;
	int_t type;
	int_t metadata;
};
