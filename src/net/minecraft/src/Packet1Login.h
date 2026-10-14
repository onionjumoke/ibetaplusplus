#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet1Login
class Packet1Login : public Packet
{
public:
	Packet1Login() = default;
	Packet1Login(const jstring &s, int_t i);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t protocolVersion;
	jstring username;
	long_t mapSeed;
	byte_t dimension;
};
