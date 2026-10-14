#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet105UpdateProgressbar
class Packet105UpdateProgressbar : public Packet
{
public:
	Packet105UpdateProgressbar() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t windowId;
	int_t progressBar;
	int_t progressBarValue;
};
