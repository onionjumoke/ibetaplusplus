#include "AchievementList.h"

#include "Achievement.h"
#include "Item.h"
#include "Block.h"
#include <cstdio>

int_t AchievementList::minDisplayColumn = 0;
int_t AchievementList::minDisplayRow = 0;
int_t AchievementList::maxDisplayColumn = 0;
int_t AchievementList::maxDisplayRow = 0;

std::vector<Achievement*> AchievementList::achievementList;

Achievement *AchievementList::openInventory = nullptr;
Achievement *AchievementList::mineWood = nullptr;
Achievement *AchievementList::buildWorkBench = nullptr;
Achievement *AchievementList::buildPickaxe = nullptr;
Achievement *AchievementList::buildFurnace = nullptr;
Achievement *AchievementList::acquireIron = nullptr;
Achievement *AchievementList::buildHoe = nullptr;
Achievement *AchievementList::makeBread = nullptr;
Achievement *AchievementList::bakeCake = nullptr;
Achievement *AchievementList::buildBetterPickaxe = nullptr;
Achievement *AchievementList::cookFish = nullptr;
Achievement *AchievementList::onARail = nullptr;
Achievement *AchievementList::buildSword = nullptr;
Achievement *AchievementList::killEnemy = nullptr;
Achievement *AchievementList::killCow = nullptr;
Achievement *AchievementList::flyPig = nullptr;

void AchievementList::initialize()
{
    openInventory        = (new Achievement(0,  "openInventory",      0,  0,  Item::book,             nullptr))->setIndependent()->registerAchievement();
    mineWood             = (new Achievement(1,  "mineWood",           2,  1,  Block::wood,             openInventory))->registerAchievement();
    buildWorkBench       = (new Achievement(2,  "buildWorkBench",     4, -1,  Block::workbench,        mineWood))->registerAchievement();
    buildPickaxe         = (new Achievement(3,  "buildPickaxe",       4,  2,  Item::pickaxeWood,       buildWorkBench))->registerAchievement();
    buildFurnace         = (new Achievement(4,  "buildFurnace",       3,  4,  Block::stoneOvenActive,  buildPickaxe))->registerAchievement();
    acquireIron          = (new Achievement(5,  "acquireIron",        1,  4,  Item::ingotIron,         buildFurnace))->registerAchievement();
    buildHoe             = (new Achievement(6,  "buildHoe",           2, -3,  Item::hoeWood,           buildWorkBench))->registerAchievement();
    makeBread            = (new Achievement(7,  "makeBread",         -1, -3,  Item::bread,             buildHoe))->registerAchievement();
    bakeCake             = (new Achievement(8,  "bakeCake",           0, -5,  Item::cake,              buildHoe))->registerAchievement();
    buildBetterPickaxe   = (new Achievement(9,  "buildBetterPickaxe", 6,  2,  Item::pickaxeStone,      buildPickaxe))->registerAchievement();
    cookFish             = (new Achievement(10, "cookFish",           2,  6,  Item::fishCooked,        buildFurnace))->registerAchievement();
    onARail              = (new Achievement(11, "onARail",            2,  3,  Block::rail,             acquireIron))->setSpecial()->registerAchievement();
    buildSword           = (new Achievement(12, "buildSword",         6, -1,  Item::swordWood,         buildWorkBench))->registerAchievement();
    killEnemy            = (new Achievement(13, "killEnemy",          8, -1,  Item::bone,              buildSword))->registerAchievement();
    killCow              = (new Achievement(14, "killCow",            7, -3,  Item::leather,           buildSword))->registerAchievement();
    flyPig               = (new Achievement(15, "flyPig",             8, -4,  Item::saddle,            killCow))->setSpecial()->registerAchievement();

    printf("%d achievements\n", (int)achievementList.size());
}
