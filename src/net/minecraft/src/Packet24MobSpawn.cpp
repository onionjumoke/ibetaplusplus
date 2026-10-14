#include "Packet24MobSpawn.h"

#include "NetHandler.h"
#include "EntityLiving.h"
#include "EntityList.h"
#include "DataWatcher.h"
#include "WatchableObject.h"
#include "MathHelper.h"

Packet24MobSpawn::Packet24MobSpawn(EntityLiving *entityliving)
{
	entityId  = entityliving->entityId;
	type      = (byte_t)(int_t)EntityList::getEntityID(entityliving);
	xPosition = MathHelper::floor_double(entityliving->posX * 32.0);
	yPosition = MathHelper::floor_double(entityliving->posY * 32.0);
	zPosition = MathHelper::floor_double(entityliving->posZ * 32.0);
	yaw       = (byte_t)(int)((entityliving->rotationYaw   * 256.0f) / 360.0f);
	pitch     = (byte_t)(int)((entityliving->rotationPitch * 256.0f) / 360.0f);
	metaData  = entityliving->getDataWatcher();
}

void Packet24MobSpawn::readPacketData(std::istream &is)
{
	entityId  = IOUtil::readInt(is);
	type      = IOUtil::readByte(is);
	xPosition = IOUtil::readInt(is);
	yPosition = IOUtil::readInt(is);
	zPosition = IOUtil::readInt(is);
	yaw       = IOUtil::readByte(is);
	pitch     = IOUtil::readByte(is);
	receivedMetadata = DataWatcher::readWatchableObjects(is);
}

void Packet24MobSpawn::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityId);
	IOUtil::writeByte(os, type);
	IOUtil::writeInt(os, xPosition);
	IOUtil::writeInt(os, yPosition);
	IOUtil::writeInt(os, zPosition);
	IOUtil::writeByte(os, yaw);
	IOUtil::writeByte(os, pitch);
	metaData->writeWatchableObjects(os);
}

void Packet24MobSpawn::processPacket(NetHandler &nethandler)
{
	nethandler.handleMobSpawn(*this);
}

int_t Packet24MobSpawn::getPacketSize()
{
	return 20;
}

std::vector<WatchableObject *> Packet24MobSpawn::getMetadata()
{
	return receivedMetadata;
}
