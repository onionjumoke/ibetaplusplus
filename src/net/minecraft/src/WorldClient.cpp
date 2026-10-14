#include "WorldClient.h"

#include "ChunkCoordinates.h"
#include "ChunkProviderClient.h"
#include "Entity.h"
#include "IWorldAccess.h"
#include "MCHash.h"
#include "NetClientHandler.h"
#include "Packet255KickDisconnect.h"
#include "SaveHandlerMP.h"
#include "WorldBlockPositionType.h"
#include "WorldInfo.h"
#include "WorldProvider.h"
#include <algorithm>

WorldClient::WorldClient(NetClientHandler *queue, long_t seed, int_t dimension)
	: World(new SaveHandlerMP(), u"MpServer", WorldProvider::getProviderForDimension(dimension), seed),
	  sendQueue(queue), clientChunkProvider(nullptr), entityHash(new MCHash())
{
	ChunkCoordinates spawn(8, 64, 8); // stack: setSpawnPoint only copies x/y/z, the Java ptr was GC'd
	setSpawnPoint(&spawn);
	setMapStorage(queue->getMapStorage());

	// World's ctor already ran chunkProvider = getChunkProvider(), but a virtual
	// call from the base ctor dispatches to World::getChunkProvider (the
	// WorldClient vtable isn't active yet), so it built a plain ChunkProvider and
	// left clientChunkProvider null -> doPreChunk crashed. Redo it here, where the
	// override resolves correctly, and discard the base provider. (Base-ctor
	// virtual gotcha.)
	delete chunkProvider;
	chunkProvider = getChunkProvider();
}

WorldClient::~WorldClient()
{
	for (WorldBlockPositionType *change : pendingBlockChanges) delete change;
	delete entityHash;
}

void WorldClient::tick()
{
	setWorldTime(getWorldTime() + 1);
	int_t light = calculateSkylightSubtracted(1.0f);
	if (light != skylightSubtracted)
	{
		skylightSubtracted = light;
		for (IWorldAccess *access : worldAccesses) access->updateAllRenderers();
	}

	for (int_t attempts = 0; attempts < 10 && !entitySpawnQueue.empty(); ++attempts)
	{
		Entity *entity = *entitySpawnQueue.begin();
		if (std::find(loadedEntityList.begin(), loadedEntityList.end(), entity) == loadedEntityList.end())
			entityJoinedWorld(entity);
		else
			entitySpawnQueue.erase(entity);
	}

	sendQueue->processReadPackets();
	for (auto it = pendingBlockChanges.begin(); it != pendingBlockChanges.end();)
	{
		WorldBlockPositionType *change = *it;
		if (--change->field_1206_d == 0)
		{
			World::setBlockAndMetadata(change->field_1202_a, change->field_1201_b, change->field_1207_c,
			                           change->field_1205_e, change->field_1204_f);
			World::markBlockNeedsUpdate(change->field_1202_a, change->field_1201_b, change->field_1207_c);
			delete change;
			it = pendingBlockChanges.erase(it);
		}
		else ++it;
	}
}

void WorldClient::invalidateBlockReceiveRegion(int_t minX, int_t minY, int_t minZ,
	                                            int_t maxX, int_t maxY, int_t maxZ)
{
	for (auto it = pendingBlockChanges.begin(); it != pendingBlockChanges.end();)
	{
		WorldBlockPositionType *change = *it;
		if (change->field_1202_a >= minX && change->field_1201_b >= minY && change->field_1207_c >= minZ &&
		    change->field_1202_a <= maxX && change->field_1201_b <= maxY && change->field_1207_c <= maxZ)
		{
			delete change;
			it = pendingBlockChanges.erase(it);
		}
		else ++it;
	}
}

IChunkProvider *WorldClient::getChunkProvider()
{
	clientChunkProvider = new ChunkProviderClient(this);
	return clientChunkProvider;
}

void WorldClient::setSpawnLocation() { ChunkCoordinates spawn(8, 64, 8); setSpawnPoint(&spawn); }
void WorldClient::updateBlocksAndPlayCaveSounds() {}
void WorldClient::scheduleBlockUpdate(int_t, int_t, int_t, int_t, int_t) {}
bool WorldClient::TickUpdates(bool) { return false; }

void WorldClient::doPreChunk(int_t chunkX, int_t chunkZ, bool load)
{
	if (load) clientChunkProvider->prepareChunk(chunkX, chunkZ);
	else
	{
		clientChunkProvider->unloadChunk(chunkX, chunkZ);
		markBlocksDirty(chunkX * 16, 0, chunkZ * 16, chunkX * 16 + 15, 128, chunkZ * 16 + 15);
	}
}

bool WorldClient::entityJoinedWorld(Entity *entity)
{
	bool added = World::entityJoinedWorld(entity);
	knownEntities.insert(entity);
	if (!added) entitySpawnQueue.insert(entity);
	return added;
}

void WorldClient::setEntityDead(Entity *entity)
{
	World::setEntityDead(entity);
	knownEntities.erase(entity);
}

void WorldClient::obtainEntitySkin(Entity *entity)
{
	World::obtainEntitySkin(entity);
	entitySpawnQueue.erase(entity);
}

void WorldClient::releaseEntitySkin(Entity *entity)
{
	World::releaseEntitySkin(entity);
	if (knownEntities.count(entity)) entitySpawnQueue.insert(entity);
}

void WorldClient::onEntityRemoved(Entity *entity)
{
	// World is about to `delete entity`. Drop every non-owning reference so the next
	// tick()'s entitySpawnQueue flush can't dereference freed memory. Must run after
	// releaseEntitySkin (which re-inserts dying entities into entitySpawnQueue) and
	// right before the delete. Java never needed this: entities were GC-managed, so a
	// stale pointer in the queue was still a live object.
	entitySpawnQueue.erase(entity);
	knownEntities.erase(entity);
	if (static_cast<Entity *>(entityHash->lookup(entity->entityId)) == entity)
		entityHash->removeObject(entity->entityId);
}

void WorldClient::addEntityToWorld(int_t entityId, Entity *entity)
{
	if (Entity *oldEntity = getEntityByID(entityId)) setEntityDead(oldEntity);
	knownEntities.insert(entity);
	entity->entityId = entityId;
	if (!entityJoinedWorld(entity)) entitySpawnQueue.insert(entity);
	entityHash->addKey(entityId, entity);
}

Entity *WorldClient::getEntityByID(int_t entityId)
{
	return static_cast<Entity *>(entityHash->lookup(entityId));
}

Entity *WorldClient::removeEntityFromWorld(int_t entityId)
{
	Entity *entity = static_cast<Entity *>(entityHash->removeObject(entityId));
	if (entity != nullptr)
	{
		knownEntities.erase(entity);
		setEntityDead(entity);
	}
	return entity;
}

bool WorldClient::setBlockMetadata(int_t x, int_t y, int_t z, int_t metadata)
{
	int_t oldId = getBlockId(x, y, z), oldMetadata = getBlockMetadata(x, y, z);
	if (!World::setBlockMetadata(x, y, z, metadata)) return false;
	pendingBlockChanges.push_back(new WorldBlockPositionType(this, x, y, z, oldId, oldMetadata));
	return true;
}

bool WorldClient::setBlockAndMetadata(int_t x, int_t y, int_t z, int_t blockId, int_t metadata)
{
	int_t oldId = getBlockId(x, y, z), oldMetadata = getBlockMetadata(x, y, z);
	if (!World::setBlockAndMetadata(x, y, z, blockId, metadata)) return false;
	pendingBlockChanges.push_back(new WorldBlockPositionType(this, x, y, z, oldId, oldMetadata));
	return true;
}

bool WorldClient::setBlock(int_t x, int_t y, int_t z, int_t blockId)
{
	int_t oldId = getBlockId(x, y, z), oldMetadata = getBlockMetadata(x, y, z);
	if (!World::setBlock(x, y, z, blockId)) return false;
	pendingBlockChanges.push_back(new WorldBlockPositionType(this, x, y, z, oldId, oldMetadata));
	return true;
}

bool WorldClient::setBlockAndMetadataAndInvalidate(int_t x, int_t y, int_t z, int_t blockId, int_t metadata)
{
	invalidateBlockReceiveRegion(x, y, z, x, y, z);
	if (!World::setBlockAndMetadata(x, y, z, blockId, metadata)) return false;
	notifyBlockChange(x, y, z, blockId);
	return true;
}

void WorldClient::sendQuittingDisconnectingPacket()
{
	sendQueue->sendPacketAndFlush(new Packet255KickDisconnect(u"Quitting"));
}

void WorldClient::updateWeather()
{
	if (worldProvider->hasNoSky) return;
	prevRainingStrength = rainingStrength;
	rainingStrength += worldInfo->getRaining() ? 0.01f : -0.01f;
	rainingStrength = std::max(0.0f, std::min(1.0f, rainingStrength));
	prevThunderingStrength = thunderingStrength;
	thunderingStrength += worldInfo->getThundering() ? 0.01f : -0.01f;
	thunderingStrength = std::max(0.0f, std::min(1.0f, thunderingStrength));
}
