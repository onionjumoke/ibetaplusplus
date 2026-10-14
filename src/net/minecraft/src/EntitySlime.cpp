#include "EntitySlime.h"

#include <cmath>

#include "EntityItem.h"
#include "EntityPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "World.h"
#include "AxisAlignedBB.h"
#include "DataWatcher.h"

EntitySlime::EntitySlime(World *world)
	: EntityLiving(world)
{
	ensureEntityInit(); // must run before setSlimeSize() touches DataWatcher id 16
	slimeJumpDelay = 0;
	texture = String::fromUTF8("/mob/slime.png");
	int i = 1 << rand.nextInt(3);
	yOffset = 0.0f;
	slimeJumpDelay = rand.nextInt(20) + 10;
	setSlimeSize(i);
}

void EntitySlime::entityInit()
{
	EntityLiving::entityInit();
	dataWatcher->addObject(16, (byte_t)1);
}

void EntitySlime::setSlimeSize(int_t i)
{
	dataWatcher->updateObject(16, (byte_t)i);
	setSize(0.6f * (float)i, 0.6f * (float)i);
	health = i * i;
	setPosition(posX, posY, posZ);
}

int_t EntitySlime::getSlimeSize()
{
	return dataWatcher->getWatchableObjectByte(16);
}

void EntitySlime::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	EntityLiving::writeEntityToNBT(nbttagcompound);
	nbttagcompound->setInteger(u"Size", getSlimeSize() - 1);
}

void EntitySlime::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	EntityLiving::readEntityFromNBT(nbttagcompound);
	setSlimeSize(nbttagcompound->getInteger(u"Size") + 1);
}

void EntitySlime::onUpdate()
{
	field_767_b = field_768_a;
	bool flag = onGround;
	EntityLiving::onUpdate();
	if (onGround && !flag)
	{
		int i = getSlimeSize();
		for (int j = 0; j < i * 8; j++)
		{
			float f = rand.nextFloat() * 3.141593f * 2.0f;
			float f1 = rand.nextFloat() * 0.5f + 0.5f;
			float f2 = MathHelper::sin(f) * (float)i * 0.5f * f1;
			float f3 = MathHelper::cos(f) * (float)i * 0.5f * f1;
			worldObj->spawnParticle(u"slime", posX + (double)f2, boundingBox->minY, posZ + (double)f3, 0.0, 0.0, 0.0);
		}
		if (i > 2)
		{
			worldObj->playSoundAtEntity(this, u"mob.slime", getSoundVolume(), ((rand.nextFloat() - rand.nextFloat()) * 0.2f + 1.0f) / 0.8f);
		}
		field_768_a = -0.5f;
	}
	field_768_a = field_768_a * 0.6f;
}

void EntitySlime::updatePlayerActionState()
{
	func_27021_X();
	EntityPlayer *entityplayer = worldObj->getClosestPlayerToEntity(this, 16.0);
	if (entityplayer != nullptr)
	{
		faceEntity(entityplayer, 10.0f, 20.0f);
	}
	if (onGround && slimeJumpDelay-- <= 0)
	{
		slimeJumpDelay = rand.nextInt(20) + 10;
		if (entityplayer != nullptr)
		{
			slimeJumpDelay /= 3;
		}
		isJumping = true;
		if (getSlimeSize() > 1)
		{
			worldObj->playSoundAtEntity(this, u"mob.slime", getSoundVolume(), ((rand.nextFloat() - rand.nextFloat()) * 0.2f + 1.0f) * 0.8f);
		}
		moveForward = 1.0f;
		rotationYaw = (float)(rand.nextFloat() * 3.141593f * 2.0f);
	}
	else
	{
		isJumping = false;
		if (onGround)
		{
			moveForward = 0.0f;
		}
	}
}

void EntitySlime::onEntityDeath()
{
	int i = getSlimeSize();
	if (!worldObj->multiplayerWorld && i > 1)
	{
		for (int j = 0; j < 4; j++)
		{
			float f = ((float)(j % 2) - 0.5f) * (float)i / 4.0f;
			float f1 = ((float)(j / 2) - 0.5f) * (float)i / 4.0f;
			EntitySlime *entityslime = new EntitySlime(worldObj);
			entityslime->setSlimeSize(i / 2);
			entityslime->setLocationAndAngles(posX + (double)f, posY + 0.5, posZ + (double)f1, rand.nextFloat() * 360.0f, 0.0f);
			worldObj->entityJoinedWorld(entityslime);
		}
	}
}

bool EntitySlime::getCanSpawnHere()
{
	int i = MathHelper::floor_double(posX);
	int j = MathHelper::floor_double(boundingBox->minY);
	int k = MathHelper::floor_double(posZ);
	if (worldObj->getSavedLightValue(EnumSkyBlock::Sky, i, j, k) > rand.nextInt(32))
	{
		return false;
	}
	if (worldObj->getBlockLightValue(i, j, k) > 0)
	{
		return false;
	}
	return EntityLiving::getCanSpawnHere();
}

jstring EntitySlime::getLivingSound() { return u"mob.slime"; }
jstring EntitySlime::getHurtSound()   { return u"mob.slime"; }
jstring EntitySlime::getDeathSound()  { return u"mob.slime"; }

float EntitySlime::getSoundVolume()
{
	return 0.4f * (float)getSlimeSize();
}

int_t EntitySlime::getDropItemId()
{
	if (getSlimeSize() == 1)
	{
		return Item::slimeBall->shiftedIndex;
	}
	return 0;
}
