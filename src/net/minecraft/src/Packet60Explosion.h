#pragma once

#include "Packet.h"
#include <set>

class NetHandler;
class ChunkPosition;

// net.minecraft.src.Packet60Explosion
class Packet60Explosion : public Packet
{
public:
	Packet60Explosion() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	double explosionX;
	double explosionY;
	double explosionZ;
	float explosionSize;
	std::set<ChunkPosition*> destroyedBlockPositions;
};
