#include "StatList.h"

#include <set>
#include <algorithm>
#include "StatBase.h"
#include "StatBasic.h"
#include "StatCrafting.h"
#include "StatCollector.h"
#include "Block.h"
#include "Item.h"
#include "ItemStack.h"
#include "BlockFlower.h"
#include "BlockGrass.h"
#include "CraftingManager.h"
#include "IRecipe.h"
#include "FurnaceRecipes.h"
#include "java/String.h"

std::map<int_t, StatBase*> StatList::statMap;
std::vector<StatBase*> StatList::allStats;
std::vector<StatBase*> StatList::generalStats;
std::vector<StatBase*> StatList::itemStats;
std::vector<StatBase*> StatList::objectCraftStats;

StatBase *StatList::startGameStat = nullptr;
StatBase *StatList::createWorldStat = nullptr;
StatBase *StatList::loadWorldStat = nullptr;
StatBase *StatList::joinMultiplayerStat = nullptr;
StatBase *StatList::leaveGameStat = nullptr;
StatBase *StatList::minutesPlayedStat = nullptr;
StatBase *StatList::distanceWalkedStat = nullptr;
StatBase *StatList::distanceSwumStat = nullptr;
StatBase *StatList::distanceFallenStat = nullptr;
StatBase *StatList::distanceClimbedStat = nullptr;
StatBase *StatList::distanceFlownStat = nullptr;
StatBase *StatList::distanceDoveStat = nullptr;
StatBase *StatList::distanceByMinecartStat = nullptr;
StatBase *StatList::distanceByBoatStat = nullptr;
StatBase *StatList::distanceByPigStat = nullptr;
StatBase *StatList::jumpStat = nullptr;
StatBase *StatList::dropStat = nullptr;
StatBase *StatList::damageDealtStat = nullptr;
StatBase *StatList::damageTakenStat = nullptr;
StatBase *StatList::deathsStat = nullptr;
StatBase *StatList::mobKillsStat = nullptr;
StatBase *StatList::playerKillsStat = nullptr;
StatBase *StatList::fishCaughtStat = nullptr;

StatBase *StatList::mineBlockStatArray[256] = {nullptr};
StatBase *StatList::craftItemStatArray[32000] = {nullptr};
StatBase *StatList::useItemStatArray[32000] = {nullptr};
StatBase *StatList::breakItemStatArray[32000] = {nullptr};

bool StatList::statsInitialized = false;
bool StatList::blockStatsInitialized = false;
bool StatList::itemStatsInitialized = false;

// Java static field initializers + static {} block, made explicit (func_27360_a was empty in Java;
// the named stats were created lazily on first class reference). Guarded so it runs exactly once.
void StatList::initStats()
{
	if (statsInitialized) return;
	statsInitialized = true;

	startGameStat       = (new StatBasic(1000, StatCollector::translateToLocal("stat.startGame")))->setIndependent()->registerStat();
	createWorldStat     = (new StatBasic(1001, StatCollector::translateToLocal("stat.createWorld")))->setIndependent()->registerStat();
	loadWorldStat       = (new StatBasic(1002, StatCollector::translateToLocal("stat.loadWorld")))->setIndependent()->registerStat();
	joinMultiplayerStat = (new StatBasic(1003, StatCollector::translateToLocal("stat.joinMultiplayer")))->setIndependent()->registerStat();
	leaveGameStat       = (new StatBasic(1004, StatCollector::translateToLocal("stat.leaveGame")))->setIndependent()->registerStat();
	jumpStat            = (new StatBasic(2010, StatCollector::translateToLocal("stat.jump")))->setIndependent()->registerStat();
	dropStat            = (new StatBasic(2011, StatCollector::translateToLocal("stat.drop")))->setIndependent()->registerStat();
	damageDealtStat     = (new StatBasic(2020, StatCollector::translateToLocal("stat.damageDealt")))->registerStat();
	damageTakenStat     = (new StatBasic(2021, StatCollector::translateToLocal("stat.damageTaken")))->registerStat();
	deathsStat          = (new StatBasic(2022, StatCollector::translateToLocal("stat.deaths")))->registerStat();
	mobKillsStat        = (new StatBasic(2023, StatCollector::translateToLocal("stat.mobKills")))->registerStat();
	playerKillsStat     = (new StatBasic(2024, StatCollector::translateToLocal("stat.playerKills")))->registerStat();
	fishCaughtStat      = (new StatBasic(2025, StatCollector::translateToLocal("stat.fishCaught")))->registerStat();

	// mineBlockStatArray = func_25153_a("stat.mineBlock", 0x1000000)
	StatBase **arr = initMineBlockStats("stat.mineBlock", 0x1000000);
	for (int_t i = 0; i < 256; i++)
		mineBlockStatArray[i] = arr[i];
	delete[] arr;

	// Java static {} block:
	minutesPlayedStat     = (new StatBasic(1100, StatCollector::translateToLocal("stat.playOneMinute"), StatBase::timeStatType))->setIndependent()->registerStat();
	distanceWalkedStat    = (new StatBasic(2000, StatCollector::translateToLocal("stat.walkOneCm"), StatBase::distanceStatType))->setIndependent()->registerStat();
	distanceSwumStat      = (new StatBasic(2001, StatCollector::translateToLocal("stat.swimOneCm"), StatBase::distanceStatType))->setIndependent()->registerStat();
	distanceFallenStat    = (new StatBasic(2002, StatCollector::translateToLocal("stat.fallOneCm"), StatBase::distanceStatType))->setIndependent()->registerStat();
	distanceClimbedStat   = (new StatBasic(2003, StatCollector::translateToLocal("stat.climbOneCm"), StatBase::distanceStatType))->setIndependent()->registerStat();
	distanceFlownStat     = (new StatBasic(2004, StatCollector::translateToLocal("stat.flyOneCm"), StatBase::distanceStatType))->setIndependent()->registerStat();
	distanceDoveStat      = (new StatBasic(2005, StatCollector::translateToLocal("stat.diveOneCm"), StatBase::distanceStatType))->setIndependent()->registerStat();
	distanceByMinecartStat= (new StatBasic(2006, StatCollector::translateToLocal("stat.minecartOneCm"), StatBase::distanceStatType))->setIndependent()->registerStat();
	distanceByBoatStat    = (new StatBasic(2007, StatCollector::translateToLocal("stat.boatOneCm"), StatBase::distanceStatType))->setIndependent()->registerStat();
	distanceByPigStat     = (new StatBasic(2008, StatCollector::translateToLocal("stat.pigOneCm"), StatBase::distanceStatType))->setIndependent()->registerStat();
	// AchievementList init is invoked explicitly from Minecraft bootstrap (not here, to avoid double-registration).
}

// func_25154_a
void StatList::initBlockStats()
{
	initStats();
	initUseItemStats(useItemStatArray, "stat.useItem", 0x1020000, 0, 256);
	initBreakItemStats(breakItemStatArray, "stat.breakItem", 0x1030000, 0, 256);
	blockStatsInitialized = true;
	initCraftingStats();
}

// func_25151_b
void StatList::initItemStats()
{
	initStats();
	initUseItemStats(useItemStatArray, "stat.useItem", 0x1020000, 256, 32000);
	initBreakItemStats(breakItemStatArray, "stat.breakItem", 0x1030000, 256, 32000);
	itemStatsInitialized = true;
	initCraftingStats();
}

// func_25157_c
void StatList::initCraftingStats()
{
	if (!blockStatsInitialized || !itemStatsInitialized) return;

	std::set<int_t> hashset;
	for (IRecipe *irecipe : CraftingManager::getInstance()->getRecipeList())
		hashset.insert(irecipe->getRecipeOutput()->itemID);
	for (auto &pair : FurnaceRecipes::smelting()->getSmeltingList())
		hashset.insert(pair.second->itemID);

	for (int_t itemId : hashset)
	{
		if (Item::itemsList[itemId] == nullptr) continue;
		std::string s = StatCollector::translateToLocalFormatted("stat.craftItem", Item::itemsList[itemId]->getStatName());
		craftItemStatArray[itemId] = (new StatCrafting(0x1010000 + itemId, s, itemId))->registerStat();
	}
	replaceAllSimilarBlocks(craftItemStatArray);
}

StatBase* StatList::getStatById(int_t i)
{
	auto it = statMap.find(i);
	return (it != statMap.end()) ? it->second : nullptr;
}

StatBase** StatList::initMineBlockStats(const std::string &s, int_t i)
{
	StatBase **astatbase = new StatBase*[256](); // value-init to nullptr (Java arrays default to null)
	for (int_t j = 0; j < 256; j++)
	{
		if (Block::blocksList[j] != nullptr && Block::blocksList[j]->getEnableStats())
		{
			std::string s1 = StatCollector::translateToLocalFormatted(s, String::toUTF8(Block::blocksList[j]->translateBlockName()));
			astatbase[j] = (new StatCrafting(i + j, s1, j))->registerStat();
			objectCraftStats.push_back((StatCrafting*)astatbase[j]);
		}
	}
	replaceAllSimilarBlocks(astatbase);
	return astatbase;
}

StatBase** StatList::initUseItemStats(StatBase **astatbase, const std::string &s, int_t i, int_t j, int_t k)
{
	if (astatbase == nullptr)
		astatbase = new StatBase*[32000]();
	for (int_t l = j; l < k; l++)
	{
		if (Item::itemsList[l] == nullptr) continue;
		std::string s1 = StatCollector::translateToLocalFormatted(s, Item::itemsList[l]->getStatName());
		astatbase[l] = (new StatCrafting(i + l, s1, l))->registerStat();
		if (l >= 256)
			itemStats.push_back((StatCrafting*)astatbase[l]);
	}
	replaceAllSimilarBlocks(astatbase);
	return astatbase;
}

StatBase** StatList::initBreakItemStats(StatBase **astatbase, const std::string &s, int_t i, int_t j, int_t k)
{
	if (astatbase == nullptr)
		astatbase = new StatBase*[32000]();
	for (int_t l = j; l < k; l++)
	{
		if (Item::itemsList[l] != nullptr && Item::itemsList[l]->isDamagable())
		{
			std::string s1 = StatCollector::translateToLocalFormatted(s, Item::itemsList[l]->getStatName());
			astatbase[l] = (new StatCrafting(i + l, s1, l))->registerStat();
		}
	}
	replaceAllSimilarBlocks(astatbase);
	return astatbase;
}

void StatList::replaceAllSimilarBlocks(StatBase **astatbase)
{
	replaceSimilarBlocks(astatbase, Block::waterStill->blockID, Block::waterMoving->blockID);
	replaceSimilarBlocks(astatbase, Block::lavaStill->blockID, Block::lavaStill->blockID);
	replaceSimilarBlocks(astatbase, Block::pumpkinLantern->blockID, Block::pumpkin->blockID);
	replaceSimilarBlocks(astatbase, Block::stoneOvenActive->blockID, Block::stoneOvenIdle->blockID);
	replaceSimilarBlocks(astatbase, Block::oreRedstoneGlowing->blockID, Block::oreRedstone->blockID);
	replaceSimilarBlocks(astatbase, Block::redstoneRepeaterActive->blockID, Block::redstoneRepeaterIdle->blockID);
	replaceSimilarBlocks(astatbase, Block::torchRedstoneActive->blockID, Block::torchRedstoneIdle->blockID);
	replaceSimilarBlocks(astatbase, Block::mushroomRed->blockID, Block::mushroomBrown->blockID);
	replaceSimilarBlocks(astatbase, Block::stairDouble->blockID, Block::stairSingle->blockID);
	replaceSimilarBlocks(astatbase, Block::grass->blockID, Block::dirt->blockID);
	replaceSimilarBlocks(astatbase, Block::tilledField->blockID, Block::dirt->blockID);
}

void StatList::replaceSimilarBlocks(StatBase **astatbase, int_t i, int_t j)
{
	if (astatbase[i] != nullptr && astatbase[j] == nullptr)
	{
		astatbase[j] = astatbase[i];
		return;
	}
	else
	{
		auto it = std::find(allStats.begin(), allStats.end(), astatbase[i]);
		if (it != allStats.end()) allStats.erase(it);
		auto it2 = std::find(objectCraftStats.begin(), objectCraftStats.end(), astatbase[i]);
		if (it2 != objectCraftStats.end()) objectCraftStats.erase(it2);
		auto it3 = std::find(generalStats.begin(), generalStats.end(), astatbase[i]);
		if (it3 != generalStats.end()) generalStats.erase(it3);
		astatbase[i] = astatbase[j];
	}
}
