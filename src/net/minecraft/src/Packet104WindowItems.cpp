#include "Packet104WindowItems.h"

#include "NetHandler.h"
#include "ItemStack.h"

void Packet104WindowItems::readPacketData(std::istream &is)
{
	windowId = IOUtil::readByte(is);
	short_t count = IOUtil::readShort(is);
	itemStack.resize(count);
	for (int_t i = 0; i < count; i++)
	{
		short_t word0 = IOUtil::readShort(is);
		if (word0 >= 0)
		{
			byte_t byte0 = IOUtil::readByte(is);
			short_t word1 = IOUtil::readShort(is);
			itemStack[i] = new ItemStack(word0, byte0, word1);
		}
		else
		{
			itemStack[i] = nullptr;
		}
	}
}

void Packet104WindowItems::writePacketData(std::ostream &os)
{
	IOUtil::writeByte(os, windowId);
	IOUtil::writeShort(os, (short_t)itemStack.size());
	for (auto *stack : itemStack)
	{
		if (stack == nullptr)
		{
			IOUtil::writeShort(os, -1);
		}
		else
		{
			IOUtil::writeShort(os, stack->itemID);
			IOUtil::writeByte(os, stack->stackSize);
			IOUtil::writeShort(os, stack->getItemDamage());
		}
	}
}

void Packet104WindowItems::processPacket(NetHandler &nethandler)
{
	nethandler.handleWindowItems(*this);
}

int_t Packet104WindowItems::getPacketSize()
{
	return 3 + (int_t)itemStack.size() * 5;
}
