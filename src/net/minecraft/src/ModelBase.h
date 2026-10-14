#pragma once

class EntityLiving;

// net.minecraft.src.ModelBase
class ModelBase
{
public:
	ModelBase();
	virtual ~ModelBase() = default;

	virtual void render(float f, float f1, float f2, float f3, float f4, float f5);
	virtual void setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5);
	virtual void setLivingAnimations(EntityLiving *entityliving, float f, float f1, float f2);

	float onGround;
	bool isRiding;
};
