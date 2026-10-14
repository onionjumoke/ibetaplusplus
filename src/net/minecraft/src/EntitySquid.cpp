#include "EntitySquid.h"

#include <cmath>

#include "EntityItem.h"
#include "Item.h"
#include "ItemStack.h"
#include "Material.h"
#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "World.h"
#include "AxisAlignedBB.h"

EntitySquid::EntitySquid(World *world)
	: EntityWaterMob(world)
{
	field_21089_a = 0.0f;
	field_21088_b = 0.0f;
	field_21087_c = 0.0f;
	field_21086_f = 0.0f;
	field_21085_g = 0.0f;
	field_21084_h = 0.0f;
	field_21083_i = 0.0f;
	field_21082_j = 0.0f;
	randomMotionSpeed = 0.0f;
	field_21080_l = 0.0f;
	field_21079_m = 0.0f;
	randomMotionVecX = 0.0f;
	randomMotionVecY = 0.0f;
	randomMotionVecZ = 0.0f;
	texture = String::fromUTF8("/mob/squid.png");
	setSize(0.95f, 0.95f);
	field_21080_l = (1.0f / (rand.nextFloat() + 1.0f)) * 0.2f;
}

void EntitySquid::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	EntityWaterMob::writeEntityToNBT(nbttagcompound);
}

void EntitySquid::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	EntityWaterMob::readEntityFromNBT(nbttagcompound);
}

jstring EntitySquid::getLivingSound() { return jstring(); }
jstring EntitySquid::getHurtSound()   { return jstring(); }
jstring EntitySquid::getDeathSound()  { return jstring(); }

float EntitySquid::getSoundVolume()
{
	return 0.4f;
}

int_t EntitySquid::getDropItemId()
{
	return 0;
}

void EntitySquid::dropFewItems()
{
	int i = rand.nextInt(3) + 1;
	for (int j = 0; j < i; j++)
	{
		entityDropItem(new ItemStack(Item::dyePowder, 1, 0), 0.0f);
	}
}

bool EntitySquid::interact(EntityPlayer *entityplayer)
{
	return false;
}

bool EntitySquid::isInWater()
{
	return worldObj->handleMaterialAcceleration(boundingBox->expand(0.0, -0.60000002384185791, 0.0), Material::water, this);
}

void EntitySquid::onLivingUpdate()
{
	EntityWaterMob::onLivingUpdate();
	field_21088_b = field_21089_a;
	field_21086_f = field_21087_c;
	field_21084_h = field_21085_g;
	field_21082_j = field_21083_i;
	field_21085_g += field_21080_l;
	if (field_21085_g > 6.283185f)
	{
		field_21085_g -= 6.283185f;
		if (rand.nextInt(10) == 0)
		{
			field_21080_l = (1.0f / (rand.nextFloat() + 1.0f)) * 0.2f;
		}
	}
	if (isInWater())
	{
		if (field_21085_g < 3.141593f)
		{
			float f = field_21085_g / 3.141593f;
			field_21083_i = MathHelper::sin(f * f * 3.141593f) * 3.141593f * 0.25f;
			if ((double)f > 0.75)
			{
				randomMotionSpeed = 1.0f;
				field_21079_m = 1.0f;
			}
			else
			{
				field_21079_m = field_21079_m * 0.8f;
			}
		}
		else
		{
			field_21083_i = 0.0f;
			randomMotionSpeed = randomMotionSpeed * 0.9f;
			field_21079_m = field_21079_m * 0.99f;
		}
		if (!isMultiplayerEntity)
		{
			motionX = (double)(randomMotionVecX * randomMotionSpeed);
			motionY = (double)(randomMotionVecY * randomMotionSpeed);
			motionZ = (double)(randomMotionVecZ * randomMotionSpeed);
		}
		float f1 = MathHelper::sqrt_double(motionX * motionX + motionZ * motionZ);
		renderYawOffset += ((-(float)std::atan2(motionX, motionZ) * 180.0f) / 3.141593f - renderYawOffset) * 0.1f;
		rotationYaw = renderYawOffset;
		field_21087_c = field_21087_c + 3.141593f * field_21079_m * 1.5f;
		field_21089_a += ((-(float)std::atan2(f1, motionY) * 180.0f) / 3.141593f - field_21089_a) * 0.1f;
	}
	else
	{
		field_21083_i = MathHelper::abs(MathHelper::sin(field_21085_g)) * 3.141593f * 0.25f;
		if (!isMultiplayerEntity)
		{
			motionX = 0.0;
			motionY -= 0.080000000000000002;
			motionY *= 0.98000001907348633;
			motionZ = 0.0;
		}
		field_21089_a += (double)(-90.0f - field_21089_a) * 0.02;
	}
}

void EntitySquid::moveEntityWithHeading(float f, float f1)
{
	moveEntity(motionX, motionY, motionZ);
}

void EntitySquid::updatePlayerActionState()
{
	if (rand.nextInt(50) == 0 || !inWater || randomMotionVecX == 0.0f && randomMotionVecY == 0.0f && randomMotionVecZ == 0.0f)
	{
		float f = rand.nextFloat() * 3.141593f * 2.0f;
		randomMotionVecX = MathHelper::cos(f) * 0.2f;
		randomMotionVecY = -0.1f + rand.nextFloat() * 0.2f;
		randomMotionVecZ = MathHelper::sin(f) * 0.2f;
	}
	func_27021_X();
}
