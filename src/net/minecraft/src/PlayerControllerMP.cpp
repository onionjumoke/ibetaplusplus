#include "PlayerControllerMP.h"

#include <cmath>

#include "Block.h"
#include "Container.h"
#include "Entity.h"
#include "EntityClientPlayerMP.h"
#include "EntityPlayer.h"
#include "GuiIngame.h"
#include "InventoryPlayer.h"
#include "ItemStack.h"
#include "Minecraft.h"
#include "NetClientHandler.h"
#include "Packet102WindowClick.h"
#include "Packet14BlockDig.h"
#include "Packet15Place.h"
#include "Packet16BlockItemSwitch.h"
#include "Packet7UseEntity.h"
#include "RenderGlobal.h"
#include "Session.h"
#include "SoundManager.h"
#include "StepSound.h"
#include "World.h"

PlayerControllerMP::PlayerControllerMP(Minecraft *minecraft, NetClientHandler *netclienthandler)
	: PlayerController(minecraft)
	, currentBlockX(-1)
	, currentBlockY(-1)
	, currentblockZ(-1)
	, curBlockDamageMP(0.0f)
	, prevBlockDamageMP(0.0f)
	, field_9441_h(0.0f)
	, blockHitDelay(0)
	, isHittingBlock(false)
	, netClientHandler(netclienthandler)
	, currentPlayerItem(0)
{
}

void PlayerControllerMP::flipPlayer(EntityPlayer *entityplayer)
{
	entityplayer->rotationYaw = -180.0f;
}

bool PlayerControllerMP::sendBlockRemoved(int_t i, int_t j, int_t k, int_t l)
{
	int_t i1 = mc->theWorld->getBlockId(i, j, k);
	bool flag = PlayerController::sendBlockRemoved(i, j, k, l);
	ItemStack *itemstack = mc->thePlayer->getCurrentEquippedItem();
	if (itemstack != nullptr)
	{
		itemstack->onDestroyBlock(i1, i, j, k, mc->thePlayer);
		if (itemstack->stackSize == 0)
		{
			itemstack->func_1097_a(mc->thePlayer);
			mc->thePlayer->destroyCurrentEquippedItem();
		}
	}
	return flag;
}

void PlayerControllerMP::clickBlock(int_t i, int_t j, int_t k, int_t l)
{
	if (!isHittingBlock || i != currentBlockX || j != currentBlockY || k != currentblockZ)
	{
		netClientHandler->addToSendQueue(new Packet14BlockDig(0, i, j, k, l));
		int_t i1 = mc->theWorld->getBlockId(i, j, k);
		if (i1 > 0 && curBlockDamageMP == 0.0f)
			Block::blocksList[i1]->onBlockClicked(mc->theWorld, i, j, k, mc->thePlayer);
		if (i1 > 0 && Block::blocksList[i1]->blockStrength(mc->thePlayer) >= 1.0f)
			sendBlockRemoved(i, j, k, l);
		else
		{
			isHittingBlock = true;
			currentBlockX = i;
			currentBlockY = j;
			currentblockZ = k;
			curBlockDamageMP = 0.0f;
			prevBlockDamageMP = 0.0f;
			field_9441_h = 0.0f;
		}
	}
}

void PlayerControllerMP::resetBlockRemoving()
{
	curBlockDamageMP = 0.0f;
	isHittingBlock = false;
}

void PlayerControllerMP::sendBlockRemoving(int_t i, int_t j, int_t k, int_t l)
{
	if (!isHittingBlock)
		return;
	syncCurrentPlayItem();
	if (blockHitDelay > 0)
	{
		blockHitDelay--;
		return;
	}
	if (i == currentBlockX && j == currentBlockY && k == currentblockZ)
	{
		int_t i1 = mc->theWorld->getBlockId(i, j, k);
		if (i1 == 0)
		{
			isHittingBlock = false;
			return;
		}
		Block *block = Block::blocksList[i1];
		curBlockDamageMP += block->blockStrength(mc->thePlayer);
		if (std::fmod(field_9441_h, 4.0f) == 0.0f && block != nullptr)
			mc->sndManager->playSound(block->stepSound->getBreakSound(), (float)i + 0.5f, (float)j + 0.5f, (float)k + 0.5f, (block->stepSound->getVolume() + 1.0f) / 8.0f, block->stepSound->getPitch() * 0.5f);
		field_9441_h++;
		if (curBlockDamageMP >= 1.0f)
		{
			isHittingBlock = false;
			netClientHandler->addToSendQueue(new Packet14BlockDig(2, i, j, k, l));
			sendBlockRemoved(i, j, k, l);
			curBlockDamageMP = 0.0f;
			prevBlockDamageMP = 0.0f;
			field_9441_h = 0.0f;
			blockHitDelay = 5;
		}
	}
	else
	{
		clickBlock(i, j, k, l);
	}
}

void PlayerControllerMP::setPartialTime(float f)
{
	if (curBlockDamageMP <= 0.0f)
	{
		mc->ingameGUI->damageGuiPartialTime = 0.0f;
		mc->renderGlobal->damagePartialTime = 0.0f; // drives the block-cracking overlay in drawBlockBreaking
	}
	else
	{
		float f1 = prevBlockDamageMP + (curBlockDamageMP - prevBlockDamageMP) * f;
		mc->ingameGUI->damageGuiPartialTime = f1;
		mc->renderGlobal->damagePartialTime = f1;
	}
}

float PlayerControllerMP::getBlockReachDistance()
{
	return 4.0f;
}

void PlayerControllerMP::func_717_a(World *world)
{
	PlayerController::func_717_a(world);
}

void PlayerControllerMP::updateController()
{
	syncCurrentPlayItem();
	prevBlockDamageMP = curBlockDamageMP;
	mc->sndManager->playRandomMusicIfReady();
}

void PlayerControllerMP::syncCurrentPlayItem()
{
	int_t i = mc->thePlayer->inventory->currentItem;
	if (i != currentPlayerItem)
	{
		currentPlayerItem = i;
		netClientHandler->addToSendQueue(new Packet16BlockItemSwitch(currentPlayerItem));
	}
}

bool PlayerControllerMP::sendPlaceBlock(EntityPlayer *entityplayer, World *world, ItemStack *itemstack, int_t i, int_t j, int_t k, int_t l)
{
	syncCurrentPlayItem();
	netClientHandler->addToSendQueue(new Packet15Place(i, j, k, l, entityplayer->inventory->getCurrentItem()));
	return PlayerController::sendPlaceBlock(entityplayer, world, itemstack, i, j, k, l);
}

bool PlayerControllerMP::sendUseItem(EntityPlayer *entityplayer, World *world, ItemStack *itemstack)
{
	syncCurrentPlayItem();
	netClientHandler->addToSendQueue(new Packet15Place(-1, -1, -1, 255, entityplayer->inventory->getCurrentItem()));
	return PlayerController::sendUseItem(entityplayer, world, itemstack);
}

EntityPlayer *PlayerControllerMP::createPlayer(World *world)
{
	return new EntityClientPlayerMP(mc, world, mc->session, netClientHandler);
}

void PlayerControllerMP::attackEntity(EntityPlayer *entityplayer, Entity *entity)
{
	syncCurrentPlayItem();
	netClientHandler->addToSendQueue(new Packet7UseEntity(entityplayer->entityId, entity->entityId, 1));
	entityplayer->attackTargetEntityWithCurrentItem(entity);
}

void PlayerControllerMP::interactWithEntity(EntityPlayer *entityplayer, Entity *entity)
{
	syncCurrentPlayItem();
	netClientHandler->addToSendQueue(new Packet7UseEntity(entityplayer->entityId, entity->entityId, 0));
	entityplayer->useCurrentItemOnEntity(entity);
}

ItemStack *PlayerControllerMP::func_27174_a(int_t i, int_t j, int_t k, bool flag, EntityPlayer *entityplayer)
{
	short_t word0 = entityplayer->craftingInventory->getNextTransactionID(entityplayer->inventory);
	ItemStack *itemstack = PlayerController::func_27174_a(i, j, k, flag, entityplayer);
	netClientHandler->addToSendQueue(new Packet102WindowClick(i, j, k, flag, itemstack, word0));
	return itemstack;
}

void PlayerControllerMP::func_20086_a(int_t i, EntityPlayer *)
{
	if (i == -9999)
		return;
}
