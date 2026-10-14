#include "Packet100OpenWindow.h"

#include "NetHandler.h"

void Packet100OpenWindow::readPacketData(std::istream &is)
{
	windowId = IOUtil::readByte(is);
	inventoryType = IOUtil::readByte(is);
	// Java uses DataInputStream.readUTF (byte-length-prefixed modified UTF-8),
	// NOT Packet.readString (char-count + 2 bytes/char). Using readString here
	// over-read the title and desynced the whole stream (garbage slotsCount, then
	// negative string lengths -> crash/OOM when opening containers).
	windowTitle = IOUtil::readUTF(is);
	slotsCount = IOUtil::readByte(is);
}

void Packet100OpenWindow::writePacketData(std::ostream &os)
{
	IOUtil::writeByte(os, windowId);
	IOUtil::writeByte(os, inventoryType);
	IOUtil::writeUTF(os, windowTitle);
	IOUtil::writeByte(os, slotsCount);
}

void Packet100OpenWindow::processPacket(NetHandler &nethandler)
{
	nethandler.handleOpenWindow(*this);
}

int_t Packet100OpenWindow::getPacketSize()
{
	return 3 + (int_t)windowTitle.length();
}
