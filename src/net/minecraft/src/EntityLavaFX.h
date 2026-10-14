#pragma once

#include "EntityFX.h"

class World;
class Tessellator;

// net.minecraft.src.EntityLavaFX
class EntityLavaFX : public EntityFX
{
public:
	EntityLavaFX(World *world, double d, double d1, double d2);

	float getEntityBrightness(float f) override;
	void renderParticle(Tessellator *tessellator, float f, float f1, float f2, float f3, float f4, float f5) override;
	void onUpdate() override;

private:
	float field_674_a;
};
