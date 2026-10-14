#pragma once

#include "Packet.h"

class NetHandler;
class Entity;

// net.minecraft.src.Packet34EntityTeleport
class Packet34EntityTeleport : public Packet
{
public:
	Packet34EntityTeleport() = default;
	Packet34EntityTeleport(Entity *entity);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId;
	int_t xPosition;
	int_t yPosition;
	int_t zPosition;
	byte_t yaw;
	byte_t pitch;
};
