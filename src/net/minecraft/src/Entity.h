#pragma once

#include "java/Type.h"
#include "java/String.h"
#include "java/Random.h"

class AxisAlignedBB;
class DataWatcher;
class World;
class Material;
class Vec3D;
class NBTTagCompound;
class NBTTagList;
class ItemStack;
class EntityItem;
class EntityPlayer;
class EntityLightningBolt;
class EntityLiving;

// net.minecraft.src.Entity
class Entity
{
public:
	Entity(World *world);
	virtual ~Entity();

protected:
	virtual void entityInit() = 0;

	// Java calls entityInit() from the Entity constructor (virtual dispatch reaches the
	// subclass override). C++ can't virtual-dispatch from a base constructor, so each concrete
	// entity constructor must call ensureEntityInit() as its FIRST statement instead. The guard
	// makes it idempotent for delegating/multiple constructors (avoids DataWatcher duplicate-id).
	bool entityInitialized = false; // not in Java
	void ensureEntityInit()
	{
		if (!entityInitialized)
		{
			entityInitialized = true;
			entityInit();
		}
	}

public:
	DataWatcher *getDataWatcher();

	bool equals(Entity *obj);
	int_t hashCode();

protected:
	void preparePlayerToSpawn();

public:
	virtual void setEntityDead();

protected:
	void setSize(float f, float f1);
	void setRotation(float f, float f1);

public:
	virtual void setPosition(double d, double d1, double d2);
	void func_346_d(float f, float f1);

	virtual void onUpdate();
	virtual void onEntityUpdate();

protected:
	virtual void setOnFireFromLava();
	virtual void kill();

public:
	bool isOffsetPositionInLiquid(double d, double d1, double d2);
	virtual void moveEntity(double d, double d1, double d2);

protected:
	virtual bool canTriggerWalking();
	virtual void updateFallState(double d, bool flag);

public:
	virtual AxisAlignedBB *getBoundingBox();

protected:
	virtual void dealFireDamage(int_t i);
	virtual void fall(float f);

public:
	bool isWet();
	virtual bool isInWater();
	virtual bool handleWaterMovement();
	bool isInsideOfMaterial(Material *material);
	virtual float getEyeHeight();
	bool handleLavaMovement();
	void moveFlying(float f, float f1, float f2);
	virtual float getEntityBrightness(float f);
	void setWorld(World *world);
	void setPositionAndRotation(double d, double d1, double d2, float f, float f1);
	void setLocationAndAngles(double d, double d1, double d2, float f, float f1);
	void turnEntity(float yaw, float pitch);
	float getDistanceToEntity(Entity *entity);
	double getDistanceSq(double d, double d1, double d2);
	double getDistance(double d, double d1, double d2);
	double getDistanceSqToEntity(Entity *entity);
	virtual void onCollideWithPlayer(EntityPlayer *entityplayer);
	void applyEntityCollision(Entity *entity);
	void addVelocity(double d, double d1, double d2);

protected:
	void setBeenAttacked();

public:
	virtual bool attackEntityFrom(Entity *entity, int_t i);
	virtual bool canBeCollidedWith();
	virtual bool canBePushed();
	virtual void addToPlayerScore(Entity *entity, int_t i);
	bool isInRangeToRenderVec3D(Vec3D *vec3d);
	virtual bool isInRangeToRenderDist(double d);
	virtual jstring getEntityTexture();

	bool addEntityID(NBTTagCompound *nbttagcompound);
	void writeToNBT(NBTTagCompound *nbttagcompound);
	void readFromNBT(NBTTagCompound *nbttagcompound);

protected:
	jstring getEntityString();
	virtual void readEntityFromNBT(NBTTagCompound *nbttagcompound) = 0;
	virtual void writeEntityToNBT(NBTTagCompound *nbttagcompound) = 0;

	NBTTagList *newDoubleNBTList(const double *ad, int_t count);
	NBTTagList *newFloatNBTList(const float *af, int_t count);

public:
	virtual float getShadowSize();
	EntityItem *dropItem(int_t i, int_t j);
	EntityItem *dropItemWithOffset(int_t i, int_t j, float f);
	EntityItem *entityDropItem(ItemStack *itemstack, float f);
	bool isEntityAlive();
	bool isEntityInsideOpaqueBlock();
	virtual bool interact(EntityPlayer *entityplayer);
	virtual AxisAlignedBB *getCollisionBox(Entity *entity);
	virtual void updateRidden();
	virtual void updateRiderPosition();
	virtual double getYOffset();
	virtual double getMountedYOffset();
	void mountEntity(Entity *entity);
	virtual void setPositionAndRotation2(double d, double d1, double d2, float f, float f1, int_t i);
	virtual float getCollisionBorderSize();
	virtual Vec3D *getLookVec();
	virtual void setInPortal();
	virtual void setVelocity(double d, double d1, double d2);
	virtual void handleHealthUpdate(byte_t byte0);
	virtual void performHurtAnimation();
	virtual void updateCloak();
	virtual void outfitWithItem(int_t i, int_t j, int_t k);
	bool isBurning();
	bool isRiding();
	virtual bool isSneaking();

protected:
	bool getEntityFlag(int_t i);
	void setEntityFlag(int_t i, bool flag);

public:
	virtual void onStruckByLightning(EntityLightningBolt *entitylightningbolt);
	virtual void onKillEntity(EntityLiving *entityliving);

protected:
	bool pushOutOfBlocks(double d, double d1, double d2);

private:
	static int_t nextEntityID;

public:
	int_t entityId;
	double renderDistanceWeight;
	bool preventEntitySpawning;
	Entity *riddenByEntity;
	Entity *ridingEntity;
	World *worldObj;
	double prevPosX;
	double prevPosY;
	double prevPosZ;
	double posX;
	double posY;
	double posZ;
	double motionX;
	double motionY;
	double motionZ;
	float rotationYaw;
	float rotationPitch;
	float prevRotationYaw;
	float prevRotationPitch;
	AxisAlignedBB *const boundingBox;
	bool onGround;
	bool isCollidedHorizontally;
	bool isCollidedVertically;
	bool isCollided;
	bool beenAttacked;
	bool isInWeb;
	bool field_9293_aM;
	bool isDead;
	float yOffset;
	float width;
	float height;
	float prevDistanceWalkedModified;
	float distanceWalkedModified;
protected:
	float fallDistance;
private:
	int_t nextStepDistance;
public:
	double lastTickPosX;
	double lastTickPosY;
	double lastTickPosZ;
	float ySize;
	float stepHeight;
	bool noClip;
	float entityCollisionReduction;
protected:
	Random rand;
public:
	int_t ticksExisted;
	int_t fireResistance;
	int_t fire;
protected:
	int_t maxAir;
	bool inWater;
public:
	int_t heartsLife;
	int_t air;
private:
	bool isFirstUpdate;
public:
	jstring skinUrl;
	jstring cloakUrl;
protected:
	bool isImmuneToFire;
	DataWatcher *dataWatcher;
public:
	float entityBrightness;
private:
	double entityRiderPitchDelta;
	double entityRiderYawDelta;
public:
	bool addedToChunk;
	int_t chunkCoordX;
	int_t chunkCoordY;
	int_t chunkCoordZ;
	int_t serverPosX;
	int_t serverPosY;
	int_t serverPosZ;
	bool ignoreFrustumCheck;
};
