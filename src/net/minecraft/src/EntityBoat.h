#pragma once

#include "Entity.h"
#include "java/Type.h"

class World;
class NBTTagCompound;
class AxisAlignedBB;
class EntityPlayer;

// net.minecraft.src.EntityBoat
class EntityBoat : public Entity
{
public:
	EntityBoat(World *world);
	EntityBoat(World *world, double d, double d1, double d2);

protected:
	void entityInit() override;
	bool canTriggerWalking() override { return false; }

public:
	AxisAlignedBB *getCollisionBox(Entity *entity) override;
	AxisAlignedBB *getBoundingBox() override { return boundingBox; }
	bool canBeCollidedWith() override;
	bool attackEntityFrom(Entity *entity, int_t i) override;
	void performHurtAnimation() override;
	bool canBePushed() override { return true; }
	double getMountedYOffset() override;
	bool interact(EntityPlayer *entityplayer) override;
	void updateRiderPosition() override;

	int_t getForwardDirection() const  { return boatRockDirection; }
	void  setForwardDirection(int_t v) { boatRockDirection = v; }
	int_t getTimeSinceHit()    const   { return boatTimeSinceHit; }
	void  setTimeSinceHit(int_t v)     { boatTimeSinceHit = v; }
	int_t getDamageTaken()     const   { return boatCurrentDamage; }
	void  setDamageTaken(int_t v)      { boatCurrentDamage = v; }

protected:
	void setPositionAndRotation2(double d, double d1, double d2, float f, float f1, int_t i) override;

public:
	void setVelocity(double d, double d1, double d2) override;
	void onUpdate() override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;
	float getShadowSize() override;

	int_t boatCurrentDamage; // Java: boatCurrentDamage
	int_t boatTimeSinceHit;  // Java: boatTimeSinceHit
	int_t boatRockDirection; // Java: boatRockDirection

private:
	int_t boatTicks;        // field_9394_d
	double boatX;           // field_9393_e
	double boatY;           // field_9392_f
	double boatZ;           // field_9391_g
	double boatYaw;         // field_9390_h
	double boatPitch;       // field_9389_i
	double velocityX;       // field_9388_j
	double velocityY;       // field_9387_k
	double velocityZ;       // field_9386_l
};
