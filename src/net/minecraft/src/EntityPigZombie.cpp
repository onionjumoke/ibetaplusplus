#include "EntityPigZombie.h"

#include "EntityPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "NBTTagCompound.h"
#include "World.h"
#include "AxisAlignedBB.h"

ItemStack *EntityPigZombie::defaultHeldItem = nullptr;

EntityPigZombie::EntityPigZombie(World *world)
	: EntityZombie(world)
{
	angerLevel = 0;
	randomSoundDelay = 0;
	texture = String::fromUTF8("/mob/pigzombie.png");
	moveSpeed = 0.5f;
	attackStrength = 5;
	isImmuneToFire = true;
	if (defaultHeldItem == nullptr)
	{
		defaultHeldItem = new ItemStack(Item::swordGold, 1);
	}
}

void EntityPigZombie::onUpdate()
{
	moveSpeed = playerToAttack == nullptr ? 0.5f : 0.95f;
	if (randomSoundDelay > 0 && --randomSoundDelay == 0)
	{
		worldObj->playSoundAtEntity(this, u"mob.zombiepig.zpigangry", getSoundVolume() * 2.0f, ((rand.nextFloat() - rand.nextFloat()) * 0.2f + 1.0f) * 1.8f);
	}
	EntityZombie::onUpdate();
}

bool EntityPigZombie::getCanSpawnHere()
{
	return worldObj->difficultySetting > 0 && worldObj->checkIfAABBIsClear(boundingBox) && worldObj->getCollidingBoundingBoxes(this, boundingBox).size() == 0 && !worldObj->getIsAnyLiquid(boundingBox);
}

void EntityPigZombie::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	EntityZombie::writeEntityToNBT(nbttagcompound);
	nbttagcompound->setShort(u"Anger", (short)angerLevel);
}

void EntityPigZombie::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	EntityZombie::readEntityFromNBT(nbttagcompound);
	angerLevel = nbttagcompound->getShort(u"Anger");
}

Entity *EntityPigZombie::findPlayerToAttack()
{
	if (angerLevel == 0)
	{
		return nullptr;
	}
	return EntityZombie::findPlayerToAttack();
}

void EntityPigZombie::onLivingUpdate()
{
	EntityZombie::onLivingUpdate();
}

bool EntityPigZombie::attackEntityFrom(Entity *entity, int_t i)
{
	if (dynamic_cast<EntityPlayer *>(entity) != nullptr)
	{
		std::vector<Entity *> list = worldObj->getEntitiesWithinAABBExcludingEntity(this, boundingBox->expand(32.0, 32.0, 32.0));
		for (size_t j = 0; j < list.size(); j++)
		{
			Entity *entity1 = list[j];
			if (dynamic_cast<EntityPigZombie *>(entity1) != nullptr)
			{
				EntityPigZombie *entitypigzombie = (EntityPigZombie *)entity1;
				entitypigzombie->becomeAngryAt(entity);
			}
		}
		becomeAngryAt(entity);
	}
	return EntityZombie::attackEntityFrom(entity, i);
}

void EntityPigZombie::becomeAngryAt(Entity *entity)
{
	playerToAttack = entity;
	angerLevel = 400 + rand.nextInt(400);
	randomSoundDelay = rand.nextInt(40);
}

jstring EntityPigZombie::getLivingSound() { return u"mob.zombiepig.zpig"; }
jstring EntityPigZombie::getHurtSound()   { return u"mob.zombiepig.zpighurt"; }
jstring EntityPigZombie::getDeathSound()  { return u"mob.zombiepig.zpigdeath"; }

int_t EntityPigZombie::getDropItemId()
{
	return Item::porkCooked->shiftedIndex;
}

ItemStack *EntityPigZombie::getHeldItem()
{
	return defaultHeldItem;
}
