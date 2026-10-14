#include "EntityClientPlayerMP.h"

#include <atomic>

#include "AxisAlignedBB.h"
#include "Container.h"
#include "InventoryPlayer.h"
#include "MathHelper.h"
#include "NetClientHandler.h"
#include "Packet10Flying.h"
#include "Packet101CloseWindow.h"
#include "Packet11PlayerPosition.h"
#include "Packet12PlayerLook.h"
#include "Packet13PlayerLookMove.h"
#include "Packet14BlockDig.h"
#include "Packet18Animation.h"
#include "Packet19EntityAction.h"
#include "Packet3Chat.h"
#include "Packet9Respawn.h"
#include "StatBase.h"
#include "World.h"
#include "java/String.h"

EntityClientPlayerMP::EntityClientPlayerMP(Minecraft *minecraft, World *world, Session *session, NetClientHandler *netclienthandler)
	: EntityPlayerSP(minecraft, world, session, 0)
	, sendQueue(netclienthandler)
	, field_9380_bx(0)
	, field_21093_bH(false)
	, oldPosX(0.0)
	, field_9378_bz(0.0)
	, oldPosY(0.0)
	, oldPosZ(0.0)
	, oldRotationYaw(0.0f)
	, oldRotationPitch(0.0f)
	, field_9382_bF(false)
	, wasSneaking(false)
	, field_12242_bI(0)
{
}

bool EntityClientPlayerMP::attackEntityFrom(Entity *, int_t)
{
	return false;
}

void EntityClientPlayerMP::heal(int_t)
{
}

void EntityClientPlayerMP::onUpdate()
{
	static std::atomic<uint32_t> un{0};
	if (un.fetch_add(1) < 10)
		(void)0;
	if (!worldObj->blockExists(MathHelper::floor_double(posX), 64, MathHelper::floor_double(posZ)))
	{
		// Pre-chunks count as existing: still send position so the server
		// streams terrain. Vanilla sends flying packets regardless.
		static std::atomic<uint32_t> fn{0};
		if (fn.fetch_add(1) < 10)
		{
			(void)0;
			func_4056_N();
		}
		return;
	}
	EntityPlayerSP::onUpdate();
	func_4056_N();
}

void EntityClientPlayerMP::func_4056_N()
{
	if (field_9380_bx++ == 20)
	{
		sendInventoryChanged();
		field_9380_bx = 0;
	}
	bool flag = isSneaking();
	if (flag != wasSneaking)
	{
		sendQueue->addToSendQueue(new Packet19EntityAction(this, flag ? 1 : 2));
		wasSneaking = flag;
	}
	double d = posX - oldPosX;
	double d1 = boundingBox->minY - field_9378_bz;
	double d2 = posY - oldPosY;
	double d3 = posZ - oldPosZ;
	double d4 = rotationYaw - oldRotationYaw;
	double d5 = rotationPitch - oldRotationPitch;
	bool flag1 = d1 != 0.0 || d2 != 0.0 || d != 0.0 || d3 != 0.0;
	bool flag2 = d4 != 0.0 || d5 != 0.0;
	if (ridingEntity != nullptr)
	{
		if (flag2)
			sendQueue->addToSendQueue(new Packet11PlayerPosition(motionX, -999.0, -999.0, motionZ, onGround));
		else
			sendQueue->addToSendQueue(new Packet13PlayerLookMove(motionX, -999.0, -999.0, motionZ, rotationYaw, rotationPitch, onGround));
		flag1 = false;
	}
	else if (flag1 && flag2)
	{
		{
			static std::atomic<int> sp{0};
			if (sp.fetch_add(1) < 20)
				(void)0;
		}
		sendQueue->addToSendQueue(new Packet13PlayerLookMove(posX, boundingBox->minY, posY, posZ, rotationYaw, rotationPitch, onGround));
		field_12242_bI = 0;
	}
	else if (flag1)
	{
		sendQueue->addToSendQueue(new Packet11PlayerPosition(posX, boundingBox->minY, posY, posZ, onGround));
		field_12242_bI = 0;
	}
	else if (flag2)
	{
		sendQueue->addToSendQueue(new Packet12PlayerLook(rotationYaw, rotationPitch, onGround));
		field_12242_bI = 0;
	}
	else
	{
		sendQueue->addToSendQueue(new Packet10Flying(onGround));
		if (field_9382_bF != onGround || field_12242_bI > 200)
			field_12242_bI = 0;
		else
			field_12242_bI++;
	}
	field_9382_bF = onGround;
	if (flag1)
	{
		oldPosX = posX;
		field_9378_bz = boundingBox->minY;
		oldPosY = posY;
		oldPosZ = posZ;
	}
	if (flag2)
	{
		oldRotationYaw = rotationYaw;
		oldRotationPitch = rotationPitch;
	}
}

void EntityClientPlayerMP::dropCurrentItem()
{
	sendQueue->addToSendQueue(new Packet14BlockDig(4, 0, 0, 0, 0));
}

void EntityClientPlayerMP::sendInventoryChanged()
{
}

void EntityClientPlayerMP::sendChatMessage(const std::string &s)
{
	sendQueue->addToSendQueue(new Packet3Chat(String::fromUTF8(s)));
}

void EntityClientPlayerMP::swingItem()
{
	EntityPlayerSP::swingItem();
	sendQueue->addToSendQueue(new Packet18Animation(this, 1));
}

void EntityClientPlayerMP::respawnPlayer()
{
	sendInventoryChanged();
	sendQueue->addToSendQueue(new Packet9Respawn((byte_t)dimension));
}

void EntityClientPlayerMP::damageEntity(int_t i)
{
	health -= i;
}

void EntityClientPlayerMP::closeScreen()
{
	sendQueue->addToSendQueue(new Packet101CloseWindow(craftingInventory->windowId));
	inventory->setItemStack(nullptr);
	EntityPlayerSP::closeScreen();
}

void EntityClientPlayerMP::setHealth(int_t i)
{
	if (field_21093_bH)
		EntityPlayerSP::setHealth(i);
	else
	{
		health = i;
		field_21093_bH = true;
	}
}

void EntityClientPlayerMP::addStat(StatBase *statbase, int_t i)
{
	if (statbase == nullptr)
		return;
	EntityPlayerSP::addStat(statbase, i);
}

void EntityClientPlayerMP::func_27027_b(StatBase *statbase, int_t i)
{
	if (statbase == nullptr)
		return;
	EntityPlayerSP::addStat(statbase, i);
}
