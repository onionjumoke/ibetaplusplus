#include "EntityFlameFX.h"

#include <cstdlib>

#include "Tessellator.h"
#include "World.h"

static inline double mathRandom_flame()
{
	return (double)std::rand() / (double)RAND_MAX;
}

EntityFlameFX::EntityFlameFX(World *world, double d, double d1, double d2,
                             double d3, double d4, double d5)
	: EntityFX(world, d, d1, d2, d3, d4, d5)
{
	motionX = motionX * 0.0099999997764825821 + d3;
	motionY = motionY * 0.0099999997764825821 + d4;
	motionZ = motionZ * 0.0099999997764825821 + d5;
	d  += (rand.nextFloat() - rand.nextFloat()) * 0.05f;
	d1 += (rand.nextFloat() - rand.nextFloat()) * 0.05f;
	d2 += (rand.nextFloat() - rand.nextFloat()) * 0.05f;
	field_672_a = particleScale;
	particleRed = particleGreen = particleBlue = 1.0f;
	particleMaxAge = (int_t)(8.0 / (mathRandom_flame() * 0.8 + 0.2)) + 4;
	noClip = true;
	particleTextureIndex = 48;
}

void EntityFlameFX::renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5)
{
	float f6 = ((float)particleAge + f) / (float)particleMaxAge;
	particleScale = field_672_a * (1.0f - f6 * f6 * 0.5f);
	EntityFX::renderParticle(tessellator, f, f1, f2, f3, f4, f5);
}

float EntityFlameFX::getEntityBrightness(float f)
{
	float f1 = ((float)particleAge + f) / (float)particleMaxAge;
	if (f1 < 0.0f) { f1 = 0.0f; }
	if (f1 > 1.0f) { f1 = 1.0f; }
	float f2 = Entity::getEntityBrightness(f);
	return f2 * f1 + (1.0f - f1);
}

void EntityFlameFX::onUpdate()
{
	prevPosX = posX;
	prevPosY = posY;
	prevPosZ = posZ;
	if (particleAge++ >= particleMaxAge)
	{
		setEntityDead();
	}
	moveEntity(motionX, motionY, motionZ);
	motionX *= 0.95999997854232788;
	motionY *= 0.95999997854232788;
	motionZ *= 0.95999997854232788;
	if (onGround)
	{
		motionX *= 0.69999998807907104;
		motionZ *= 0.69999998807907104;
	}
}
