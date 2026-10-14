#pragma once

#include "Entity.h"
#include "java/Type.h"
#include "java/String.h"

class World;
class Vec3D;
class NBTTagCompound;
class ItemStack;
class MovingObjectPosition;
class EntityPlayer;

// net.minecraft.src.EntityLiving
class EntityLiving : public Entity
{
public:
	EntityLiving(World *world);

protected:
	void entityInit() override;

public:
	bool canEntityBeSeen(Entity *entity);
	jstring getEntityTexture() override;
	bool canBeCollidedWith() override;
	bool canBePushed() override;
	float getEyeHeight() override;

	virtual int_t getTalkInterval();
	virtual void playLivingSound();

	void onEntityUpdate() override;

	void spawnExplosionParticle();
	void updateRidden() override;
	void setPositionAndRotation2(double d, double d1, double d2, float f, float f1, int_t i);
	void onUpdate() override;

protected:
	void setSize(float f, float f1);

public:
	virtual void heal(int_t i);
	bool attackEntityFrom(Entity *entity, int_t i) override;
	void performHurtAnimation() override;

protected:
	virtual void damageEntity(int_t i);
	virtual float getSoundVolume();
	virtual jstring getLivingSound();
	virtual jstring getHurtSound();
	virtual jstring getDeathSound();

public:
	virtual void knockBack(Entity *entity, int_t i, double d, double d1);
	virtual void onDeath(Entity *entity);

protected:
	virtual void dropFewItems();
	virtual int_t getDropItemId();
	void fall(float f) override;

public:
	virtual void moveEntityWithHeading(float f, float f1);
	virtual bool isOnLadder();

	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;
	bool isEntityAlive();
	virtual bool canBreatheUnderwater();
	virtual void onLivingUpdate();

protected:
	virtual bool isMovementBlocked();
	virtual void jump();
	virtual bool canDespawn();
	virtual void func_27021_X();
	virtual void updatePlayerActionState();
	virtual int_t func_25026_x();

public:
	void faceEntity(Entity *entity, float f, float f1);
	bool hasCurrentTarget();
	Entity *getCurrentTarget();

private:
	float updateRotation(float f, float f1, float f2);

public:
	virtual void onEntityDeath();
	virtual bool getCanSpawnHere();

protected:
	void kill() override;

public:
	float getSwingProgress(float f);
	Vec3D *getPosition(float f);
	Vec3D *getLookVec() override;
	Vec3D *getLook(float f);
	MovingObjectPosition *rayTrace(double d, float f);
	virtual int_t getMaxSpawnedInChunk();
	virtual ItemStack *getHeldItem();
	void handleHealthUpdate(byte_t byte0) override;
	virtual bool isPlayerSleeping();
	virtual int_t getItemIcon(ItemStack *itemstack);

public:
	int_t  heartsHalvesLife;
	float  field_9365_p;
	float  field_9363_r;
	float  renderYawOffset;
	float  prevRenderYawOffset;
protected:
	float  field_9362_u;
	float  field_9361_v;
	float  field_9360_w;
	float  field_9359_x;
	bool   field_9358_y;
	jstring texture;
	bool   field_9355_A;
	float  field_9353_B;
	jstring field_9351_C;
	float  field_9349_D;
	int_t  scoreValue;
	float  field_9345_F;
public:
	bool   isMultiplayerEntity;
	float  prevSwingProgress;
	float  swingProgress;
	int_t  health;
	int_t  prevHealth;
private:
	int_t  livingSoundTime;
public:
	int_t  hurtTime;
	int_t  maxHurtTime;
	float  attackedAtYaw;
	int_t  deathTime;
	int_t  attackTime;
	float  cameraPitch;
	float  field_9328_R;
protected:
	bool   unused_flag;
public:
	int_t  field_9326_T;
	float  field_9325_U;
	float  field_705_Q;
	float  field_704_R;
	float  field_703_S;
protected:
	int_t  newPosRotationIncrements;
	double newPosX;
	double newPosY;
	double newPosZ;
	double newRotationYaw;
	double newRotationPitch;
	float  field_9348_ae;
	int_t  field_9346_af;
	int_t  entityAge;
public:
	float  moveStrafing;
	float  moveForward;
protected:
	float  randomYawVelocity;
public:
	bool   isJumping;
protected:
	float  defaultPitch;
	float  moveSpeed;
private:
	Entity *currentTarget;
protected:
	int_t  numTicksToChaseTarget;
};
