#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet17Sleep
class Packet17Sleep : public Packet
{
public:
	Packet17Sleep() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId;          // buttonFacing
	int_t sleepDirection;    // platePressed
	int_t bedX;              // buttonPressed
	int_t bedY;              // plateWeight
	int_t bedZ;              // plateMaterial

	int_t &sleepState = sleepDirection;
};
