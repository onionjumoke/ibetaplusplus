#pragma once

#include "Packet.h"
#include <vector>

class NetHandler;
class ItemStack;

// net.minecraft.src.Packet104WindowItems
class Packet104WindowItems : public Packet
{
public:
	Packet104WindowItems() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t windowId;
	std::vector<ItemStack*> itemStack;
};
