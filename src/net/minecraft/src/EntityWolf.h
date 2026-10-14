#pragma once

#include "EntityAnimal.h"

class World;
class NBTTagCompound;
class Entity;
class EntityPlayer;

// net.minecraft.src.EntityWolf
class EntityWolf : public EntityAnimal
{
public:
	EntityWolf(World *world);

protected:
	void entityInit() override;
	bool canTriggerWalking() override;

public:
	jstring getEntityTexture() override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;

protected:
	bool canDespawn() override;
	jstring getLivingSound() override;
	jstring getHurtSound() override;
	jstring getDeathSound() override;
	float getSoundVolume() override;
	int_t getDropItemId() override;
	void updatePlayerActionState() override;

public:
	void onLivingUpdate() override;
	void onUpdate() override;

	bool getWolfShaking();
	float getShadingWhileShaking(float f);
	float getShakeAngle(float f, float f1);
	float getInterestedAngle(float f);

	float getEyeHeight() override;

protected:
	int_t func_25026_x() override;

private:
	void getPathOrWalkableBlock(Entity *entity, float f);

protected:
	bool isMovementCeased() override;

public:
	bool attackEntityFrom(Entity *entity, int_t i) override;

protected:
	Entity *findPlayerToAttack() override;
	void attackEntity(Entity *entity, float f) override;

public:
	bool interact(EntityPlayer *entityplayer) override;

private:
	void showHeartsOrSmokeFX(bool flag);

public:
	void handleHealthUpdate(byte_t byte0) override;
	float setTailRotation();
	int_t getMaxSpawnedInChunk() override;
	jstring getWolfOwner();
	void setWolfOwner(const jstring &s);
	bool isWolfSitting();
	void setWolfSitting(bool flag);
	bool isWolfAngry();
	void setWolfAngry(bool flag);
	bool isWolfTamed();
	void setWolfTamed(bool flag);

private:
	bool looksWithInterest;
	float field_25048_b;
	float field_25054_c;
	bool isWolfShaking;
	bool field_25052_g;
	float timeWolfIsShaking;
	float prevTimeWolfIsShaking;
};
