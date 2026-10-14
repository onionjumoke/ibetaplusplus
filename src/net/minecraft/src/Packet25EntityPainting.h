#pragma once

#include "Packet.h"

class NetHandler;
class EntityPainting;

// net.minecraft.src.Packet25EntityPainting
class Packet25EntityPainting : public Packet
{
public:
	Packet25EntityPainting() = default;
	Packet25EntityPainting(EntityPainting *entitypainting);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId;
	int_t xPosition;
	int_t yPosition;
	int_t zPosition;
	int_t direction;
	jstring title;
};
