#include "Packet34EntityTeleport.h"

#include "NetHandler.h"
#include "Entity.h"
#include "MathHelper.h"

Packet34EntityTeleport::Packet34EntityTeleport(Entity *entity)
{
	entityId = entity->entityId;
	xPosition = MathHelper::floor_double(entity->posX * 32.0);
	yPosition = MathHelper::floor_double(entity->posY * 32.0);
	zPosition = MathHelper::floor_double(entity->posZ * 32.0);
	yaw = (byte_t)(int)((entity->rotationYaw * 256.0f) / 360.0f);
	pitch = (byte_t)(int)((entity->rotationPitch * 256.0f) / 360.0f);
}

void Packet34EntityTeleport::readPacketData(std::istream &is)
{
	entityId = IOUtil::readInt(is);
	xPosition = IOUtil::readInt(is);
	yPosition = IOUtil::readInt(is);
	zPosition = IOUtil::readInt(is);
	yaw = (byte_t)is.get();
	pitch = (byte_t)is.get();
}

void Packet34EntityTeleport::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityId);
	IOUtil::writeInt(os, xPosition);
	IOUtil::writeInt(os, yPosition);
	IOUtil::writeInt(os, zPosition);
	os.put(yaw);
	os.put(pitch);
}

void Packet34EntityTeleport::processPacket(NetHandler &nethandler)
{
	nethandler.handleEntityTeleport(*this);
}

int_t Packet34EntityTeleport::getPacketSize()
{
	return 34;
}
