#include "Packet60Explosion.h"

#include "NetHandler.h"
#include "ChunkPosition.h"

void Packet60Explosion::readPacketData(std::istream &is)
{
	explosionX = IOUtil::readDouble(is);
	explosionY = IOUtil::readDouble(is);
	explosionZ = IOUtil::readDouble(is);
	explosionSize = IOUtil::readFloat(is);
	int_t count = IOUtil::readInt(is);
	int_t j = (int_t)explosionX;
	int_t k = (int_t)explosionY;
	int_t l = (int_t)explosionZ;
	for (int_t i = 0; i < count; i++)
	{
		int_t j1 = IOUtil::readByte(is) + j;
		int_t k1 = IOUtil::readByte(is) + k;
		int_t l1 = IOUtil::readByte(is) + l;
		destroyedBlockPositions.insert(new ChunkPosition(j1, k1, l1));
	}
}

void Packet60Explosion::writePacketData(std::ostream &os)
{
	IOUtil::writeDouble(os, explosionX);
	IOUtil::writeDouble(os, explosionY);
	IOUtil::writeDouble(os, explosionZ);
	IOUtil::writeFloat(os, explosionSize);
	IOUtil::writeInt(os, (int_t)destroyedBlockPositions.size());
	int_t i = (int_t)explosionX;
	int_t j = (int_t)explosionY;
	int_t k = (int_t)explosionZ;
	for (auto *pos : destroyedBlockPositions)
	{
		int_t l = pos->x - i;
		int_t i1 = pos->y - j;
		int_t j1 = pos->z - k;
		os.put(l);
		os.put(i1);
		os.put(j1);
	}
}

void Packet60Explosion::processPacket(NetHandler &nethandler)
{
	nethandler.handleExplosion(*this);
}

int_t Packet60Explosion::getPacketSize()
{
	return 32 + (int_t)destroyedBlockPositions.size() * 3;
}
