#pragma once

#include "EntityLiving.h"
#include "java/Type.h"
#include "java/String.h"

class World;
class InventoryPlayer;
class Container;
class ChunkCoordinates;
class Entity;
class EntityItem;
class EntityFish;
class EntityLiving;
class ItemStack;
class Block;
class NBTTagCompound;
class IInventory;
class TileEntityFurnace;
class TileEntityDispenser;
class TileEntitySign;
class StatBase;
enum class EnumStatus;

// net.minecraft.src.EntityPlayer
class EntityPlayer : public EntityLiving
{
public:
	EntityPlayer(World *world);
	~EntityPlayer() override;

protected:
	void entityInit() override;

public:
	void onUpdate() override;

protected:
	bool isMovementBlocked() override;

public:
	virtual void closeScreen();
	void updateCloak() override;
	void updateRidden() override;
	void preparePlayerToSpawn() { /* visibility */ EntityLiving::preparePlayerToSpawn(); }

protected:
	void updatePlayerActionState() override;

public:
	void onLivingUpdate() override;

private:
	void collideWithPlayer(Entity *entity);

public:
	int_t getScore();
	void onDeath(Entity *entity) override;
	void addToPlayerScore(Entity *entity, int_t i) override;
	virtual void dropCurrentItem();
	void dropPlayerItem(ItemStack *itemstack);
	void dropPlayerItemWithRandomChoice(ItemStack *itemstack, bool flag);

protected:
	virtual void joinEntityItemWithWorld(EntityItem *entityitem);

public:
	float getCurrentPlayerStrVsBlock(Block *block);
	bool canHarvestBlock(Block *block);

	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;

	virtual void displayGUIChest(IInventory *iinventory);
	virtual void displayWorkbenchGUI(int_t i, int_t j, int_t k);
	virtual void onItemPickup(Entity *entity, int_t i);

	float getEyeHeight() override;

protected:
	virtual void resetHeight();

public:
	bool attackEntityFrom(Entity *entity, int_t i) override;

protected:
	virtual bool func_27025_G();
	virtual void alertWolves(EntityLiving *entityliving, bool flag);
	void damageEntity(int_t i) override;

public:
	virtual void displayGUIFurnace(TileEntityFurnace *tileentityfurnace);
	virtual void displayGUIDispenser(TileEntityDispenser *tileentitydispenser);
	virtual void displayGUIEditSign(TileEntitySign *tileentitysign);
	void useCurrentItemOnEntity(Entity *entity);
	ItemStack *getCurrentEquippedItem();
	void destroyCurrentEquippedItem();
	double getYOffset() override;
	virtual void swingItem();
	void attackTargetEntityWithCurrentItem(Entity *entity);
	virtual void respawnPlayer();
	virtual void func_6420_o() = 0;
	virtual void onItemStackChanged(ItemStack *itemstack);
	void setEntityDead() { EntityLiving::setEntityDead(); /* overridden behavior */ }
	bool isEntityInsideOpaqueBlock();
	EnumStatus sleepInBedAt(int_t i, int_t j, int_t k);

private:
	void func_22052_e(int_t i);

public:
	void wakeUpPlayer(bool flag, bool flag1, bool flag2);

private:
	bool isInBed();

public:
	static ChunkCoordinates *func_25060_a(World *world, ChunkCoordinates *chunkcoordinates);
	float getBedOrientationInDegrees();
	bool isPlayerSleeping() override;
	bool isPlayerFullyAsleep();
	int_t func_22060_M();
	int_t getSleepTimer() const { return sleepTimer; }
	virtual void addChatMessage(const std::string &s);
	virtual void addChatMessage(const char16_t *s) {}
	ChunkCoordinates *getPlayerSpawnCoordinate();
	void setPlayerSpawnCoordinate(ChunkCoordinates *chunkcoordinates);
	void triggerAchievement(StatBase *statbase);
	virtual void addStat(StatBase *statbase, int_t i);

protected:
	void jump() override;

public:
	void moveEntityWithHeading(float f, float f1) override;

private:
	void addMovementStat(double d, double d1, double d2);
	void addMountedMovementStat(double d, double d1, double d2);

protected:
	void fall(float f) override;

public:
	void onKillEntity(EntityLiving *entityliving) override;
	int_t getItemIcon(ItemStack *itemstack) override;
	void setInPortal() override;

	virtual int_t getPlayerArmorValue() { return 0; }
	virtual void setHealth(int_t i) { health = i; }

	InventoryPlayer *inventory;
	Container *inventorySlots;
	Container *craftingInventory;
	byte_t  field_9371_f;
	int_t   score;
	float   field_775_e;
	float   field_774_f;
	bool    isSwinging;
	int_t   swingProgressInt;
	jstring username;
	int_t   dimension;
	jstring playerCloakUrl;
	double  field_20066_r;
	double  field_20065_s;
	double  field_20064_t;
	double  field_20063_u;
	double  field_20062_v;
	double  field_20061_w;
protected:
	bool    sleeping;
public:
	ChunkCoordinates *bedChunkCoordinates;
private:
	int_t   sleepTimer;
public:
	float   field_22063_x;
	float   field_22062_y;
	float   field_22061_z;
private:
	ChunkCoordinates *playerSpawnCoordinate;
	ChunkCoordinates *startMinecartRidingCoordinate;
public:
	int_t   timeUntilPortal;
protected:
	bool    inPortal;
public:
	float   timeInPortal;
	float   prevTimeInPortal;
private:
	int_t   damageRemainder;
public:
	EntityFish *fishEntity;
};
