#include "EntityLiving.h"

#include <cmath>
#include <cstdlib>
#include <random>
#include <vector>

#include "AxisAlignedBB.h"
#include "Block.h"
#include "EntityPlayer.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "Material.h"
#include "MovingObjectPosition.h"
#include "NBTTagCompound.h"
#include "StepSound.h"
#include "Vec3D.h"
#include "World.h"

static inline double mathRandom_living()
{
	return (double)std::rand() / (double)RAND_MAX;
}

static inline double nextGaussian()
{
	static std::mt19937 gen{std::random_device{}()};
	static std::normal_distribution<double> dist(0.0, 1.0);
	return dist(gen);
}

EntityLiving::EntityLiving(World *world)
	: Entity(world)
{
	heartsHalvesLife = 20;
	renderYawOffset = 0.0f;
	prevRenderYawOffset = 0.0f;
	field_9362_u = 0.0f;
	field_9361_v = 0.0f;
	field_9360_w = 0.0f;
	field_9359_x = 0.0f;
	field_9358_y = true;
	texture = String::fromUTF8("/mob/char.png");
	field_9355_A = true;
	field_9353_B = 0.0f;
	field_9351_C = jstring();
	field_9349_D = 1.0f;
	scoreValue = 0;
	field_9345_F = 0.0f;
	isMultiplayerEntity = false;
	prevSwingProgress = 0.0f;
	swingProgress = 0.0f;
	livingSoundTime = 0;
	hurtTime = 0;
	maxHurtTime = 0;
	attackedAtYaw = 0.0f;
	deathTime = 0;
	attackTime = 0;
	cameraPitch = 0.0f;
	field_9328_R = 0.0f;
	unused_flag = false;
	field_9326_T = -1;
	field_9325_U = (float)(mathRandom_living() * 0.89999997615814209 + 0.10000000149011612);
	field_705_Q = 0.0f;
	field_704_R = 0.0f;
	field_703_S = 0.0f;
	newPosRotationIncrements = 0;
	newPosX = newPosY = newPosZ = 0.0;
	newRotationYaw = newRotationPitch = 0.0;
	field_9348_ae = 0.0f;
	field_9346_af = 0;
	entityAge = 0;
	isJumping = false;
	moveStrafing = 0.0f;
	moveForward = 0.0f;
	randomYawVelocity = 0.0f;
	defaultPitch = 0.0f;
	moveSpeed = 0.7f;
	currentTarget = nullptr;
	numTicksToChaseTarget = 0;
	health = 10;
	prevHealth = 0;
	preventEntitySpawning = true;
	field_9363_r = (float)(mathRandom_living() + 1.0) * 0.01f;
	setPosition(posX, posY, posZ);
	field_9365_p = (float)mathRandom_living() * 12398.0f;
	rotationYaw = (float)(mathRandom_living() * 3.1415927410125732 * 2.0);
	stepHeight = 0.5f;
}

void EntityLiving::entityInit()
{
}

bool EntityLiving::canEntityBeSeen(Entity *entity)
{
	return worldObj->rayTraceBlocks(Vec3D::createVector(posX, posY + (double)getEyeHeight(), posZ),
	                                Vec3D::createVector(entity->posX, entity->posY + (double)entity->getEyeHeight(), entity->posZ)) == nullptr;
}

jstring EntityLiving::getEntityTexture()
{
	return texture;
}

bool EntityLiving::canBeCollidedWith()
{
	return !isDead;
}

bool EntityLiving::canBePushed()
{
	return !isDead;
}

float EntityLiving::getEyeHeight()
{
	return height * 0.85f;
}

int_t EntityLiving::getTalkInterval()
{
	return 80;
}

void EntityLiving::playLivingSound()
{
	jstring s = getLivingSound();
	if (!s.empty())
	{
		worldObj->playSoundAtEntity(this, s, getSoundVolume(), (rand.nextFloat() - rand.nextFloat()) * 0.2f + 1.0f);
	}
}

void EntityLiving::onEntityUpdate()
{
	prevSwingProgress = swingProgress;
	Entity::onEntityUpdate();
	if (rand.nextInt(1000) < livingSoundTime++)
	{
		livingSoundTime = -getTalkInterval();
		playLivingSound();
	}
	if (isEntityAlive() && isEntityInsideOpaqueBlock())
	{
		attackEntityFrom(nullptr, 1);
	}
	if (isImmuneToFire || worldObj->multiplayerWorld)
	{
		fire = 0;
	}
	if (isEntityAlive() && isInsideOfMaterial(Material::water) && !canBreatheUnderwater())
	{
		air--;
		if (air == -20)
		{
			air = 0;
			for (int_t i = 0; i < 8; i++)
			{
				float f  = rand.nextFloat() - rand.nextFloat();
				float f1 = rand.nextFloat() - rand.nextFloat();
				float f2 = rand.nextFloat() - rand.nextFloat();
				worldObj->spawnParticle(u"bubble", posX + (double)f, posY + (double)f1, posZ + (double)f2, motionX, motionY, motionZ);
			}
			attackEntityFrom(nullptr, 2);
		}
		fire = 0;
	}
	else
	{
		air = maxAir;
	}
	cameraPitch = field_9328_R;
	if (attackTime > 0)
	{
		attackTime--;
	}
	if (hurtTime > 0)
	{
		hurtTime--;
	}
	if (heartsLife > 0)
	{
		heartsLife--;
	}
	if (health <= 0)
	{
		deathTime++;
		if (deathTime > 20)
		{
			onEntityDeath();
			setEntityDead();
			for (int_t j = 0; j < 20; j++)
			{
				double d  = nextGaussian() * 0.02;
				double d1 = nextGaussian() * 0.02;
				double d2 = nextGaussian() * 0.02;
				worldObj->spawnParticle(u"explode",
					(posX + (double)(rand.nextFloat() * width * 2.0f)) - (double)width,
					posY + (double)(rand.nextFloat() * height),
					(posZ + (double)(rand.nextFloat() * width * 2.0f)) - (double)width,
					d, d1, d2);
			}
		}
	}
	field_9359_x = field_9360_w;
	prevRenderYawOffset = renderYawOffset;
	prevRotationYaw = rotationYaw;
	prevRotationPitch = rotationPitch;
}

void EntityLiving::spawnExplosionParticle()
{
	for (int_t i = 0; i < 20; i++)
	{
		double d  = nextGaussian() * 0.02;
		double d1 = nextGaussian() * 0.02;
		double d2 = nextGaussian() * 0.02;
		double d3 = 10.0;
		worldObj->spawnParticle(u"explode",
			(posX + (double)(rand.nextFloat() * width * 2.0f)) - (double)width - d * d3,
			(posY + (double)(rand.nextFloat() * height)) - d1 * d3,
			(posZ + (double)(rand.nextFloat() * width * 2.0f)) - (double)width - d2 * d3,
			d, d1, d2);
	}
}

void EntityLiving::updateRidden()
{
	Entity::updateRidden();
	field_9362_u = field_9361_v;
	field_9361_v = 0.0f;
}

void EntityLiving::setPositionAndRotation2(double d, double d1, double d2, float f, float f1, int_t i)
{
	yOffset = 0.0f;
	newPosX = d;
	newPosY = d1;
	newPosZ = d2;
	newRotationYaw = f;
	newRotationPitch = f1;
	newPosRotationIncrements = i;
}

void EntityLiving::onUpdate()
{
	Entity::onUpdate();
	onLivingUpdate();
	double d  = posX - prevPosX;
	double d1 = posZ - prevPosZ;
	float f  = MathHelper::sqrt_double(d * d + d1 * d1);
	float f1 = renderYawOffset;
	float f2 = 0.0f;
	field_9362_u = field_9361_v;
	float f3 = 0.0f;
	if (f > 0.05f)
	{
		f3 = 1.0f;
		f2 = f * 3.0f;
		f1 = ((float)std::atan2(d1, d) * 180.0f) / 3.141593f - 90.0f;
	}
	if (swingProgress > 0.0f)
	{
		f1 = rotationYaw;
	}
	if (!onGround)
	{
		f3 = 0.0f;
	}
	field_9361_v = field_9361_v + (f3 - field_9361_v) * 0.3f;
	float f4;
	for (f4 = f1 - renderYawOffset; f4 < -180.0f; f4 += 360.0f) {}
	for (;                          f4 >= 180.0f;  f4 -= 360.0f) {}
	renderYawOffset += f4 * 0.3f;
	float f5;
	for (f5 = rotationYaw - renderYawOffset; f5 < -180.0f; f5 += 360.0f) {}
	for (;                                   f5 >= 180.0f;  f5 -= 360.0f) {}
	bool flag = f5 < -90.0f || f5 >= 90.0f;
	if (f5 < -75.0f) { f5 = -75.0f; }
	if (f5 >= 75.0f) { f5 =  75.0f; }
	renderYawOffset = rotationYaw - f5;
	if (f5 * f5 > 2500.0f)
	{
		renderYawOffset += f5 * 0.2f;
	}
	if (flag)
	{
		f2 *= -1.0f;
	}
	for (; rotationYaw   - prevRotationYaw   < -180.0f; prevRotationYaw   -= 360.0f) {}
	for (; rotationYaw   - prevRotationYaw   >= 180.0f; prevRotationYaw   += 360.0f) {}
	for (; renderYawOffset - prevRenderYawOffset < -180.0f; prevRenderYawOffset -= 360.0f) {}
	for (; renderYawOffset - prevRenderYawOffset >= 180.0f; prevRenderYawOffset += 360.0f) {}
	for (; rotationPitch - prevRotationPitch < -180.0f; prevRotationPitch -= 360.0f) {}
	for (; rotationPitch - prevRotationPitch >= 180.0f; prevRotationPitch += 360.0f) {}
	field_9360_w += f2;
}

void EntityLiving::setSize(float f, float f1)
{
	Entity::setSize(f, f1);
}

void EntityLiving::heal(int_t i)
{
	if (health <= 0)
	{
		return;
	}
	health += i;
	if (health > 20)
	{
		health = 20;
	}
	heartsLife = heartsHalvesLife / 2;
}

bool EntityLiving::attackEntityFrom(Entity *entity, int_t i)
{
	if (worldObj->multiplayerWorld)
	{
		return false;
	}
	entityAge = 0;
	if (health <= 0)
	{
		return false;
	}
	field_704_R = 1.5f;
	bool flag = true;
	if ((float)heartsLife > (float)heartsHalvesLife / 2.0f)
	{
		if (i <= field_9346_af)
		{
			return false;
		}
		damageEntity(i - field_9346_af);
		field_9346_af = i;
		flag = false;
	}
	else
	{
		field_9346_af = i;
		prevHealth = health;
		heartsLife = heartsHalvesLife;
		damageEntity(i);
		hurtTime = maxHurtTime = 10;
	}
	attackedAtYaw = 0.0f;
	if (flag)
	{
		worldObj->setEntityState(this, (unsigned char)2);
		setBeenAttacked();
		if (entity != nullptr)
		{
			double d = entity->posX - posX;
			double d1;
			for (d1 = entity->posZ - posZ; d * d + d1 * d1 < 0.0001; d1 = (mathRandom_living() - mathRandom_living()) * 0.01)
			{
				d = (mathRandom_living() - mathRandom_living()) * 0.01;
			}
			attackedAtYaw = (float)((std::atan2(d1, d) * 180.0) / 3.1415927410125732) - rotationYaw;
			knockBack(entity, i, d, d1);
		}
		else
		{
			attackedAtYaw = (float)((int_t)(mathRandom_living() * 2.0) * 180);
		}
	}
	if (health <= 0)
	{
		if (flag)
		{
			worldObj->playSoundAtEntity(this, getDeathSound(), getSoundVolume(), (rand.nextFloat() - rand.nextFloat()) * 0.2f + 1.0f);
		}
		onDeath(entity);
	}
	else if (flag)
	{
		worldObj->playSoundAtEntity(this, getHurtSound(), getSoundVolume(), (rand.nextFloat() - rand.nextFloat()) * 0.2f + 1.0f);
	}
	return true;
}

void EntityLiving::performHurtAnimation()
{
	hurtTime = maxHurtTime = 10;
	attackedAtYaw = 0.0f;
}

void EntityLiving::damageEntity(int_t i)
{
	health -= i;
}

float EntityLiving::getSoundVolume()
{
	return 1.0f;
}

jstring EntityLiving::getLivingSound()
{
	return jstring();
}

jstring EntityLiving::getHurtSound()
{
	return u"random.hurt";
}

jstring EntityLiving::getDeathSound()
{
	return u"random.hurt";
}

void EntityLiving::knockBack(Entity *entity, int_t i, double d, double d1)
{
	float f = MathHelper::sqrt_double(d * d + d1 * d1);
	float f1 = 0.4f;
	motionX /= 2.0;
	motionY /= 2.0;
	motionZ /= 2.0;
	motionX -= (d  / (double)f) * (double)f1;
	motionY += 0.40000000596046448;
	motionZ -= (d1 / (double)f) * (double)f1;
	if (motionY > 0.40000000596046448)
	{
		motionY = 0.40000000596046448;
	}
}

void EntityLiving::onDeath(Entity *entity)
{
	if (scoreValue >= 0 && entity != nullptr)
	{
		entity->addToPlayerScore(this, scoreValue);
	}
	if (entity != nullptr)
	{
		entity->onKillEntity(this);
	}
	unused_flag = true;
	if (!worldObj->multiplayerWorld)
	{
		dropFewItems();
	}
	worldObj->setEntityState(this, (unsigned char)3);
}

void EntityLiving::dropFewItems()
{
	int_t i = getDropItemId();
	if (i > 0)
	{
		int_t j = rand.nextInt(3);
		for (int_t k = 0; k < j; k++)
		{
			dropItem(i, 1);
		}
	}
}

int_t EntityLiving::getDropItemId()
{
	return 0;
}

void EntityLiving::fall(float f)
{
	Entity::fall(f);
	int_t i = (int_t)std::ceil(f - 3.0f);
	if (i > 0)
	{
		attackEntityFrom(nullptr, i);
		int_t j = worldObj->getBlockId(MathHelper::floor_double(posX), MathHelper::floor_double(posY - 0.20000000298023224 - (double)yOffset), MathHelper::floor_double(posZ));
		if (j > 0)
		{
			StepSound *stepsound = Block::blocksList[j]->stepSound;
			worldObj->playSoundAtEntity(this, stepsound->stepSoundDir(), stepsound->getVolume() * 0.5f, stepsound->getPitch() * 0.75f);
		}
	}
}

void EntityLiving::moveEntityWithHeading(float f, float f1)
{
	if (isInWater())
	{
		double d = posY;
		moveFlying(f, f1, 0.02f);
		moveEntity(motionX, motionY, motionZ);
		motionX *= 0.80000001192092896;
		motionY *= 0.80000001192092896;
		motionZ *= 0.80000001192092896;
		motionY -= 0.02;
		if (isCollidedHorizontally && isOffsetPositionInLiquid(motionX, ((motionY + 0.60000002384185791) - posY) + d, motionZ))
		{
			motionY = 0.30000001192092896;
		}
	}
	else if (handleLavaMovement())
	{
		double d1 = posY;
		moveFlying(f, f1, 0.02f);
		moveEntity(motionX, motionY, motionZ);
		motionX *= 0.5;
		motionY *= 0.5;
		motionZ *= 0.5;
		motionY -= 0.02;
		if (isCollidedHorizontally && isOffsetPositionInLiquid(motionX, ((motionY + 0.60000002384185791) - posY) + d1, motionZ))
		{
			motionY = 0.30000001192092896;
		}
	}
	else
	{
		float f2 = 0.91f;
		if (onGround)
		{
			f2 = 0.5460001f;
			int_t i = worldObj->getBlockId(MathHelper::floor_double(posX), MathHelper::floor_double(boundingBox->minY) - 1, MathHelper::floor_double(posZ));
			if (i > 0)
			{
				f2 = Block::blocksList[i]->slipperiness * 0.91f;
			}
		}
		float f3 = 0.1627714f / (f2 * f2 * f2);
		moveFlying(f, f1, onGround ? 0.1f * f3 : 0.02f);
		f2 = 0.91f;
		if (onGround)
		{
			f2 = 0.5460001f;
			int_t j = worldObj->getBlockId(MathHelper::floor_double(posX), MathHelper::floor_double(boundingBox->minY) - 1, MathHelper::floor_double(posZ));
			if (j > 0)
			{
				f2 = Block::blocksList[j]->slipperiness * 0.91f;
			}
		}
		if (isOnLadder())
		{
			float f4 = 0.15f;
			if (motionX < (double)(-f4)) { motionX = -f4; }
			if (motionX >  (double)f4)   { motionX =  f4; }
			if (motionZ < (double)(-f4)) { motionZ = -f4; }
			if (motionZ >  (double)f4)   { motionZ =  f4; }
			fallDistance = 0.0f;
			if (motionY < -0.14999999999999999)
			{
				motionY = -0.14999999999999999;
			}
			if (isSneaking() && motionY < 0.0)
			{
				motionY = 0.0;
			}
		}
		moveEntity(motionX, motionY, motionZ);
		if (isCollidedHorizontally && isOnLadder())
		{
			motionY = 0.2;
		}
		motionY -= 0.08;
		motionY *= 0.98000001907348633;
		motionX *= f2;
		motionZ *= f2;
	}
	field_705_Q = field_704_R;
	double d2 = posX - prevPosX;
	double d3 = posZ - prevPosZ;
	float f5 = MathHelper::sqrt_double(d2 * d2 + d3 * d3) * 4.0f;
	if (f5 > 1.0f) { f5 = 1.0f; }
	field_704_R += (f5 - field_704_R) * 0.4f;
	field_703_S += field_704_R;
}

bool EntityLiving::isOnLadder()
{
	int_t i = MathHelper::floor_double(posX);
	int_t j = MathHelper::floor_double(boundingBox->minY);
	int_t k = MathHelper::floor_double(posZ);
	return worldObj->getBlockId(i, j, k) == Block::ladder->blockID;
}

void EntityLiving::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	nbttagcompound->setShort(u"Health",     (short_t)health);
	nbttagcompound->setShort(u"HurtTime",   (short_t)hurtTime);
	nbttagcompound->setShort(u"DeathTime",  (short_t)deathTime);
	nbttagcompound->setShort(u"AttackTime", (short_t)attackTime);
}

void EntityLiving::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	health = nbttagcompound->getShort(u"Health");
	if (!nbttagcompound->hasKey(u"Health"))
	{
		health = 10;
	}
	hurtTime   = nbttagcompound->getShort(u"HurtTime");
	deathTime  = nbttagcompound->getShort(u"DeathTime");
	attackTime = nbttagcompound->getShort(u"AttackTime");
}

bool EntityLiving::isEntityAlive()
{
	return !isDead && health > 0;
}

bool EntityLiving::canBreatheUnderwater()
{
	return false;
}

void EntityLiving::onLivingUpdate()
{
	if (newPosRotationIncrements > 0)
	{
		double d  = posX + (newPosX - posX) / (double)newPosRotationIncrements;
		double d1 = posY + (newPosY - posY) / (double)newPosRotationIncrements;
		double d2 = posZ + (newPosZ - posZ) / (double)newPosRotationIncrements;
		double d3;
		for (d3 = newRotationYaw - (double)rotationYaw; d3 < -180.0; d3 += 360.0) {}
		for (;                                          d3 >= 180.0; d3 -= 360.0) {}
		rotationYaw += (float)(d3 / (double)newPosRotationIncrements);
		rotationPitch += (float)((newRotationPitch - (double)rotationPitch) / (double)newPosRotationIncrements);
		newPosRotationIncrements--;
		setPosition(d, d1, d2);
		setRotation(rotationYaw, rotationPitch);
		std::vector<AxisAlignedBB *> list1 = worldObj->getCollidingBoundingBoxes(this, boundingBox->contract(0.03125, 0.0, 0.03125));
		if (list1.size() > 0)
		{
			double d4 = 0.0;
			for (size_t j = 0; j < list1.size(); j++)
			{
				AxisAlignedBB *axisalignedbb = list1[j];
				if (axisalignedbb->maxY > d4)
				{
					d4 = axisalignedbb->maxY;
				}
			}
			d1 += d4 - boundingBox->minY;
			setPosition(d, d1, d2);
		}
	}
	if (isMovementBlocked())
	{
		isJumping = false;
		moveStrafing = 0.0f;
		moveForward = 0.0f;
		randomYawVelocity = 0.0f;
	}
	else if (!isMultiplayerEntity)
	{
		updatePlayerActionState();
	}
	bool flag  = isInWater();
	bool flag1 = handleLavaMovement();
	if (isJumping)
	{
		if (flag)
		{
			motionY += 0.039999999105930328;
		}
		else if (flag1)
		{
			motionY += 0.039999999105930328;
		}
		else if (onGround)
		{
			jump();
		}
	}
	moveStrafing *= 0.98f;
	moveForward *= 0.98f;
	randomYawVelocity *= 0.9f;
	moveEntityWithHeading(moveStrafing, moveForward);
	std::vector<Entity *> list = worldObj->getEntitiesWithinAABBExcludingEntity(this, boundingBox->expand(0.20000000298023224, 0.0, 0.20000000298023224));
	if (list.size() > 0)
	{
		for (size_t i = 0; i < list.size(); i++)
		{
			Entity *entity = list[i];
			if (entity->canBePushed())
			{
				entity->applyEntityCollision(this);
			}
		}
	}
}

bool EntityLiving::isMovementBlocked()
{
	return health <= 0;
}

void EntityLiving::jump()
{
	motionY = 0.41999998688697815;
}

bool EntityLiving::canDespawn()
{
	return true;
}

void EntityLiving::func_27021_X()
{
	EntityPlayer *entityplayer = worldObj->getClosestPlayerToEntity(this, -1.0);
	if (canDespawn() && entityplayer != nullptr)
	{
		double d  = ((Entity *)entityplayer)->posX - posX;
		double d1 = ((Entity *)entityplayer)->posY - posY;
		double d2 = ((Entity *)entityplayer)->posZ - posZ;
		double d3 = d * d + d1 * d1 + d2 * d2;
		if (d3 > 16384.0)
		{
			setEntityDead();
		}
		if (entityAge > 600 && rand.nextInt(800) == 0)
		{
			if (d3 < 1024.0)
			{
				entityAge = 0;
			}
			else
			{
				setEntityDead();
			}
		}
	}
}

void EntityLiving::updatePlayerActionState()
{
	entityAge++;
	(void)worldObj->getClosestPlayerToEntity(this, -1.0);
	func_27021_X();
	moveStrafing = 0.0f;
	moveForward = 0.0f;
	float f = 8.0f;
	if (rand.nextFloat() < 0.02f)
	{
		EntityPlayer *entityplayer1 = worldObj->getClosestPlayerToEntity(this, f);
		if (entityplayer1 != nullptr)
		{
			currentTarget = entityplayer1;
			numTicksToChaseTarget = 10 + rand.nextInt(20);
		}
		else
		{
			randomYawVelocity = (rand.nextFloat() - 0.5f) * 20.0f;
		}
	}
	if (currentTarget != nullptr)
	{
		faceEntity(currentTarget, 10.0f, (float)func_25026_x());
		if (numTicksToChaseTarget-- <= 0 || currentTarget->isDead || currentTarget->getDistanceSqToEntity(this) > (double)(f * f))
		{
			currentTarget = nullptr;
		}
	}
	else
	{
		if (rand.nextFloat() < 0.05f)
		{
			randomYawVelocity = (rand.nextFloat() - 0.5f) * 20.0f;
		}
		rotationYaw += randomYawVelocity;
		rotationPitch = defaultPitch;
	}
	bool flag  = isInWater();
	bool flag1 = handleLavaMovement();
	if (flag || flag1)
	{
		isJumping = rand.nextFloat() < 0.8f;
	}
}

int_t EntityLiving::func_25026_x()
{
	return 40;
}

void EntityLiving::faceEntity(Entity *entity, float f, float f1)
{
	double d  = entity->posX - posX;
	double d2 = entity->posZ - posZ;
	double d1;
	if (dynamic_cast<EntityLiving *>(entity) != nullptr)
	{
		EntityLiving *entityliving = (EntityLiving *)entity;
		d1 = (posY + (double)getEyeHeight()) - (entityliving->posY + (double)entityliving->getEyeHeight());
	}
	else
	{
		d1 = (entity->boundingBox->minY + entity->boundingBox->maxY) / 2.0 - (posY + (double)getEyeHeight());
	}
	double d3 = MathHelper::sqrt_double(d * d + d2 * d2);
	float f2 = (float)((std::atan2(d2, d) * 180.0) / 3.1415927410125732) - 90.0f;
	float f3 = (float)(-((std::atan2(d1, d3) * 180.0) / 3.1415927410125732));
	rotationPitch = -updateRotation(rotationPitch, f3, f1);
	rotationYaw   =  updateRotation(rotationYaw,   f2, f);
}

bool EntityLiving::hasCurrentTarget()
{
	return currentTarget != nullptr;
}

Entity *EntityLiving::getCurrentTarget()
{
	return currentTarget;
}

float EntityLiving::updateRotation(float f, float f1, float f2)
{
	float f3;
	for (f3 = f1 - f; f3 < -180.0f; f3 += 360.0f) {}
	for (;             f3 >= 180.0f; f3 -= 360.0f) {}
	if (f3 >  f2) { f3 =  f2; }
	if (f3 < -f2) { f3 = -f2; }
	return f + f3;
}

void EntityLiving::onEntityDeath()
{
}

bool EntityLiving::getCanSpawnHere()
{
	return worldObj->checkIfAABBIsClear(boundingBox) && worldObj->getCollidingBoundingBoxes(this, boundingBox).size() == 0 && !worldObj->getIsAnyLiquid(boundingBox);
}

void EntityLiving::kill()
{
	attackEntityFrom(nullptr, 4);
}

float EntityLiving::getSwingProgress(float f)
{
	float f1 = swingProgress - prevSwingProgress;
	if (f1 < 0.0f)
	{
		f1++;
	}
	return prevSwingProgress + f1 * f;
}

Vec3D *EntityLiving::getPosition(float f)
{
	if (f == 1.0f)
	{
		return Vec3D::createVector(posX, posY, posZ);
	}
	double d  = prevPosX + (posX - prevPosX) * (double)f;
	double d1 = prevPosY + (posY - prevPosY) * (double)f;
	double d2 = prevPosZ + (posZ - prevPosZ) * (double)f;
	return Vec3D::createVector(d, d1, d2);
}

Vec3D *EntityLiving::getLookVec()
{
	return getLook(1.0f);
}

Vec3D *EntityLiving::getLook(float f)
{
	if (f == 1.0f)
	{
		float f1 = MathHelper::cos(-rotationYaw   * 0.01745329f - 3.141593f);
		float f3 = MathHelper::sin(-rotationYaw   * 0.01745329f - 3.141593f);
		float f5 = -MathHelper::cos(-rotationPitch * 0.01745329f);
		float f7 = MathHelper::sin(-rotationPitch * 0.01745329f);
		return Vec3D::createVector(f3 * f5, f7, f1 * f5);
	}
	float f2 = prevRotationPitch + (rotationPitch - prevRotationPitch) * f;
	float f4 = prevRotationYaw   + (rotationYaw   - prevRotationYaw)   * f;
	float f6 = MathHelper::cos(-f4 * 0.01745329f - 3.141593f);
	float f8 = MathHelper::sin(-f4 * 0.01745329f - 3.141593f);
	float f9 = -MathHelper::cos(-f2 * 0.01745329f);
	float f10 = MathHelper::sin(-f2 * 0.01745329f);
	return Vec3D::createVector(f8 * f9, f10, f6 * f9);
}

MovingObjectPosition *EntityLiving::rayTrace(double d, float f)
{
	Vec3D *vec3d  = getPosition(f);
	Vec3D *vec3d1 = getLook(f);
	Vec3D *vec3d2 = vec3d->addVector(vec3d1->xCoord * d, vec3d1->yCoord * d, vec3d1->zCoord * d);
	return worldObj->rayTraceBlocks(vec3d, vec3d2);
}

int_t EntityLiving::getMaxSpawnedInChunk()
{
	return 4;
}

ItemStack *EntityLiving::getHeldItem()
{
	return nullptr;
}

void EntityLiving::handleHealthUpdate(byte_t byte0)
{
	if (byte0 == 2)
	{
		field_704_R = 1.5f;
		heartsLife = heartsHalvesLife;
		hurtTime = maxHurtTime = 10;
		attackedAtYaw = 0.0f;
		worldObj->playSoundAtEntity(this, getHurtSound(), getSoundVolume(), (rand.nextFloat() - rand.nextFloat()) * 0.2f + 1.0f);
		attackEntityFrom(nullptr, 0);
	}
	else if (byte0 == 3)
	{
		worldObj->playSoundAtEntity(this, getDeathSound(), getSoundVolume(), (rand.nextFloat() - rand.nextFloat()) * 0.2f + 1.0f);
		health = 0;
		onDeath(nullptr);
	}
	else
	{
		Entity::handleHealthUpdate(byte0);
	}
}

bool EntityLiving::isPlayerSleeping()
{
	return false;
}

int_t EntityLiving::getItemIcon(ItemStack *itemstack)
{
	return itemstack->getIconIndex();
}
