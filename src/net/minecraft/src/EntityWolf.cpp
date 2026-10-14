#include "EntityWolf.h"

#include <cmath>

#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemFood.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "World.h"
#include "AxisAlignedBB.h"
#include "DataWatcher.h"

EntityWolf::EntityWolf(World *world)
	: EntityAnimal(world)
{
	ensureEntityInit();
	looksWithInterest = false;
	field_25048_b = 0.0f;
	field_25054_c = 0.0f;
	isWolfShaking = false;
	field_25052_g = false;
	timeWolfIsShaking = 0.0f;
	prevTimeWolfIsShaking = 0.0f;
	texture = String::fromUTF8("/mob/wolf.png");
	setSize(0.8f, 0.8f);
	moveSpeed = 1.1f;
	health = 8;
}

void EntityWolf::entityInit()
{
	EntityAnimal::entityInit();
	dataWatcher->addObject(16, (byte_t)0);
	dataWatcher->addObject(17, std::string()); // DataWatcher stores String as UTF-8 std::string, not jstring
	dataWatcher->addObject(18, (int_t)health);
}

bool EntityWolf::canTriggerWalking()
{
	return false;
}

jstring EntityWolf::getEntityTexture()
{
	if (isWolfTamed())
	{
		return String::fromUTF8("/mob/wolf_tame.png");
	}
	if (isWolfAngry())
	{
		return String::fromUTF8("/mob/wolf_angry.png");
	}
	return String::fromUTF8("/mob/wolf.png");
}

void EntityWolf::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	EntityAnimal::writeEntityToNBT(nbttagcompound);
	nbttagcompound->setBoolean(u"Angry", isWolfAngry());
	nbttagcompound->setBoolean(u"Sitting", isWolfSitting());
	if (getWolfOwner().length() > 0)
	{
		nbttagcompound->setString(u"Owner", getWolfOwner());
	}
}

void EntityWolf::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	EntityAnimal::readEntityFromNBT(nbttagcompound);
	setWolfAngry(nbttagcompound->getBoolean(u"Angry"));
	setWolfSitting(nbttagcompound->getBoolean(u"Sitting"));
	jstring s = nbttagcompound->getString(u"Owner");
	if (s.length() > 0)
	{
		setWolfOwner(s);
		setWolfTamed(true);
	}
}

bool EntityWolf::canDespawn()
{
	return !isWolfTamed();
}

jstring EntityWolf::getLivingSound() { return u"mob.wolf.bark"; }
jstring EntityWolf::getHurtSound()   { return u"mob.wolf.hurt"; }
jstring EntityWolf::getDeathSound()  { return u"mob.wolf.death"; }

float EntityWolf::getSoundVolume()
{
	return 0.4f;
}

int_t EntityWolf::getDropItemId()
{
	return -1;
}

void EntityWolf::updatePlayerActionState()
{
	EntityAnimal::updatePlayerActionState();
	if (!hasAttacked && !hasPath() && isWolfTamed() && riddenByEntity == nullptr)
	{
		Entity *entity = worldObj->getClosestPlayerToEntity(this, 10.0);
		if (entity != nullptr)
		{
			setPathToEntity(worldObj->getPathToEntity(this, entity, 16.0f));
		}
		else if (rand.nextInt(100) == 0)
		{
			looksWithInterest = true;
			field_25054_c = 0.0f;
			field_25048_b = 0.0f;
		}
	}
}

void EntityWolf::onLivingUpdate()
{
	EntityAnimal::onLivingUpdate();
	if (!worldObj->multiplayerWorld && isWolfShaking && !field_25052_g && !hasPath() && onGround)
	{
		field_25052_g = true;
		timeWolfIsShaking = 0.0f;
		prevTimeWolfIsShaking = 0.0f;
		worldObj->setEntityState(this, (byte_t)8);
	}
}

void EntityWolf::onUpdate()
{
	EntityAnimal::onUpdate();
	if (!worldObj->multiplayerWorld)
	{
		dataWatcher->updateObject(18, (int_t)health);
	}
	else
	{
		health = dataWatcher->getWatchableObjectInt(18);
	}
	if (isWolfSitting())
	{
		// Sitting state
	}
}

bool EntityWolf::getWolfShaking()
{
	return isWolfShaking;
}

float EntityWolf::getShadingWhileShaking(float f)
{
	return 0.75f + (prevTimeWolfIsShaking + (timeWolfIsShaking - prevTimeWolfIsShaking) * f) / 2.0f * 0.25f;
}

float EntityWolf::getShakeAngle(float f, float f1)
{
	float f2 = (prevTimeWolfIsShaking + (timeWolfIsShaking - prevTimeWolfIsShaking) * f + f1) / 1.8f;
	if (f2 < 0.0f)
	{
		f2 = 0.0f;
	}
	else if (f2 > 1.0f)
	{
		f2 = 1.0f;
	}
	return MathHelper::sin(f2 * 3.141593f) * MathHelper::sin(f2 * 3.141593f * 11.0f) * 0.15f * 3.141593f;
}

float EntityWolf::getInterestedAngle(float f)
{
	return (field_25048_b + (field_25054_c - field_25048_b) * f) * 0.15f * 3.141593f;
}

float EntityWolf::getEyeHeight()
{
	return height * 0.8f;
}

int_t EntityWolf::func_25026_x()
{
	if (isWolfSitting())
	{
		return 20;
	}
	return EntityAnimal::func_25026_x();
}

void EntityWolf::getPathOrWalkableBlock(Entity *entity, float f)
{
	PathEntity *pathentity = worldObj->getPathToEntity(this, entity, 16.0f);
	if (pathentity == nullptr && f > 12.0f)
	{
		int i = MathHelper::floor_double(entity->posX) - 2;
		int j = MathHelper::floor_double(entity->posZ) - 2;
		int k = MathHelper::floor_double(entity->boundingBox->minY);
		for (int l = 0; l <= 4; l++)
		{
			for (int i1 = 0; i1 <= 4; i1++)
			{
				if ((l < 1 || i1 < 1 || l > 3 || i1 > 3) && worldObj->isBlockNormalCube(i + l, k - 1, j + i1) && !worldObj->isBlockNormalCube(i + l, k, j + i1) && !worldObj->isBlockNormalCube(i + l, k + 1, j + i1))
				{
					setLocationAndAngles((float)(i + l) + 0.5f, (double)k, (float)(j + i1) + 0.5f, rotationYaw, rotationPitch);
					return;
				}
			}
		}
	}
	else
	{
		setPathToEntity(pathentity);
	}
}

bool EntityWolf::isMovementCeased()
{
	return isWolfSitting() || field_25052_g;
}

bool EntityWolf::attackEntityFrom(Entity *entity, int_t i)
{
	EntityAnimal::attackEntityFrom(entity, i);
	setWolfSitting(false);
	if (entity != nullptr && !(dynamic_cast<EntityPlayer *>(entity) != nullptr && isWolfTamed()) && dynamic_cast<EntityWolf *>(entity) == nullptr)
	{
		setWolfAngry(true);
	}
	return true;
}

Entity *EntityWolf::findPlayerToAttack()
{
	if (isWolfAngry())
	{
		return worldObj->getClosestPlayerToEntity(this, 16.0);
	}
	return nullptr;
}

void EntityWolf::attackEntity(Entity *entity, float f)
{
	if (f < 2.0f && entity->boundingBox->maxY > boundingBox->minY && entity->boundingBox->minY < boundingBox->maxY)
	{
		attackTime = 20;
		entity->attackEntityFrom(this, 2);
	}
}

bool EntityWolf::interact(EntityPlayer *entityplayer)
{
	ItemStack *itemstack = entityplayer->inventory->getCurrentItem();
	if (!isWolfTamed())
	{
		if (itemstack != nullptr && itemstack->itemID == Item::bone->shiftedIndex && !isWolfAngry())
		{
			itemstack->stackSize--;
			if (itemstack->stackSize <= 0)
			{
				entityplayer->inventory->setInventorySlotContents(entityplayer->inventory->currentItem, nullptr);
			}
			if (!worldObj->multiplayerWorld)
			{
				if (rand.nextInt(3) == 0)
				{
					setWolfTamed(true);
					setPathToEntity(nullptr);
					setWolfSitting(true);
					health = 20;
					setWolfOwner(entityplayer->username);
					showHeartsOrSmokeFX(true);
					worldObj->setEntityState(this, (byte_t)7);
				}
				else
				{
					showHeartsOrSmokeFX(false);
					worldObj->setEntityState(this, (byte_t)6);
				}
			}
			return true;
		}
	}
	else
	{
		ItemFood *itemfood = dynamic_cast<ItemFood *>(Item::itemsList[itemstack->itemID]);
		if (itemstack != nullptr && itemfood != nullptr)
		{
			if (itemfood->getIsWolfsFavoriteMeat() && dataWatcher->getWatchableObjectInt(18) < 20)
			{
				itemstack->stackSize--;
				if (itemstack->stackSize <= 0)
				{
					entityplayer->inventory->setInventorySlotContents(entityplayer->inventory->currentItem, nullptr);
				}
				heal(itemfood->getHealAmount());
				return true;
			}
		}
		if (entityplayer->username == getWolfOwner())
		{
			if (!worldObj->multiplayerWorld)
			{
				setWolfSitting(!isWolfSitting());
				isJumping = false;
				setPathToEntity(nullptr);
			}
			return true;
		}
	}
	return EntityAnimal::interact(entityplayer);
}

void EntityWolf::showHeartsOrSmokeFX(bool flag)
{
	jstring s = flag ? u"heart" : u"smoke";
	for (int i = 0; i < 7; i++)
	{
		double d = rand.nextGaussian() * 0.02;
		double d1 = rand.nextGaussian() * 0.02;
		double d2 = rand.nextGaussian() * 0.02;
		worldObj->spawnParticle(s, posX + (double)(rand.nextFloat() * width * 2.0f) - (double)width, posY + 0.5 + (double)(rand.nextFloat() * height), posZ + (double)(rand.nextFloat() * width * 2.0f) - (double)width, d, d1, d2);
	}
}

void EntityWolf::handleHealthUpdate(byte_t byte0)
{
	if (byte0 == 8)
	{
		field_25052_g = true;
		timeWolfIsShaking = 0.0f;
		prevTimeWolfIsShaking = 0.0f;
	}
	else
	{
		EntityAnimal::handleHealthUpdate(byte0);
	}
}

float EntityWolf::setTailRotation()
{
	if (isWolfAngry())
	{
		return 1.53938f;
	}
	if (isWolfTamed())
	{
		return (0.55f - (float)(20 - dataWatcher->getWatchableObjectInt(18)) * 0.02f) * 3.141593f;
	}
	return 0.6283185f;
}

int_t EntityWolf::getMaxSpawnedInChunk()
{
	return 8;
}

jstring EntityWolf::getWolfOwner()
{
	return String::fromUTF8(dataWatcher->getWatchableObjectString(17));
}

void EntityWolf::setWolfOwner(const jstring &s)
{
	dataWatcher->updateObject(17, String::toUTF8(s)); // convert jstring -> UTF-8 to match DataWatcher's String type
}

bool EntityWolf::isWolfSitting()
{
	return (dataWatcher->getWatchableObjectByte(16) & 1) != 0;
}

void EntityWolf::setWolfSitting(bool flag)
{
	byte_t byte0 = dataWatcher->getWatchableObjectByte(16);
	if (flag)
	{
		dataWatcher->updateObject(16, (byte_t)(byte0 | 1));
	}
	else
	{
		dataWatcher->updateObject(16, (byte_t)(byte0 & ~1));
	}
}

bool EntityWolf::isWolfAngry()
{
	return (dataWatcher->getWatchableObjectByte(16) & 2) != 0;
}

void EntityWolf::setWolfAngry(bool flag)
{
	byte_t byte0 = dataWatcher->getWatchableObjectByte(16);
	if (flag)
	{
		dataWatcher->updateObject(16, (byte_t)(byte0 | 2));
	}
	else
	{
		dataWatcher->updateObject(16, (byte_t)(byte0 & ~2));
	}
}

bool EntityWolf::isWolfTamed()
{
	return (dataWatcher->getWatchableObjectByte(16) & 4) != 0;
}

void EntityWolf::setWolfTamed(bool flag)
{
	byte_t byte0 = dataWatcher->getWatchableObjectByte(16);
	if (flag)
	{
		dataWatcher->updateObject(16, (byte_t)(byte0 | 4));
	}
	else
	{
		dataWatcher->updateObject(16, (byte_t)(byte0 & ~4));
	}
}