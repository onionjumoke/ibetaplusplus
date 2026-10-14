#include "EntityLavaFX.h"

#include <cstdlib>

#include "Tessellator.h"
#include "World.h"

static inline double mathRandom_lava()
{
	return (double)std::rand() / (double)RAND_MAX;
}

EntityLavaFX::EntityLavaFX(World *world, double d, double d1, double d2)
	: EntityFX(world, d, d1, d2, 0.0, 0.0, 0.0)
{
	motionX *= 0.80000001192092896;
	motionY *= 0.80000001192092896;
	motionZ *= 0.80000001192092896;
	motionY = rand.nextFloat() * 0.4f + 0.05f;
	particleRed = particleGreen = particleBlue = 1.0f;
	particleScale *= rand.nextFloat() * 2.0f + 0.2f;
	field_674_a = particleScale;
	particleMaxAge = (int_t)(16.0 / (mathRandom_lava() * 0.8 + 0.2));
	noClip = false;
	particleTextureIndex = 49;
}

float EntityLavaFX::getEntityBrightness(float f)
{
	return 1.0f;
}

void EntityLavaFX::renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5)
{
	float f6 = ((float)particleAge + f) / (float)particleMaxAge;
	particleScale = field_674_a * (1.0f - f6 * f6);
	EntityFX::renderParticle(tessellator, f, f1, f2, f3, f4, f5);
}

void EntityLavaFX::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	if (particleAge++ >= particleMaxAge)
	{
		setEntityDead();
	}
	float f = (float)particleAge / (float)particleMaxAge;
	if (rand.nextFloat() > f)
	{
		worldObj->spawnParticle(u"smoke", posX, posY, posZ, motionX, motionY, motionZ);
	}
	motionY -= 0.03;
	moveEntity(motionX, motionY, motionZ);
	motionX *= 0.99900001287460327;
	motionY *= 0.99900001287460327;
	motionZ *= 0.99900001287460327;
	if (onGround)
	{
		motionX *= 0.69999998807907104;
		motionZ *= 0.69999998807907104;
	}
}
