#include "Packet103SetSlot.h"

#include "NetHandler.h"
#include "ItemStack.h"

void Packet103SetSlot::readPacketData(std::istream &is)
{
	windowId = IOUtil::readByte(is);
	itemSlot = IOUtil::readShort(is);
	short_t word0 = IOUtil::readShort(is);
	if (word0 >= 0)
	{
		byte_t byte0 = IOUtil::readByte(is);
		short_t word1 = IOUtil::readShort(is);
		myItemStack = new ItemStack(word0, byte0, word1);
	}
	else
	{
		myItemStack = nullptr;
	}
}

void Packet103SetSlot::writePacketData(std::ostream &os)
{
	IOUtil::writeByte(os, windowId);
	IOUtil::writeShort(os, itemSlot);
	if (myItemStack == nullptr)
	{
		IOUtil::writeShort(os, -1);
	}
	else
	{
		IOUtil::writeShort(os, myItemStack->itemID);
		IOUtil::writeByte(os, myItemStack->stackSize);
		IOUtil::writeShort(os, myItemStack->getItemDamage());
	}
}

void Packet103SetSlot::processPacket(NetHandler &nethandler)
{
	nethandler.handleSetSlot(*this);
}

int_t Packet103SetSlot::getPacketSize()
{
	return 8;
}
