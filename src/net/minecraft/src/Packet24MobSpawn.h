#pragma once

#include "Packet.h"
#include <vector>

class NetHandler;
class EntityLiving;
class DataWatcher;
class WatchableObject;

// net.minecraft.src.Packet24MobSpawn
class Packet24MobSpawn : public Packet
{
public:
	Packet24MobSpawn() = default;
	Packet24MobSpawn(EntityLiving *entityliving);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	std::vector<WatchableObject *> getMetadata();

	int_t entityId;
	byte_t type;
	int_t xPosition;
	int_t yPosition;
	int_t zPosition;
	byte_t yaw;
	byte_t pitch;

private:
	DataWatcher *metaData;
	std::vector<WatchableObject *> receivedMetadata;
};
