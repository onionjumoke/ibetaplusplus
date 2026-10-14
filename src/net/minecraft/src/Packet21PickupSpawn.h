#pragma once

#include "Packet.h"

class NetHandler;
class EntityItem;

// net.minecraft.src.Packet21PickupSpawn
class Packet21PickupSpawn : public Packet
{
public:
	Packet21PickupSpawn() = default;
	Packet21PickupSpawn(EntityItem *entityitem);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId;
	int_t xPosition;
	int_t yPosition;
	int_t zPosition;
	byte_t rotation;
	byte_t pitch;
	byte_t roll;
	int_t itemID;
	int_t count;
	int_t itemDamage;
};
