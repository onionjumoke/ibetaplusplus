#pragma once

#include "EntityPlayer.h"

class IInventory;
class Minecraft;
class MovementInput;
class NBTTagCompound;
class Session;
class StatBase;
class TileEntitySign;
class TileEntityFurnace;
class TileEntityDispenser;

// net.minecraft.src.EntityPlayerSP
class EntityPlayerSP : public EntityPlayer
{
public:
	EntityPlayerSP(Minecraft *minecraft, World *world, Session *session, int_t i);
	~EntityPlayerSP() override;

	void moveEntity(double d, double d1, double d2) override;
	void updatePlayerActionState() override;
	void onLivingUpdate() override;
	void resetPlayerKeyState();
	void handleKeyPress(int_t i, bool flag);
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;
	void closeScreen() override;
	void displayGUIEditSign(TileEntitySign *tileentitysign) override;
	void displayGUIChest(IInventory *iinventory) override;
	void displayWorkbenchGUI(int_t i, int_t j, int_t k) override;
	void displayGUIFurnace(TileEntityFurnace *tileentityfurnace) override;
	void displayGUIDispenser(TileEntityDispenser *tileentitydispenser) override;
	void onItemPickup(Entity *entity, int_t i) override;
	int_t getPlayerArmorValue() override;
	virtual void sendChatMessage(const std::string &s);
	bool isSneaking() override;
	void setHealth(int_t i) override;
	void respawnPlayer() override;
	void func_6420_o() override;
	void addChatMessage(const std::string &s) override;
	void addChatMessage(const char16_t *s) override;
	void addStat(StatBase *statbase, int_t i) override;

private:
	bool isBlockTranslucent(int_t i, int_t j, int_t k);
	bool pushOutOfBlocks(double d, double d1, double d2);

public:
	MovementInput *movementInput;

protected:
	Minecraft *mc;
};
