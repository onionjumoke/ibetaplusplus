#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet27Position
class Packet27Position : public Packet
{
public:
	Packet27Position() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

private:
	float strafeMovement;    // signStanding
	float forwardMovement;   // bedOccupied
	bool isJumping;          // leverFacing
	bool isSneaking;         // leverState
	float ladderFacing;
	float signWall;
};
