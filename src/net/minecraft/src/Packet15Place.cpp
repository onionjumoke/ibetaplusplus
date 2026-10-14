#include "Packet15Place.h"

#include "NetHandler.h"
#include "ItemStack.h"

Packet15Place::Packet15Place(int_t x, int_t y, int_t z, int_t direction, ItemStack *itemstack) :
	xPosition(x),
	yPosition(y),
	zPosition(z),
	direction(direction),
	itemStack(itemstack)
{
}

void Packet15Place::readPacketData(std::istream &is)
{
	xPosition = IOUtil::readInt(is);
	yPosition = is.get();
	zPosition = IOUtil::readInt(is);
	direction = is.get();
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

void Packet15Place::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, xPosition);
	os.put(yPosition);
	IOUtil::writeInt(os, zPosition);
	os.put(direction);
	if (itemStack == nullptr)
	{
		IOUtil::writeShort(os, -1);
	}
	else
	{
		IOUtil::writeShort(os, itemStack->itemID);
		os.put(itemStack->stackSize);
		IOUtil::writeShort(os, itemStack->getItemDamage());
	}
}

void Packet15Place::processPacket(NetHandler &nethandler)
{
	nethandler.handlePlace(*this);
}

int_t Packet15Place::getPacketSize()
{
	return 15;
}
