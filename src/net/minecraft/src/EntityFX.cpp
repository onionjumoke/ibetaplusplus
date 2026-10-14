#include "EntityFX.h"

#include <cmath>
#include <cstdlib>

#include "MathHelper.h"
#include "NBTTagCompound.h"
#include "Tessellator.h"
#include "World.h"

double EntityFX::interpPosX = 0.0;
double EntityFX::interpPosY = 0.0;
double EntityFX::interpPosZ = 0.0;

static inline double mathRandom()
{
	return (double)std::rand() / (double)RAND_MAX;
}

EntityFX::EntityFX(World *world, double d, double d1, double d2,
                   double d3, double d4, double d5)
	: Entity(world)
{
	entityInit();
	particleAge = 0;
	particleMaxAge = 0;
	setSize(0.2f, 0.2f);
	yOffset = height / 2.0f;
	setPosition(d, d1, d2);
	particleRed = particleGreen = particleBlue = 1.0f;
	motionX = d3 + (double)((float)(mathRandom() * 2.0 - 1.0) * 0.4f);
	motionY = d4 + (double)((float)(mathRandom() * 2.0 - 1.0) * 0.4f);
	motionZ = d5 + (double)((float)(mathRandom() * 2.0 - 1.0) * 0.4f);
	float f = (float)(mathRandom() + mathRandom() + 1.0) * 0.15f;
	float f1 = MathHelper::sqrt_double(motionX * motionX + motionY * motionY + motionZ * motionZ);
	motionX = (motionX / (double)f1) * (double)f * 0.40000000596046448;
	motionY = (motionY / (double)f1) * (double)f * 0.40000000596046448 + 0.10000000149011612;
	motionZ = (motionZ / (double)f1) * (double)f * 0.40000000596046448;
	particleTextureJitterX = rand.nextFloat() * 3.0f;
	particleTextureJitterY = rand.nextFloat() * 3.0f;
	particleScale = (rand.nextFloat() * 0.5f + 0.5f) * 2.0f;
	particleMaxAge = (int_t)(4.0f / (rand.nextFloat() * 0.9f + 0.1f));
	particleAge = 0;
	particleTextureIndex = 0;
	particleGravity = 0.0f;
}

EntityFX *EntityFX::func_407_b(float f)
{
	motionX *= f;
	motionY = (motionY - 0.10000000149011612) * (double)f + 0.10000000149011612;
	motionZ *= f;
	return this;
}

EntityFX *EntityFX::func_405_d(float f)
{
	setSize(0.2f * f, 0.2f * f);
	particleScale *= f;
	return this;
}

bool EntityFX::canTriggerWalking()
{
	return false;
}

void EntityFX::entityInit()
{
}

void EntityFX::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	if (particleAge++ >= particleMaxAge)
	{
		setEntityDead();
	}
	motionY -= 0.04 * (double)particleGravity;
	moveEntity(motionX, motionY, motionZ);
	motionX *= 0.98000001907348633;
	motionY *= 0.98000001907348633;
	motionZ *= 0.98000001907348633;
	if (onGround)
	{
		motionX *= 0.69999998807907104;
		motionZ *= 0.69999998807907104;
	}
}

void EntityFX::renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5)
{
	float f6  = (float)(particleTextureIndex % 16) / 16.0f;
	float f7  = f6 + 0.0624375f;
	float f8  = (float)(particleTextureIndex / 16) / 16.0f;
	float f9  = f8 + 0.0624375f;
	float f10 = 0.1f * particleScale;
	float f11 = (float)((prevPosX + (posX - prevPosX) * (double)f) - interpPosX);
	float f12 = (float)((prevPosY + (posY - prevPosY) * (double)f) - interpPosY);
	float f13 = (float)((prevPosZ + (posZ - prevPosZ) * (double)f) - interpPosZ);
	float f14 = getEntityBrightness(f);
	tessellator->setColorOpaque_F(particleRed * f14, particleGreen * f14, particleBlue * f14);
	tessellator->addVertexWithUV(f11 - f1 * f10 - f4 * f10, f12 - f2 * f10, f13 - f3 * f10 - f5 * f10, f7, f9);
	tessellator->addVertexWithUV((f11 - f1 * f10) + f4 * f10, f12 + f2 * f10, (f13 - f3 * f10) + f5 * f10, f7, f8);
	tessellator->addVertexWithUV(f11 + f1 * f10 + f4 * f10, f12 + f2 * f10, f13 + f3 * f10 + f5 * f10, f6, f8);
	tessellator->addVertexWithUV((f11 + f1 * f10) - f4 * f10, f12 - f2 * f10, (f13 + f3 * f10) - f5 * f10, f6, f9);
}

int_t EntityFX::getFXLayer()
{
	return 0;
}

void EntityFX::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
}

void EntityFX::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
}
