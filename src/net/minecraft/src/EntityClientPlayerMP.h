#pragma once

#include "EntityPlayerSP.h"

class NetClientHandler;

// net.minecraft.src.EntityClientPlayerMP
class EntityClientPlayerMP : public EntityPlayerSP
{
public:
	EntityClientPlayerMP(Minecraft *minecraft, World *world, Session *session, NetClientHandler *netclienthandler);

	bool attackEntityFrom(Entity *entity, int_t i) override;
	void heal(int_t i) override;
	void onUpdate() override;
	void func_4056_N();
	void dropCurrentItem() override;
	void sendChatMessage(const std::string &s) override;
	void swingItem() override;
	void respawnPlayer() override;
	void damageEntity(int_t i) override;
	void closeScreen() override;
	void setHealth(int_t i) override;
	void addStat(StatBase *statbase, int_t i) override;
	void func_27027_b(StatBase *statbase, int_t i);

public:
	NetClientHandler *sendQueue;

private:
	void sendInventoryChanged();

	int_t field_9380_bx;
	bool field_21093_bH;
	double oldPosX;
	double field_9378_bz;
	double oldPosY;
	double oldPosZ;
	float oldRotationYaw;
	float oldRotationPitch;
	bool field_9382_bF;
	bool wasSneaking;
	int_t field_12242_bI;
};
