#include "EntityFallingSand.h"

#include "Block.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "World.h"
#include "BlockPistonMoving.h"

EntityFallingSand::EntityFallingSand(World *world)
	: Entity(world)
{
	fallTime = 0;
}

EntityFallingSand::EntityFallingSand(World *world, double d, double d1, double d2, int_t i)
	: Entity(world)
{
	fallTime = 0;
	blockID = i;
	setSize(0.98f, 0.98f);
	yOffset = height / 2.0f;
	setPosition(d, d1, d2);
	motionX = 0.0;
	motionY = 0.0;
	motionZ = 0.0;
	prevPosX = d;
	prevPosY = d1;
	prevPosZ = d2;
}

void EntityFallingSand::entityInit()
{
}

void EntityFallingSand::onUpdate()
{
	if (blockID == 0)
	{
		setEntityDead();
		return;
	}
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	fallTime++;
	motionY -= 0.039999999105930328;
	moveEntity(motionX, motionY, motionZ);
	motionX *= 0.98000001907348633;
	motionY *= 0.98000001907348633;
	motionZ *= 0.98000001907348633;
	int i = MathHelper::floor_double(posX);
	int j = MathHelper::floor_double(posY);
	int k = MathHelper::floor_double(posZ);
	if (worldObj->getBlockId(i, j, k) == Block::pistonMoving->blockID)
	{
		;
	}
	else if (worldObj->getBlockId(i, j - 1, k) == Block::pistonMoving->blockID)
	{
		j--;
	}
	if (fallTime == 1 && worldObj->getBlockId(i, j, k) == blockID)
	{
		worldObj->setBlockWithNotify(i, j, k, 0);
	}
	else if (!worldObj->multiplayerWorld && fallTime > 600)
	{
		setEntityDead();
		dropItem(blockID, 1);
	}
	else if (onGround)
	{
		motionX *= 0.69999998807907104;
		motionZ *= 0.69999998807907104;
		motionY *= -0.5;
		if (worldObj->getBlockId(i, j, k) != Block::pistonMoving->blockID)
		{
			setEntityDead();
			if ((!worldObj->canBlockBePlacedAt(blockID, i, j, k, true, 1) || Block::sand->canPlaceBlockAt(worldObj, i, j, k) || !worldObj->setBlockWithNotify(i, j, k, blockID)) && !worldObj->multiplayerWorld)
			{
				dropItem(blockID, 1);
			}
		}
	}
	else if (!worldObj->multiplayerWorld && fallTime > 600)
	{
		setEntityDead();
		dropItem(blockID, 1);
	}
}

void EntityFallingSand::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	nbttagcompound->setByte(u"Tile", (byte_t)blockID);
}

void EntityFallingSand::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	blockID = nbttagcompound->getByte(u"Tile") & 0xFF;
}

float EntityFallingSand::getShadowSize()
{
	return 0.0f;
}
