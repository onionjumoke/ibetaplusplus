#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet23VehicleSpawn
class Packet23VehicleSpawn : public Packet
{
public:
	Packet23VehicleSpawn() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId;
	int_t xPosition;
	int_t yPosition;
	int_t zPosition;
	int_t speedX;      // blockMaterial
	int_t speedY;      // blockHardness
	int_t speedZ;      // blockResistance
	int_t type;
	int_t throwerEntityId; // blockLightValue

	int_t &velocityX = speedX;
	int_t &velocityY = speedY;
	int_t &velocityZ = speedZ;
};
