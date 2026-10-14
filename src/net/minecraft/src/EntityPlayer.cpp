#include "EntityPlayer.h"

#include <cmath>
#include <vector>

#include "AchievementList.h"
#include "AxisAlignedBB.h"
#include "Block.h"
#include "BlockBed.h"
#include "ChunkCoordinates.h"
#include "Container.h"
#include "ContainerPlayer.h"
#include "DataWatcher.h"
#include "Entity.h"
#include "EntityArrow.h"
#include "EntityBoat.h"
#include "EntityCreeper.h"
#include "EntityFish.h"
#include "EntityGhast.h"
#include "EntityItem.h"
#include "EntityLiving.h"
#include "EntityMinecart.h"
#include "EntityMob.h"
#include "EntityPig.h"
#include "EntityWolf.h"
#include "EnumStatus.h"
#include "IChunkProvider.h"
#include "IInventory.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "Material.h"
#include "NBTTagCompound.h"
#include "NBTTagList.h"
#include "StatBase.h"
#include "StatList.h"
#include "TileEntityDispenser.h"
#include "TileEntityFurnace.h"
#include "TileEntitySign.h"
#include "World.h"
#include "WorldProvider.h"

EntityPlayer::EntityPlayer(World *world)
	: EntityLiving(world)
{
	inventory = new InventoryPlayer(this);
	field_9371_f = 0;
	score = 0;
	field_775_e = 0.0f;
	field_774_f = 0.0f;
	isSwinging = false;
	swingProgressInt = 0;
	dimension = 0;
	field_20066_r = field_20065_s = field_20064_t = 0.0;
	field_20063_u = field_20062_v = field_20061_w = 0.0;
	sleeping = false;
	bedChunkCoordinates = nullptr;
	sleepTimer = 0;
	field_22063_x = field_22062_y = field_22061_z = 0.0f;
	playerSpawnCoordinate = nullptr;
	startMinecartRidingCoordinate = nullptr;
	timeUntilPortal = 20;
	inPortal = false;
	timeInPortal = 0.0f;
	prevTimeInPortal = 0.0f;
	damageRemainder = 0;
	fishEntity = nullptr;
	inventorySlots = new ContainerPlayer(inventory, !world->multiplayerWorld);
	craftingInventory = inventorySlots;
	yOffset = 1.62f;
	ChunkCoordinates *chunkcoordinates = world->getSpawnPoint();
	setLocationAndAngles((double)chunkcoordinates->x + 0.5, (double)(chunkcoordinates->y + 1), (double)chunkcoordinates->z + 0.5, 0.0f, 0.0f);
	health = 20;
	field_9351_C = u"humanoid";
	field_9353_B = 180.0f;
	fireResistance = 20;
	texture = u"/mob/char.png";
}

EntityPlayer::~EntityPlayer()
{
	if (craftingInventory != inventorySlots)
		delete craftingInventory;
	delete inventorySlots;
	delete inventory;
	delete bedChunkCoordinates;
	delete playerSpawnCoordinate;
	delete startMinecartRidingCoordinate;
}

void EntityPlayer::entityInit()
{
	EntityLiving::entityInit();
	dataWatcher->addObject(16, (byte_t)0);
}

void EntityPlayer::onUpdate()
{
	if (isPlayerSleeping())
	{
		sleepTimer++;
		if (sleepTimer > 100)
		{
			sleepTimer = 100;
		}
		if (!worldObj->multiplayerWorld)
		{
			if (!isInBed())
			{
				wakeUpPlayer(true, true, false);
			}
			else if (worldObj->isDaytime())
			{
				wakeUpPlayer(false, true, true);
			}
		}
	}
	else if (sleepTimer > 0)
	{
		sleepTimer++;
		if (sleepTimer >= 110)
		{
			sleepTimer = 0;
		}
	}
	EntityLiving::onUpdate();
	if (!worldObj->multiplayerWorld && craftingInventory != nullptr && !craftingInventory->isUsableByPlayer(this))
	{
		closeScreen();
		craftingInventory = inventorySlots;
	}
	field_20066_r = field_20063_u;
	field_20065_s = field_20062_v;
	field_20064_t = field_20061_w;
	double d  = posX - field_20063_u;
	double d1 = posY - field_20062_v;
	double d2 = posZ - field_20061_w;
	double d3 = 10.0;
	if (d  >  d3) { field_20066_r = field_20063_u = posX; }
	if (d2 >  d3) { field_20064_t = field_20061_w = posZ; }
	if (d1 >  d3) { field_20065_s = field_20062_v = posY; }
	if (d  < -d3) { field_20066_r = field_20063_u = posX; }
	if (d2 < -d3) { field_20064_t = field_20061_w = posZ; }
	if (d1 < -d3) { field_20065_s = field_20062_v = posY; }
	field_20063_u += d  * 0.25;
	field_20061_w += d2 * 0.25;
	field_20062_v += d1 * 0.25;
	addStat(StatList::minutesPlayedStat, 1);
	if (ridingEntity == nullptr)
	{
		startMinecartRidingCoordinate = nullptr;
	}
}

bool EntityPlayer::isMovementBlocked()
{
	return health <= 0 || isPlayerSleeping();
}

void EntityPlayer::closeScreen()
{
	craftingInventory = inventorySlots;
}

void EntityPlayer::updateCloak()
{
	playerCloakUrl = String::fromUTF8("http://s3.amazonaws.com/MinecraftCloaks/") + username + String::fromUTF8(".png");
	cloakUrl = playerCloakUrl;
}

void EntityPlayer::updateRidden()
{
	double d  = posX;
	double d1 = posY;
	double d2 = posZ;
	EntityLiving::updateRidden();
	field_775_e = field_774_f;
	field_774_f = 0.0f;
	addMountedMovementStat(posX - d, posY - d1, posZ - d2);
}

void EntityPlayer::updatePlayerActionState()
{
	if (isSwinging)
	{
		swingProgressInt++;
		if (swingProgressInt >= 8)
		{
			swingProgressInt = 0;
			isSwinging = false;
		}
	}
	else
	{
		swingProgressInt = 0;
	}
	swingProgress = (float)swingProgressInt / 8.0f;
}

void EntityPlayer::onLivingUpdate()
{
	if (worldObj->difficultySetting == 0 && health < 20 && (ticksExisted % 20) * 12 == 0)
	{
		heal(1);
	}
	inventory->decrementAnimations();
	field_775_e = field_774_f;
	EntityLiving::onLivingUpdate();
	float f = MathHelper::sqrt_double(motionX * motionX + motionZ * motionZ);
	float f1 = (float)std::atan(-motionY * 0.20000000298023224) * 15.0f;
	if (f > 0.1f)
	{
		f = 0.1f;
	}
	if (!onGround || health <= 0)
	{
		f = 0.0f;
	}
	if (onGround || health <= 0)
	{
		f1 = 0.0f;
	}
	field_774_f += (f - field_774_f) * 0.4f;
	field_9328_R += (f1 - field_9328_R) * 0.8f;
	if (health > 0)
	{
		std::vector<Entity *> list = worldObj->getEntitiesWithinAABBExcludingEntity(this, boundingBox->expand(1.0, 0.0, 1.0));
		for (size_t i = 0; i < list.size(); i++)
		{
			Entity *entity = list[i];
			if (!entity->isDead)
			{
				collideWithPlayer(entity);
			}
		}
	}
}

void EntityPlayer::collideWithPlayer(Entity *entity)
{
	entity->onCollideWithPlayer(this);
}

int_t EntityPlayer::getScore()
{
	return score;
}

void EntityPlayer::onDeath(Entity *entity)
{
	EntityLiving::onDeath(entity);
	setSize(0.2f, 0.2f);
	setPosition(posX, posY, posZ);
	motionY = 0.10000000149011612;
	if (username == u"Notch")
	{
		dropPlayerItemWithRandomChoice(new ItemStack(Item::appleRed, 1), true);
	}
	inventory->dropAllItems();
	if (entity != nullptr)
	{
		motionX = -MathHelper::cos(((attackedAtYaw + rotationYaw) * 3.141593f) / 180.0f) * 0.1f;
		motionZ = -MathHelper::sin(((attackedAtYaw + rotationYaw) * 3.141593f) / 180.0f) * 0.1f;
	}
	else
	{
		motionX = motionZ = 0.0;
	}
	yOffset = 0.1f;
	addStat(StatList::deathsStat, 1);
}

void EntityPlayer::addToPlayerScore(Entity *entity, int_t i)
{
	score += i;
	if (dynamic_cast<EntityPlayer *>(entity) != nullptr)
	{
		addStat(StatList::playerKillsStat, 1);
	}
	else
	{
		addStat(StatList::mobKillsStat, 1);
	}
}

void EntityPlayer::dropCurrentItem()
{
	dropPlayerItemWithRandomChoice(inventory->decrStackSize(inventory->currentItem, 1), false);
}

void EntityPlayer::dropPlayerItem(ItemStack *itemstack)
{
	dropPlayerItemWithRandomChoice(itemstack, false);
}

void EntityPlayer::dropPlayerItemWithRandomChoice(ItemStack *itemstack, bool flag)
{
	if (itemstack == nullptr)
	{
		return;
	}
	EntityItem *entityitem = new EntityItem(worldObj, posX, (posY - 0.30000001192092896) + (double)getEyeHeight(), posZ, itemstack);
	entityitem->delayBeforeCanPickup = 40;
	float f = 0.1f;
	if (flag)
	{
		float f2 = rand.nextFloat() * 0.5f;
		float f4 = rand.nextFloat() * 3.141593f * 2.0f;
		entityitem->motionX = -MathHelper::sin(f4) * f2;
		entityitem->motionZ =  MathHelper::cos(f4) * f2;
		entityitem->motionY = 0.20000000298023224;
	}
	else
	{
		float f1 = 0.3f;
		entityitem->motionX = -MathHelper::sin((rotationYaw / 180.0f) * 3.141593f) * MathHelper::cos((rotationPitch / 180.0f) * 3.141593f) * f1;
		entityitem->motionZ =  MathHelper::cos((rotationYaw / 180.0f) * 3.141593f) * MathHelper::cos((rotationPitch / 180.0f) * 3.141593f) * f1;
		entityitem->motionY = -MathHelper::sin((rotationPitch / 180.0f) * 3.141593f) * f1 + 0.1f;
		f1 = 0.02f;
		float f3 = rand.nextFloat() * 3.141593f * 2.0f;
		f1 *= rand.nextFloat();
		entityitem->motionX += std::cos(f3) * (double)f1;
		entityitem->motionY += (double)((rand.nextFloat() - rand.nextFloat()) * 0.1f);
		entityitem->motionZ += std::sin(f3) * (double)f1;
	}
	joinEntityItemWithWorld(entityitem);
	addStat(StatList::dropStat, 1);
}

void EntityPlayer::joinEntityItemWithWorld(EntityItem *entityitem)
{
	worldObj->entityJoinedWorld(entityitem);
}

float EntityPlayer::getCurrentPlayerStrVsBlock(Block *block)
{
	float f = inventory->getStrVsBlock(block);
	if (isInsideOfMaterial(Material::water))
	{
		f /= 5.0f;
	}
	if (!onGround)
	{
		f /= 5.0f;
	}
	return f;
}

bool EntityPlayer::canHarvestBlock(Block *block)
{
	return inventory->canHarvestBlock(block);
}

void EntityPlayer::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	EntityLiving::readEntityFromNBT(nbttagcompound);
	NBTTagList *nbttaglist = nbttagcompound->getTagList(u"Inventory");
	inventory->readFromNBT(nbttaglist);
	dimension = nbttagcompound->getInteger(u"Dimension");
	sleeping = nbttagcompound->getBoolean(u"Sleeping");
	sleepTimer = nbttagcompound->getShort(u"SleepTimer");
	if (sleeping)
	{
		bedChunkCoordinates = new ChunkCoordinates(MathHelper::floor_double(posX), MathHelper::floor_double(posY), MathHelper::floor_double(posZ));
		wakeUpPlayer(true, true, false);
	}
	if (nbttagcompound->hasKey(u"SpawnX") && nbttagcompound->hasKey(u"SpawnY") && nbttagcompound->hasKey(u"SpawnZ"))
	{
		playerSpawnCoordinate = new ChunkCoordinates(nbttagcompound->getInteger(u"SpawnX"), nbttagcompound->getInteger(u"SpawnY"), nbttagcompound->getInteger(u"SpawnZ"));
	}
}

void EntityPlayer::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	EntityLiving::writeEntityToNBT(nbttagcompound);
	nbttagcompound->setTag(u"Inventory", inventory->writeToNBT(new NBTTagList()));
	nbttagcompound->setInteger(u"Dimension", dimension);
	nbttagcompound->setBoolean(u"Sleeping", sleeping);
	nbttagcompound->setShort(u"SleepTimer", (short_t)sleepTimer);
	if (playerSpawnCoordinate != nullptr)
	{
		nbttagcompound->setInteger(u"SpawnX", playerSpawnCoordinate->x);
		nbttagcompound->setInteger(u"SpawnY", playerSpawnCoordinate->y);
		nbttagcompound->setInteger(u"SpawnZ", playerSpawnCoordinate->z);
	}
}

void EntityPlayer::displayGUIChest(IInventory *iinventory)
{
}

void EntityPlayer::displayWorkbenchGUI(int_t i, int_t j, int_t k)
{
}

void EntityPlayer::onItemPickup(Entity *entity, int_t i)
{
}

float EntityPlayer::getEyeHeight()
{
	return 0.12f;
}

void EntityPlayer::resetHeight()
{
	yOffset = 1.62f;
}

bool EntityPlayer::attackEntityFrom(Entity *entity, int_t i)
{
	entityAge = 0;
	if (health <= 0)
	{
		return false;
	}
	if (isPlayerSleeping() && !worldObj->multiplayerWorld)
	{
		wakeUpPlayer(true, true, false);
	}
	if (dynamic_cast<EntityMob *>(entity) != nullptr || dynamic_cast<EntityArrow *>(entity) != nullptr)
	{
		if (worldObj->difficultySetting == 0) { i = 0; }
		if (worldObj->difficultySetting == 1) { i = i / 3 + 1; }
		if (worldObj->difficultySetting == 3) { i = (i * 3) / 2; }
	}
	if (i == 0)
	{
		return false;
	}
	Entity *obj = entity;
	EntityArrow *arrow = dynamic_cast<EntityArrow *>(obj);
	if (arrow != nullptr && arrow->owner != nullptr)
	{
		obj = arrow->owner;
	}
	EntityLiving *eliving = dynamic_cast<EntityLiving *>(obj);
	if (eliving != nullptr)
	{
		alertWolves(eliving, false);
	}
	addStat(StatList::damageTakenStat, i);
	return EntityLiving::attackEntityFrom(entity, i);
}

bool EntityPlayer::func_27025_G()
{
	return false;
}

void EntityPlayer::alertWolves(EntityLiving *entityliving, bool flag)
{
	if (dynamic_cast<EntityCreeper *>(entityliving) != nullptr || dynamic_cast<EntityGhast *>(entityliving) != nullptr)
	{
		return;
	}
	EntityWolf *wolfCheck = dynamic_cast<EntityWolf *>(entityliving);
	if (wolfCheck != nullptr)
	{
		if (wolfCheck->isWolfTamed() && username == wolfCheck->getWolfOwner())
		{
			return;
		}
	}
	if (dynamic_cast<EntityPlayer *>(entityliving) != nullptr && !func_27025_G())
	{
		return;
	}
	// Java: worldObj.getEntitiesWithinAABB(EntityWolf.class, ...)
	std::vector<Entity *> list = worldObj->getEntitiesWithinAABBExcludingEntity(this, AxisAlignedBB::getBoundingBoxFromPool(posX, posY, posZ, posX + 1.0, posY + 1.0, posZ + 1.0)->expand(16.0, 4.0, 16.0));
	for (size_t i = 0; i < list.size(); i++)
	{
		Entity *entity = list[i];
		EntityWolf *entitywolf1 = dynamic_cast<EntityWolf *>(entity);
		if (entitywolf1 == nullptr) continue;
		if (entitywolf1->isWolfTamed() && entitywolf1->getTarget() == nullptr && username == entitywolf1->getWolfOwner() && (!flag || !entitywolf1->isWolfSitting()))
		{
			entitywolf1->setWolfSitting(false);
			entitywolf1->setTarget(entityliving);
		}
	}
}

void EntityPlayer::damageEntity(int_t i)
{
	int_t j = 25 - inventory->getTotalArmorValue();
	int_t k = i * j + damageRemainder;
	inventory->damageArmor(i);
	i = k / 25;
	damageRemainder = k % 25;
	EntityLiving::damageEntity(i);
}

void EntityPlayer::displayGUIFurnace(TileEntityFurnace *tileentityfurnace) {}
void EntityPlayer::displayGUIDispenser(TileEntityDispenser *tileentitydispenser) {}
void EntityPlayer::displayGUIEditSign(TileEntitySign *tileentitysign) {}

void EntityPlayer::useCurrentItemOnEntity(Entity *entity)
{
	if (entity->interact(this))
	{
		return;
	}
	ItemStack *itemstack = getCurrentEquippedItem();
	EntityLiving *living = dynamic_cast<EntityLiving *>(entity);
	if (itemstack != nullptr && living != nullptr)
	{
		itemstack->useItemOnEntity(living);
		if (itemstack->stackSize <= 0)
		{
			itemstack->func_1097_a(this);
			destroyCurrentEquippedItem();
		}
	}
}

ItemStack *EntityPlayer::getCurrentEquippedItem()
{
	return inventory->getCurrentItem();
}

void EntityPlayer::destroyCurrentEquippedItem()
{
	inventory->setInventorySlotContents(inventory->currentItem, nullptr);
}

double EntityPlayer::getYOffset()
{
	return (double)(yOffset - 0.5f);
}

void EntityPlayer::swingItem()
{
	swingProgressInt = -1;
	isSwinging = true;
}

void EntityPlayer::attackTargetEntityWithCurrentItem(Entity *entity)
{
	int_t i = inventory->getDamageVsEntity(entity);
	if (i > 0)
	{
		if (motionY < 0.0)
		{
			i++;
		}
		entity->attackEntityFrom(this, i);
		ItemStack *itemstack = getCurrentEquippedItem();
		EntityLiving *living = dynamic_cast<EntityLiving *>(entity);
		if (itemstack != nullptr && living != nullptr)
		{
			itemstack->hitEntity(living, this);
			if (itemstack->stackSize <= 0)
			{
				itemstack->func_1097_a(this);
				destroyCurrentEquippedItem();
			}
		}
		if (living != nullptr)
		{
			if (entity->isEntityAlive())
			{
				alertWolves(living, true);
			}
			addStat(StatList::damageDealtStat, i);
		}
	}
}

void EntityPlayer::respawnPlayer()
{
}

void EntityPlayer::onItemStackChanged(ItemStack *itemstack)
{
}

bool EntityPlayer::isEntityInsideOpaqueBlock()
{
	return !sleeping && Entity::isEntityInsideOpaqueBlock();
}

EnumStatus EntityPlayer::sleepInBedAt(int_t i, int_t j, int_t k)
{
	if (!worldObj->multiplayerWorld)
	{
		if (isPlayerSleeping() || !isEntityAlive())
		{
			return EnumStatus::OTHER_PROBLEM;
		}
		if (worldObj->worldProvider->isNether)
		{
			return EnumStatus::NOT_POSSIBLE_HERE;
		}
		if (worldObj->isDaytime())
		{
			return EnumStatus::NOT_POSSIBLE_NOW;
		}
		if (std::abs(posX - (double)i) > 3.0 || std::abs(posY - (double)j) > 2.0 || std::abs(posZ - (double)k) > 3.0)
		{
			return EnumStatus::TOO_FAR_AWAY;
		}
	}
	setSize(0.2f, 0.2f);
	yOffset = 0.2f;
	if (worldObj->blockExists(i, j, k))
	{
		int_t l = worldObj->getBlockMetadata(i, j, k);
		int_t i1 = BlockBed::getDirectionFromMetadata(l);
		float f  = 0.5f;
		float f1 = 0.5f;
		switch (i1)
		{
			case 0: f1 = 0.9f; break;
			case 2: f1 = 0.1f; break;
			case 1: f  = 0.1f; break;
			case 3: f  = 0.9f; break;
		}
		func_22052_e(i1);
		setPosition((double)((float)i + f), (double)((float)j + 0.9375f), (double)((float)k + f1));
	}
	else
	{
		setPosition((double)((float)i + 0.5f), (double)((float)j + 0.9375f), (double)((float)k + 0.5f));
	}
	sleeping = true;
	sleepTimer = 0;
	bedChunkCoordinates = new ChunkCoordinates(i, j, k);
	motionX = motionZ = motionY = 0.0;
	if (!worldObj->multiplayerWorld)
	{
		worldObj->updateAllPlayersSleepingFlag();
	}
	return EnumStatus::OK;
}

void EntityPlayer::func_22052_e(int_t i)
{
	field_22063_x = 0.0f;
	field_22061_z = 0.0f;
	switch (i)
	{
		case 0: field_22061_z = -1.8f; break;
		case 2: field_22061_z =  1.8f; break;
		case 1: field_22063_x =  1.8f; break;
		case 3: field_22063_x = -1.8f; break;
	}
}

void EntityPlayer::wakeUpPlayer(bool flag, bool flag1, bool flag2)
{
	setSize(0.6f, 1.8f);
	resetHeight();
	ChunkCoordinates *chunkcoordinates = bedChunkCoordinates;
	if (chunkcoordinates != nullptr && worldObj->getBlockId(chunkcoordinates->x, chunkcoordinates->y, chunkcoordinates->z) == Block::blockBed->blockID)
	{
		BlockBed::setBedOccupied(worldObj, chunkcoordinates->x, chunkcoordinates->y, chunkcoordinates->z, false);
		ChunkCoordinates *chunkcoordinates2 = BlockBed::getNearestEmptyChunkCoordinates(worldObj, chunkcoordinates->x, chunkcoordinates->y, chunkcoordinates->z, 0);
		if (chunkcoordinates2 == nullptr)
		{
			chunkcoordinates2 = new ChunkCoordinates(chunkcoordinates->x, chunkcoordinates->y + 1, chunkcoordinates->z);
		}
		setPosition((double)((float)chunkcoordinates2->x + 0.5f), (double)((float)chunkcoordinates2->y + yOffset + 0.1f), (double)((float)chunkcoordinates2->z + 0.5f));
	}
	sleeping = false;
	if (!worldObj->multiplayerWorld && flag1)
	{
		worldObj->updateAllPlayersSleepingFlag();
	}
	sleepTimer = flag ? 0 : 100;
	if (flag2)
	{
		setPlayerSpawnCoordinate(bedChunkCoordinates);
	}
}

bool EntityPlayer::isInBed()
{
	return worldObj->getBlockId(bedChunkCoordinates->x, bedChunkCoordinates->y, bedChunkCoordinates->z) == Block::blockBed->blockID;
}

ChunkCoordinates *EntityPlayer::func_25060_a(World *world, ChunkCoordinates *chunkcoordinates)
{
	IChunkProvider *ichunkprovider = world->getIChunkProvider();
	ichunkprovider->prepareChunk((chunkcoordinates->x - 3) >> 4, (chunkcoordinates->z - 3) >> 4);
	ichunkprovider->prepareChunk((chunkcoordinates->x + 3) >> 4, (chunkcoordinates->z - 3) >> 4);
	ichunkprovider->prepareChunk((chunkcoordinates->x - 3) >> 4, (chunkcoordinates->z + 3) >> 4);
	ichunkprovider->prepareChunk((chunkcoordinates->x + 3) >> 4, (chunkcoordinates->z + 3) >> 4);
	if (world->getBlockId(chunkcoordinates->x, chunkcoordinates->y, chunkcoordinates->z) != Block::blockBed->blockID)
	{
		return nullptr;
	}
	return BlockBed::getNearestEmptyChunkCoordinates(world, chunkcoordinates->x, chunkcoordinates->y, chunkcoordinates->z, 0);
}

float EntityPlayer::getBedOrientationInDegrees()
{
	if (bedChunkCoordinates != nullptr)
	{
		int_t i = worldObj->getBlockMetadata(bedChunkCoordinates->x, bedChunkCoordinates->y, bedChunkCoordinates->z);
		int_t j = BlockBed::getDirectionFromMetadata(i);
		switch (j)
		{
			case 0: return 90.0f;
			case 1: return 0.0f;
			case 2: return 270.0f;
			case 3: return 180.0f;
		}
	}
	return 0.0f;
}

bool EntityPlayer::isPlayerSleeping()
{
	return sleeping;
}

bool EntityPlayer::isPlayerFullyAsleep()
{
	return sleeping && sleepTimer >= 100;
}

int_t EntityPlayer::func_22060_M()
{
	return sleepTimer;
}

void EntityPlayer::addChatMessage(const std::string &s)
{
}

ChunkCoordinates *EntityPlayer::getPlayerSpawnCoordinate()
{
	return playerSpawnCoordinate;
}

void EntityPlayer::setPlayerSpawnCoordinate(ChunkCoordinates *chunkcoordinates)
{
	if (chunkcoordinates != nullptr)
	{
		playerSpawnCoordinate = new ChunkCoordinates(chunkcoordinates);
	}
	else
	{
		playerSpawnCoordinate = nullptr;
	}
}

void EntityPlayer::triggerAchievement(StatBase *statbase)
{
	addStat(statbase, 1);
}

void EntityPlayer::addStat(StatBase *statbase, int_t i)
{
}

void EntityPlayer::jump()
{
	EntityLiving::jump();
	addStat(StatList::jumpStat, 1);
}

void EntityPlayer::moveEntityWithHeading(float f, float f1)
{
	double d  = posX;
	double d1 = posY;
	double d2 = posZ;
	EntityLiving::moveEntityWithHeading(f, f1);
	addMovementStat(posX - d, posY - d1, posZ - d2);
}

void EntityPlayer::addMovementStat(double d, double d1, double d2)
{
	if (ridingEntity != nullptr)
	{
		return;
	}
	if (isInsideOfMaterial(Material::water))
	{
		int_t i = (int_t)std::round(MathHelper::sqrt_double(d * d + d1 * d1 + d2 * d2) * 100.0f);
		if (i > 0)
		{
			addStat(StatList::distanceDoveStat, i);
		}
	}
	else if (isInWater())
	{
		int_t j = (int_t)std::round(MathHelper::sqrt_double(d * d + d2 * d2) * 100.0f);
		if (j > 0)
		{
			addStat(StatList::distanceSwumStat, j);
		}
	}
	else if (isOnLadder())
	{
		if (d1 > 0.0)
		{
			addStat(StatList::distanceClimbedStat, (int_t)std::round(d1 * 100.0));
		}
	}
	else if (onGround)
	{
		int_t k = (int_t)std::round(MathHelper::sqrt_double(d * d + d2 * d2) * 100.0f);
		if (k > 0)
		{
			addStat(StatList::distanceWalkedStat, k);
		}
	}
	else
	{
		int_t l = (int_t)std::round(MathHelper::sqrt_double(d * d + d2 * d2) * 100.0f);
		if (l > 25)
		{
			addStat(StatList::distanceFlownStat, l);
		}
	}
}

void EntityPlayer::addMountedMovementStat(double d, double d1, double d2)
{
	if (ridingEntity != nullptr)
	{
		int_t i = (int_t)std::round(MathHelper::sqrt_double(d * d + d1 * d1 + d2 * d2) * 100.0f);
		if (i > 0)
		{
			if (dynamic_cast<EntityMinecart *>(ridingEntity) != nullptr)
			{
				addStat(StatList::distanceByMinecartStat, i);
				if (startMinecartRidingCoordinate == nullptr)
				{
					startMinecartRidingCoordinate = new ChunkCoordinates(MathHelper::floor_double(posX), MathHelper::floor_double(posY), MathHelper::floor_double(posZ));
				}
				else if (startMinecartRidingCoordinate->getSqDistanceTo(MathHelper::floor_double(posX), MathHelper::floor_double(posY), MathHelper::floor_double(posZ)) >= 1000.0)
				{
					addStat(AchievementList::onARail, 1);
				}
			}
			else if (dynamic_cast<EntityBoat *>(ridingEntity) != nullptr)
			{
				addStat(StatList::distanceByBoatStat, i);
			}
			else if (dynamic_cast<EntityPig *>(ridingEntity) != nullptr)
			{
				addStat(StatList::distanceByPigStat, i);
			}
		}
	}
}

void EntityPlayer::fall(float f)
{
	if (f >= 2.0f)
	{
		addStat(StatList::distanceFallenStat, (int_t)std::round((double)f * 100.0));
	}
	EntityLiving::fall(f);
}

void EntityPlayer::onKillEntity(EntityLiving *entityliving)
{
	if (dynamic_cast<EntityMob *>(entityliving) != nullptr)
	{
		triggerAchievement(AchievementList::killEnemy);
	}
}

int_t EntityPlayer::getItemIcon(ItemStack *itemstack)
{
	int_t i = EntityLiving::getItemIcon(itemstack);
	if (itemstack->itemID == Item::fishingRod->shiftedIndex && fishEntity != nullptr)
	{
		i = itemstack->getIconIndex() + 16;
	}
	return i;
}

void EntityPlayer::setInPortal()
{
	if (timeUntilPortal > 0)
	{
		timeUntilPortal = 10;
		return;
	}
	inPortal = true;
}
