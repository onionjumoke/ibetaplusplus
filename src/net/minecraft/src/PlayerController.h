#pragma once

#include "java/Type.h"

class Minecraft;
class World;
class EntityPlayer;
class Entity;
class ItemStack;

// net.minecraft.src.PlayerController
class PlayerController
{
public:
	PlayerController(Minecraft *minecraft);
	virtual ~PlayerController() = default;

	virtual void func_717_a(World *world);
	virtual void clickBlock(int_t i, int_t j, int_t k, int_t l);
	virtual bool sendBlockRemoved(int_t i, int_t j, int_t k, int_t l);
	virtual void sendBlockRemoving(int_t i, int_t j, int_t k, int_t l);
	virtual void resetBlockRemoving();
	virtual void setPartialTime(float f);
	virtual float getBlockReachDistance();
	virtual bool sendUseItem(EntityPlayer *entityplayer, World *world, ItemStack *itemstack);
	virtual void flipPlayer(EntityPlayer *entityplayer);
	virtual void updateController();
	virtual bool shouldDrawHUD();
	virtual void func_6473_b(EntityPlayer *entityplayer);
	virtual bool sendPlaceBlock(EntityPlayer *entityplayer, World *world, ItemStack *itemstack, int_t i, int_t j, int_t k, int_t l);
	virtual EntityPlayer *createPlayer(World *world);
	virtual void interactWithEntity(EntityPlayer *entityplayer, Entity *entity);
	virtual void attackEntity(EntityPlayer *entityplayer, Entity *entity);
	virtual ItemStack *func_27174_a(int_t i, int_t j, int_t k, bool flag, EntityPlayer *entityplayer);
	virtual void func_20086_a(int_t i, EntityPlayer *entityplayer);
	virtual void windowClick(int_t windowId, int_t slotId, int_t button, bool shift, EntityPlayer *player);
	virtual void onContainerClosed(int_t windowId, EntityPlayer *player);

protected:
	Minecraft *mc;

public:
	bool field_1064_b;
};
