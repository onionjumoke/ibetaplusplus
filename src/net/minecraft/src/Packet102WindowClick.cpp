#include "Packet102WindowClick.h"

#include "NetHandler.h"
#include "ItemStack.h"

Packet102WindowClick::Packet102WindowClick(int_t windowId, int_t slot, int_t mouseClick, bool shift, ItemStack *itemstack, short_t action) :
	window_Id(windowId),
	inventorySlot(slot),
	mouseClick(mouseClick),
	action(action),
	itemStack(itemstack),
	shiftPressed(shift)
{
}

void Packet102WindowClick::readPacketData(std::istream &is)
{
	window_Id = IOUtil::readByte(is);
	inventorySlot = IOUtil::readShort(is);
	mouseClick = IOUtil::readByte(is);
	action = IOUtil::readShort(is);
	shiftPressed = IOUtil::readBoolean(is);
	short_t word0 = IOUtil::readShort(is);
	if (word0 >= 0)
	{
		byte_t byte0 = IOUtil::readByte(is);
		short_t word1 = IOUtil::readShort(is);
		itemStack = new ItemStack(word0, byte0, word1);
	}
	else
	{
		itemStack = nullptr;
	}
}

void Packet102WindowClick::writePacketData(std::ostream &os)
{
	IOUtil::writeByte(os, window_Id);
	IOUtil::writeShort(os, inventorySlot);
	IOUtil::writeByte(os, mouseClick);
	IOUtil::writeShort(os, action);
	IOUtil::writeBoolean(os, shiftPressed);
	if (itemStack == nullptr)
	{
		IOUtil::writeShort(os, -1);
	}
	else
	{
		IOUtil::writeShort(os, itemStack->itemID);
		IOUtil::writeByte(os, itemStack->stackSize);
		IOUtil::writeShort(os, itemStack->getItemDamage());
	}
}

void Packet102WindowClick::processPacket(NetHandler &nethandler)
{
	nethandler.handleWindowClick(*this);
}

int_t Packet102WindowClick::getPacketSize()
{
	return 11;
}
