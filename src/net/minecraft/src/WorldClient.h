#pragma once

#include "World.h"
#include <unordered_set>
#include <vector>

class ChunkProviderClient;
class MCHash;
class NetClientHandler;
class WorldBlockPositionType;

class WorldClient : public World
{
public:
	WorldClient(NetClientHandler *sendQueue, long_t seed, int_t dimension);
	~WorldClient() override;

	void tick() override;
	void invalidateBlockReceiveRegion(int_t minX, int_t minY, int_t minZ,
	                                  int_t maxX, int_t maxY, int_t maxZ);
	void setSpawnLocation() override;
	void scheduleBlockUpdate(int_t x, int_t y, int_t z, int_t blockId, int_t delay) override;
	bool TickUpdates(bool force) override;
	void doPreChunk(int_t chunkX, int_t chunkZ, bool load);
	bool entityJoinedWorld(Entity *entity) override;
	void setEntityDead(Entity *entity) override;
	void addEntityToWorld(int_t entityId, Entity *entity);
	Entity *getEntityByID(int_t entityId);
	Entity *removeEntityFromWorld(int_t entityId);
	bool setBlockMetadata(int_t x, int_t y, int_t z, int_t metadata) override;
	bool setBlockAndMetadata(int_t x, int_t y, int_t z, int_t blockId, int_t metadata) override;
	bool setBlock(int_t x, int_t y, int_t z, int_t blockId) override;
	bool setBlockAndMetadataAndInvalidate(int_t x, int_t y, int_t z, int_t blockId, int_t metadata);
	void sendQuittingDisconnectingPacket() override;
	void setRainStrength(float strength) { rainingStrength = strength; }

protected:
	IChunkProvider *getChunkProvider() override;
	void updateBlocksAndPlayCaveSounds() override;
	void obtainEntitySkin(Entity *entity) override;
	void releaseEntitySkin(Entity *entity) override;
	void onEntityRemoved(Entity *entity) override;
	void updateWeather() override;

private:
	std::vector<WorldBlockPositionType *> pendingBlockChanges;
	NetClientHandler *sendQueue;
	ChunkProviderClient *clientChunkProvider;
	MCHash *entityHash;
	std::unordered_set<Entity *> knownEntities;
	std::unordered_set<Entity *> entitySpawnQueue;
};
