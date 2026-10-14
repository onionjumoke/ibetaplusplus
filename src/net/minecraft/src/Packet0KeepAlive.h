#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet0KeepAlive
class Packet0KeepAlive : public Packet
{
public:
	Packet0KeepAlive() = default;
	explicit Packet0KeepAlive(int_t id) : keepAliveId(id) {}

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t keepAliveId = 0;
};
