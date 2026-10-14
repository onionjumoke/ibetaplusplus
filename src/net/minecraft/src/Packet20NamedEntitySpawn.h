#pragma once

#include "Packet.h"

class NetHandler;
class EntityPlayer;

// net.minecraft.src.Packet20NamedEntitySpawn
class Packet20NamedEntitySpawn : public Packet
{
public:
	Packet20NamedEntitySpawn() = default;
	Packet20NamedEntitySpawn(EntityPlayer *entityplayer);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId;
	jstring name;
	int_t xPosition;
	int_t yPosition;
	int_t zPosition;
	byte_t rotation;
	byte_t pitch;
	int_t currentItem;
};
