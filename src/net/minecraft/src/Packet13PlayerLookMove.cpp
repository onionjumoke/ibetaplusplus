#include "Packet13PlayerLookMove.h"

#include "NetHandler.h"

Packet13PlayerLookMove::Packet13PlayerLookMove()
{
	rotating = true;
	moving = true;
}

Packet13PlayerLookMove::Packet13PlayerLookMove(double d, double d1, double d2, double d3, float f, float f1, bool flag)
{
	xPosition = d;
	yPosition = d1;
	stance = d2;
	zPosition = d3;
	yaw = f;
	pitch = f1;
	onGround = flag;
	rotating = true;
	moving = true;
}

void Packet13PlayerLookMove::readPacketData(std::istream &is)
{
	xPosition = IOUtil::readDouble(is);
	yPosition = IOUtil::readDouble(is);
	stance = IOUtil::readDouble(is);
	zPosition = IOUtil::readDouble(is);
	yaw = IOUtil::readFloat(is);
	pitch = IOUtil::readFloat(is);
	Packet10Flying::readPacketData(is);
}

void Packet13PlayerLookMove::writePacketData(std::ostream &os)
{
	IOUtil::writeDouble(os, xPosition);
	IOUtil::writeDouble(os, yPosition);
	IOUtil::writeDouble(os, stance);
	IOUtil::writeDouble(os, zPosition);
	IOUtil::writeFloat(os, yaw);
	IOUtil::writeFloat(os, pitch);
	Packet10Flying::writePacketData(os);
}

void Packet13PlayerLookMove::processPacket(NetHandler &nethandler)
{
	// Server -> client teleport/position correction (also the respawn place).
	// Without this the client ignored every correction and stayed at a
	// garbage position while the world streamed around the real spawn.
	nethandler.handleFlying(*this);
}

int_t Packet13PlayerLookMove::getPacketSize()
{
	return 41;
}
