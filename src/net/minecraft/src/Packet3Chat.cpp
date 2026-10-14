#include "Packet3Chat.h"

#include "NetHandler.h"

Packet3Chat::Packet3Chat(const jstring &s)
{
	if ((int_t)s.length() > 119)
		message = s.substr(0, 119);
	else
		message = s;
}

void Packet3Chat::readPacketData(std::istream &is)
{
	message = Packet::readString(is, 119);
}

void Packet3Chat::writePacketData(std::ostream &os)
{
	Packet::writeString(message, os);
}

void Packet3Chat::processPacket(NetHandler &nethandler)
{
	nethandler.handleChat(*this);
}

int_t Packet3Chat::getPacketSize()
{
	return (int_t)message.length() * 2;
}
