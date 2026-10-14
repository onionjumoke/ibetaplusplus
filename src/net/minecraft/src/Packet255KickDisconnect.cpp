#include "Packet255KickDisconnect.h"

#include "NetHandler.h"

Packet255KickDisconnect::Packet255KickDisconnect(const jstring &r) :
	reason(r)
{
}

void Packet255KickDisconnect::readPacketData(std::istream &is)
{
	reason = Packet::readString(is, 100);
}

void Packet255KickDisconnect::writePacketData(std::ostream &os)
{
	Packet::writeString(reason, os);
}

void Packet255KickDisconnect::processPacket(NetHandler &nethandler)
{
	nethandler.handleKickDisconnect(*this);
}

int_t Packet255KickDisconnect::getPacketSize()
{
	return 4 + (int_t)reason.length() * 2;
}
