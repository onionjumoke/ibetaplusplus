#pragma once

#include "Packet.h"
#include <vector>

class NetHandler;

// net.minecraft.src.Packet51MapChunk
class Packet51MapChunk : public Packet
{
public:
	Packet51MapChunk();

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t xPosition;
	int_t yPosition;
	int_t zPosition;
	int_t xSize;
	int_t ySize;
	int_t zSize;
	std::vector<byte_t> chunk;

private:
	int_t chunkSize;
};
