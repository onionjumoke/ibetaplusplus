#pragma once

#include "PlayerController.h"

class NetClientHandler;

// net.minecraft.src.PlayerControllerMP
class PlayerControllerMP : public PlayerController
{
public:
	PlayerControllerMP(Minecraft *minecraft, NetClientHandler *netclienthandler);

	void flipPlayer(EntityPlayer *entityplayer) override;
	bool sendBlockRemoved(int_t i, int_t j, int_t k, int_t l) override;
	void clickBlock(int_t i, int_t j, int_t k, int_t l) override;
	void resetBlockRemoving() override;
	void sendBlockRemoving(int_t i, int_t j, int_t k, int_t l) override;
	void setPartialTime(float f) override;
	float getBlockReachDistance() override;
	void func_717_a(World *world) override;
	void updateController() override;
	bool sendPlaceBlock(EntityPlayer *entityplayer, World *world, ItemStack *itemstack, int_t i, int_t j, int_t k, int_t l) override;
	bool sendUseItem(EntityPlayer *entityplayer, World *world, ItemStack *itemstack) override;
	EntityPlayer *createPlayer(World *world) override;
	void attackEntity(EntityPlayer *entityplayer, Entity *entity) override;
	void interactWithEntity(EntityPlayer *entityplayer, Entity *entity) override;
	ItemStack *func_27174_a(int_t i, int_t j, int_t k, bool flag, EntityPlayer *entityplayer) override;
	void func_20086_a(int_t i, EntityPlayer *entityplayer) override;

private:
	void syncCurrentPlayItem();

	int_t currentBlockX;
	int_t currentBlockY;
	int_t currentblockZ;
	float curBlockDamageMP;
	float prevBlockDamageMP;
	float field_9441_h;
	int_t blockHitDelay;
	bool isHittingBlock;
	NetClientHandler *netClientHandler;
	int_t currentPlayerItem;
};
