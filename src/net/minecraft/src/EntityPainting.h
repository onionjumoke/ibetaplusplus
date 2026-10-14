#pragma once

#include "Entity.h"
#include "java/Type.h"

class World;
class EnumArt;
class EntityPlayer;
class NBTTagCompound;
class ItemStack;

// net.minecraft.src.EntityPainting
class EntityPainting : public Entity
{
public:
	EntityPainting(World *world);
	EntityPainting(World *world, int_t i, int_t j, int_t k, int_t l);
	EntityPainting(World *world, int_t i, int_t j, int_t k, int_t l, jstring s);

protected:
	void entityInit() override;

public:
	void func_412_b(int_t i);
	void onUpdate() override;
	bool onValidSurface();
	bool canBeCollidedWith() override;
	bool attackEntityFrom(Entity *entity, int_t i) override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;

private:
	float func_411_c(int_t i);

public:
	int_t xPosition;
	int_t yPosition;
	int_t zPosition;
	int_t direction;
	EnumArt *art;

private:
	int_t field_695_c;
};
