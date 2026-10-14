#include "EntityPlayerSP.h"

#include <atomic>

#include "GuiIngame.h"
#include "Material.h"
#include "MathHelper.h"
#include "Minecraft.h"
#include "MovementInput.h"
#include "Session.h"
#include "SoundManager.h"
#include "StatFileWriter.h"
#include "World.h"
#include "java/String.h"
#include "AchievementList.h"
#include "AxisAlignedBB.h"
#include "EntityPickupFX.h"
#include "GuiChest.h"
#include "GuiCrafting.h"
#ifdef MC_IOS
#include "GuiCraftingMenu.h"
#endif
#include "GuiDispenser.h"
#include "GuiEditSign.h"
#include "GuiFurnace.h"
#include "InventoryPlayer.h"
#include "Achievement.h"
#include "EffectRenderer.h"
#include "GuiAchievement.h"
#include "NBTTagCompound.h"

EntityPlayerSP::EntityPlayerSP(Minecraft *minecraft, World *world, Session *session, int_t i)
	: EntityPlayer(world)
	, movementInput(nullptr)
	, mc(minecraft)
{
	ensureEntityInit();
	dimension = i;
	if (session != nullptr)
	{
		if (!session->username.empty())
			skinUrl = String::fromUTF8("http://s3.amazonaws.com/MinecraftSkins/" + session->username + ".png");
		username = String::fromUTF8(session->username);
	}
}

EntityPlayerSP::~EntityPlayerSP()
{
	delete movementInput;
}

void EntityPlayerSP::moveEntity(double d, double d1, double d2)
{
	EntityPlayer::moveEntity(d, d1, d2);
}

void EntityPlayerSP::updatePlayerActionState()
{
	EntityPlayer::updatePlayerActionState();
	if (movementInput != nullptr)
	{
		moveStrafing = movementInput->moveStrafe;
		moveForward = movementInput->moveForward;
		isJumping = movementInput->jump;
	}
}

void EntityPlayerSP::onLivingUpdate()
{
	if (!mc->statFileWriter->hasAchievementUnlocked(AchievementList::openInventory))
		mc->guiAchievement->queueAchievementInformation(AchievementList::openInventory);

	prevTimeInPortal = timeInPortal;
	if (inPortal)
	{
		if (!worldObj->multiplayerWorld && ridingEntity != nullptr)
			mountEntity(nullptr);
		if (mc->currentScreen != nullptr)
			mc->displayGuiScreen(nullptr);
		if (timeInPortal == 0.0f)
			mc->sndManager->playSoundFX(u"portal.trigger", 1.0f, rand.nextFloat() * 0.4f + 0.8f);
		timeInPortal += 0.0125f;
		if (timeInPortal >= 1.0f)
		{
			timeInPortal = 1.0f;
			if (!worldObj->multiplayerWorld)
			{
				timeUntilPortal = 10;
				mc->sndManager->playSoundFX(u"portal.travel", 1.0f, rand.nextFloat() * 0.4f + 0.8f);
				mc->usePortal();
			}
		}
		inPortal = false;
	}
	else
	{
		if (timeInPortal > 0.0f)
			timeInPortal -= 0.05f;
		if (timeInPortal < 0.0f)
			timeInPortal = 0.0f;
	}
	if (timeUntilPortal > 0)
		timeUntilPortal--;
	if (movementInput != nullptr)
		movementInput->updatePlayerMoveState(this);
	if (movementInput != nullptr && movementInput->sneak && ySize < 0.2f)
		ySize = 0.2f;
	pushOutOfBlocks(posX - (double)width * 0.35, boundingBox->minY + 0.5, posZ + (double)width * 0.35);
	pushOutOfBlocks(posX - (double)width * 0.35, boundingBox->minY + 0.5, posZ - (double)width * 0.35);
	pushOutOfBlocks(posX + (double)width * 0.35, boundingBox->minY + 0.5, posZ - (double)width * 0.35);
	pushOutOfBlocks(posX + (double)width * 0.35, boundingBox->minY + 0.5, posZ + (double)width * 0.35);
	EntityPlayer::onLivingUpdate();
}

void EntityPlayerSP::resetPlayerKeyState()
{
	if (movementInput != nullptr)
		movementInput->resetKeyState();
}

void EntityPlayerSP::handleKeyPress(int_t i, bool flag)
{
	if (movementInput != nullptr)
		movementInput->checkKeyForMovementInput(i, flag);
}

void EntityPlayerSP::writeEntityToNBT(NBTTagCompound *nbttagcompound)
{
	EntityPlayer::writeEntityToNBT(nbttagcompound);
	nbttagcompound->setInteger(u"Score", score);
}

void EntityPlayerSP::readEntityFromNBT(NBTTagCompound *nbttagcompound)
{
	EntityPlayer::readEntityFromNBT(nbttagcompound);
	score = nbttagcompound->getInteger(u"Score");
}

void EntityPlayerSP::closeScreen()
{
	EntityPlayer::closeScreen();
	mc->displayGuiScreen(nullptr);
}

void EntityPlayerSP::displayGUIEditSign(TileEntitySign *tileentitysign)
{
	mc->displayGuiScreen(new GuiEditSign(tileentitysign));
}

void EntityPlayerSP::displayGUIChest(IInventory *iinventory)
{
	mc->displayGuiScreen(new GuiChest(inventory, iinventory));
}

void EntityPlayerSP::displayWorkbenchGUI(int_t i, int_t j, int_t k)
{
#ifdef MC_IOS
	// Touch: MCPE-style crafting screen (scrollable recipe list, flat icons,
	// tap to craft) instead of the desktop 3x3 drag grid.
	mc->displayGuiScreen(new GuiCraftingMenu(nullptr, this, true));
#else
	mc->displayGuiScreen(new GuiCrafting(inventory, worldObj, i, j, k));
#endif
}

void EntityPlayerSP::displayGUIFurnace(TileEntityFurnace *tileentityfurnace)
{
	mc->displayGuiScreen(new GuiFurnace(inventory, tileentityfurnace));
}

void EntityPlayerSP::displayGUIDispenser(TileEntityDispenser *tileentitydispenser)
{
	mc->displayGuiScreen(new GuiDispenser(inventory, tileentitydispenser));
}

void EntityPlayerSP::onItemPickup(Entity *entity, int_t i)
{
	(void)i;
	mc->effectRenderer->addEffect(new EntityPickupFX(mc->theWorld, entity, this, -0.5f));
}

int_t EntityPlayerSP::getPlayerArmorValue()
{
	return inventory->getTotalArmorValue();
}

void EntityPlayerSP::sendChatMessage(const std::string &)
{
}

bool EntityPlayerSP::isSneaking()
{
	return movementInput != nullptr && movementInput->sneak && !sleeping;
}

void EntityPlayerSP::setHealth(int_t i)
{
	int_t j = health - i;
	if (j <= 0)
	{
		health = i;
		if (j < 0)
			heartsLife = heartsHalvesLife / 2;
	}
	else
	{
		field_9346_af = j;
		prevHealth = health;
		heartsLife = heartsHalvesLife;
		damageEntity(j);
		hurtTime = maxHurtTime = 10;
	}
}

void EntityPlayerSP::respawnPlayer()
{
	mc->respawn(false, 0);
}

void EntityPlayerSP::func_6420_o()
{
	//??
}

void EntityPlayerSP::addChatMessage(const std::string &s)
{
	mc->ingameGUI->addChatMessageTranslate(s);
}

void EntityPlayerSP::addChatMessage(const char16_t *s)
{
	mc->ingameGUI->addChatMessageTranslate(String::toUTF8(s));
}

void EntityPlayerSP::addStat(StatBase *statbase, int_t i)
{
	if (statbase == nullptr)
		return;
	if (statbase->isAchievement())
	{
		Achievement *achievement = static_cast<Achievement *>(statbase);
		if (achievement->parentAchievement == nullptr || mc->statFileWriter->hasAchievementUnlocked(achievement->parentAchievement))
		{
			if (!mc->statFileWriter->hasAchievementUnlocked(achievement))
				mc->guiAchievement->queueTakenAchievement(achievement);
			mc->statFileWriter->readStat(statbase, i);
		}
	}
	else
	{
		mc->statFileWriter->readStat(statbase, i);
	}
}

bool EntityPlayerSP::isBlockTranslucent(int_t i, int_t j, int_t k)
{
	return worldObj->isBlockNormalCube(i, j, k);
}

bool EntityPlayerSP::pushOutOfBlocks(double d, double d1, double d2)
{
	int_t i = MathHelper::floor_double(d);
	int_t j = MathHelper::floor_double(d1);
	int_t k = MathHelper::floor_double(d2);
	double d3 = d - (double)i;
	double d4 = d2 - (double)k;
	if (isBlockTranslucent(i, j, k) || isBlockTranslucent(i, j + 1, k))
	{
		bool flag = !isBlockTranslucent(i - 1, j, k) && !isBlockTranslucent(i - 1, j + 1, k);
		bool flag1 = !isBlockTranslucent(i + 1, j, k) && !isBlockTranslucent(i + 1, j + 1, k);
		bool flag2 = !isBlockTranslucent(i, j, k - 1) && !isBlockTranslucent(i, j + 1, k - 1);
		bool flag3 = !isBlockTranslucent(i, j, k + 1) && !isBlockTranslucent(i, j + 1, k + 1);
		byte_t byte0 = -1;
		double d5 = 9999.0;
		if (flag && d3 < d5) { d5 = d3; byte0 = 0; }
		if (flag1 && 1.0 - d3 < d5) { d5 = 1.0 - d3; byte0 = 1; }
		if (flag2 && d4 < d5) { d5 = d4; byte0 = 4; }
		if (flag3 && 1.0 - d4 < d5) { byte0 = 5; }
		float f = 0.1f;
		if (byte0 == 0) motionX = -f;
		if (byte0 == 1) motionX = f;
		if (byte0 == 4) motionZ = -f;
		if (byte0 == 5) motionZ = f;
	}
	return false;
}
