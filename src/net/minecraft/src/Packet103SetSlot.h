#pragma once

#include "Packet.h"

class NetHandler;
class ItemStack;

// net.minecraft.src.Packet103SetSlot
class Packet103SetSlot : public Packet
{
public:
	Packet103SetSlot() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t windowId;
	int_t itemSlot;
	ItemStack *myItemStack;
};
