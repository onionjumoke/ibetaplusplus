#pragma once

#include "Entity.h"
#include "java/Type.h"

class World;
class EntityPlayer;
class ItemStack;
class NBTTagCompound;
class Material;

// net.minecraft.src.EntityItem
class EntityItem : public Entity
{
public:
	EntityItem(World *world, double d, double d1, double d2, ItemStack *itemstack);
	EntityItem(World *world);

protected:
	bool canTriggerWalking() override;
	void entityInit() override;

public:
	void onUpdate() override;
	bool handleWaterMovement() override;

protected:
	void dealFireDamage(int_t i) override;

public:
	bool attackEntityFrom(Entity *entity, int_t i) override;
	void writeEntityToNBT(NBTTagCompound *nbttagcompound) override;
	void readEntityFromNBT(NBTTagCompound *nbttagcompound) override;
	void onCollideWithPlayer(EntityPlayer *entityplayer) override;

public:
	ItemStack *item;
	int age;
	int delayBeforeCanPickup;
	float hoverStart;

private:
	int health;
};
