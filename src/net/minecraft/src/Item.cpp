#include "Item.h"
#include "ItemStack.h"
#include "ItemSpade.h"
#include "ItemPickaxe.h"
#include "ItemAxe.h"
#include "ItemSword.h"
#include "ItemHoe.h"
#include "ItemFood.h"
#include "ItemSoup.h"
#include "ItemSeeds.h"
#include "ItemArmor.h"
#include "ItemBow.h"
#include "ItemCoal.h"
#include "ItemFlintAndSteel.h"
#include "ItemPainting.h"
#include "ItemSign.h"
#include "ItemDoor.h"
#include "ItemBucket.h"
#include "ItemMinecart.h"
#include "ItemSaddle.h"
#include "ItemRedstone.h"
#include "ItemSnowball.h"
#include "ItemBoat.h"
#include "ItemReed.h"
#include "ItemEgg.h"
#include "ItemFishingRod.h"
#include "ItemDye.h"
#include "ItemBed.h"
#include "ItemCookie.h"
#include "ItemMap.h"
#include "ItemShears.h"
#include "ItemRecord.h"
#include "Block.h"
#include "EntityPlayer.h"
#include "EntityLiving.h"
#include "World.h"
#include "StatCollector.h"
#include "StatList.h"
#include <iostream>
#include "Material.h"

Item* Item::itemsList[32000] = { nullptr };
Random Item::itemRand;

Item* Item::shovelSteel = nullptr;
Item* Item::pickaxeSteel = nullptr;
Item* Item::axeSteel = nullptr;
Item* Item::flintAndSteel = nullptr;
Item* Item::appleRed = nullptr;
Item* Item::bow = nullptr;
Item* Item::arrow = nullptr;
Item* Item::coal = nullptr;
Item* Item::diamond = nullptr;
Item* Item::ingotIron = nullptr;
Item* Item::ingotGold = nullptr;
Item* Item::swordSteel = nullptr;
Item* Item::swordWood = nullptr;
Item* Item::shovelWood = nullptr;
Item* Item::pickaxeWood = nullptr;
Item* Item::axeWood = nullptr;
Item* Item::swordStone = nullptr;
Item* Item::shovelStone = nullptr;
Item* Item::pickaxeStone = nullptr;
Item* Item::axeStone = nullptr;
Item* Item::swordDiamond = nullptr;
Item* Item::shovelDiamond = nullptr;
Item* Item::pickaxeDiamond = nullptr;
Item* Item::axeDiamond = nullptr;
Item* Item::stick = nullptr;
Item* Item::bowlEmpty = nullptr;
Item* Item::bowlSoup = nullptr;
Item* Item::swordGold = nullptr;
Item* Item::shovelGold = nullptr;
Item* Item::pickaxeGold = nullptr;
Item* Item::axeGold = nullptr;
Item* Item::silk = nullptr;
Item* Item::feather = nullptr;
Item* Item::gunpowder = nullptr;
Item* Item::hoeWood = nullptr;
Item* Item::hoeStone = nullptr;
Item* Item::hoeSteel = nullptr;
Item* Item::hoeDiamond = nullptr;
Item* Item::hoeGold = nullptr;
Item* Item::seeds = nullptr;
Item* Item::wheat = nullptr;
Item* Item::bread = nullptr;
Item* Item::helmetLeather = nullptr;
Item* Item::plateLeather = nullptr;
Item* Item::legsLeather = nullptr;
Item* Item::bootsLeather = nullptr;
Item* Item::helmetChain = nullptr;
Item* Item::plateChain = nullptr;
Item* Item::legsChain = nullptr;
Item* Item::bootsChain = nullptr;
Item* Item::helmetSteel = nullptr;
Item* Item::plateSteel = nullptr;
Item* Item::legsSteel = nullptr;
Item* Item::bootsSteel = nullptr;
Item* Item::helmetDiamond = nullptr;
Item* Item::plateDiamond = nullptr;
Item* Item::legsDiamond = nullptr;
Item* Item::bootsDiamond = nullptr;
Item* Item::helmetGold = nullptr;
Item* Item::plateGold = nullptr;
Item* Item::legsGold = nullptr;
Item* Item::bootsGold = nullptr;
Item* Item::flint = nullptr;
Item* Item::porkRaw = nullptr;
Item* Item::porkCooked = nullptr;
Item* Item::painting = nullptr;
Item* Item::appleGold = nullptr;
Item* Item::sign = nullptr;
Item* Item::doorWood = nullptr;
Item* Item::bucketEmpty = nullptr;
Item* Item::bucketWater = nullptr;
Item* Item::bucketLava = nullptr;
Item* Item::minecartEmpty = nullptr;
Item* Item::saddle = nullptr;
Item* Item::doorSteel = nullptr;
Item* Item::redstone = nullptr;
Item* Item::snowball = nullptr;
Item* Item::boat = nullptr;
Item* Item::leather = nullptr;
Item* Item::bucketMilk = nullptr;
Item* Item::brick = nullptr;
Item* Item::clay = nullptr;
Item* Item::reed = nullptr;
Item* Item::paper = nullptr;
Item* Item::book = nullptr;
Item* Item::slimeBall = nullptr;
Item* Item::minecartCrate = nullptr;
Item* Item::minecartPowered = nullptr;
Item* Item::egg = nullptr;
Item* Item::compass = nullptr;
Item* Item::fishingRod = nullptr;
Item* Item::pocketSundial = nullptr;
Item* Item::lightStoneDust = nullptr;
Item* Item::fishRaw = nullptr;
Item* Item::fishCooked = nullptr;
Item* Item::dyePowder = nullptr;
Item* Item::bone = nullptr;
Item* Item::sugar = nullptr;
Item* Item::cake = nullptr;
Item* Item::bed = nullptr;
Item* Item::redstoneRepeater = nullptr;
Item* Item::cookie = nullptr;
Item* Item::mapItem = nullptr;
Item* Item::shears = nullptr;
Item* Item::record13 = nullptr;
Item* Item::recordCat = nullptr;

Item::Item(int i) {
    maxStackSize = 64;
    maxDamage = 0;
    bFull3D = false;
    hasSubtypes = false;
    containerItem = nullptr;
    shiftedIndex = 256 + i;
    if (itemsList[256 + i] != nullptr) {
        std::cout << "CONFLICT @ " << i << std::endl;
    }
    itemsList[256 + i] = this;
}

Item* Item::setIconIndex(int i) {
    iconIndex = i;
    return this;
}

Item* Item::setMaxStackSize(int i) {
    maxStackSize = i;
    return this;
}

Item* Item::setIconCoord(int i, int j) {
    iconIndex = i + j * 16;
    return this;
}

int Item::getIconFromDamage(int i) {
    return iconIndex;
}

int Item::getIconIndex(ItemStack* itemstack) {
    return getIconFromDamage(itemstack->getItemDamage());
}

bool Item::onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
    return false;
}

float Item::getStrVsBlock(ItemStack* itemstack, Block* block) {
    return 1.0f;
}

ItemStack* Item::onItemRightClick(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) {
    return itemstack;
}

int Item::getItemStackLimit() {
    return maxStackSize;
}

int Item::getPlacedBlockMetadata(int i) {
    return 0;
}

bool Item::getHasSubtypes() {
    return hasSubtypes;
}

Item* Item::setHasSubtypes(bool flag) {
    hasSubtypes = flag;
    return this;
}

int Item::getMaxDamage() {
    return maxDamage;
}

Item* Item::setMaxDamage(int i) {
    maxDamage = i;
    return this;
}

bool Item::isDamagable() {
    return maxDamage > 0 && !hasSubtypes;
}

bool Item::hitEntity(ItemStack* itemstack, EntityLiving* entityliving, EntityLiving* entityliving1) {
    return false;
}

bool Item::onBlockDestroyed(ItemStack* itemstack, int i, int j, int k, int l, EntityLiving* entityliving) {
    return false;
}

int Item::getDamageVsEntity(Entity* entity) {
    return 1;
}

bool Item::canHarvestBlock(Block* block) {
    return false;
}

void Item::saddleEntity(ItemStack* itemstack, EntityLiving* entityliving) {
}

Item* Item::setFull3D() {
    bFull3D = true;
    return this;
}

bool Item::isFull3D() {
    return bFull3D;
}

bool Item::shouldRotateAroundWhenRendering() {
    return false;
}

Item* Item::setItemName(const std::string& s) {
    itemName = "item." + s;
    return this;
}

std::string Item::getItemName() {
    return itemName;
}

std::string Item::getItemNameIS(ItemStack* itemstack) {
    return itemName;
}

Item* Item::setContainerItem(Item* item) {
    if (maxStackSize > 1) {
        throw std::invalid_argument("Max stack size must be 1 for items with crafting results");
    }
    containerItem = item;
    return this;
}

Item* Item::getContainerItem() {
    return containerItem;
}

bool Item::hasContainerItem() {
    return containerItem != nullptr;
}

std::string Item::getStatName() {
    return StatCollector::translateToLocal(getItemName() + ".name");
}

int Item::getColorFromDamage(int i) {
    return 0xffffff;
}

void Item::onUpdate(ItemStack* itemstack, World* world, Entity* entity, int i, bool flag) {
}

void Item::onCreated(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) {
}

void Item::initItems() {
    shovelSteel = (new ItemSpade(0, EnumToolMaterial::IRON))->setIconCoord(2, 5)->setItemName("shovelIron");
    pickaxeSteel = (new ItemPickaxe(1, EnumToolMaterial::IRON))->setIconCoord(2, 6)->setItemName("pickaxeIron");
    axeSteel = (new ItemAxe(2, EnumToolMaterial::IRON))->setIconCoord(2, 7)->setItemName("hatchetIron");
    flintAndSteel = (new ItemFlintAndSteel(3))->setIconCoord(5, 0)->setItemName("flintAndSteel");
    appleRed = (new ItemFood(4, 4, false))->setIconCoord(10, 0)->setItemName("apple");
    bow = (new ItemBow(5))->setIconCoord(5, 1)->setItemName("bow");
    arrow = (new Item(6))->setIconCoord(5, 2)->setItemName("arrow");
    coal = (new ItemCoal(7))->setIconCoord(7, 0)->setItemName("coal");
    diamond = (new Item(8))->setIconCoord(7, 3)->setItemName("emerald");
    ingotIron = (new Item(9))->setIconCoord(7, 1)->setItemName("ingotIron");
    ingotGold = (new Item(10))->setIconCoord(7, 2)->setItemName("ingotGold");
    swordSteel = (new ItemSword(11, EnumToolMaterial::IRON))->setIconCoord(2, 4)->setItemName("swordIron");
    swordWood = (new ItemSword(12, EnumToolMaterial::WOOD))->setIconCoord(0, 4)->setItemName("swordWood");
    shovelWood = (new ItemSpade(13, EnumToolMaterial::WOOD))->setIconCoord(0, 5)->setItemName("shovelWood");
    pickaxeWood = (new ItemPickaxe(14, EnumToolMaterial::WOOD))->setIconCoord(0, 6)->setItemName("pickaxeWood");
    axeWood = (new ItemAxe(15, EnumToolMaterial::WOOD))->setIconCoord(0, 7)->setItemName("hatchetWood");
    swordStone = (new ItemSword(16, EnumToolMaterial::STONE))->setIconCoord(1, 4)->setItemName("swordStone");
    shovelStone = (new ItemSpade(17, EnumToolMaterial::STONE))->setIconCoord(1, 5)->setItemName("shovelStone");
    pickaxeStone = (new ItemPickaxe(18, EnumToolMaterial::STONE))->setIconCoord(1, 6)->setItemName("pickaxeStone");
    axeStone = (new ItemAxe(19, EnumToolMaterial::STONE))->setIconCoord(1, 7)->setItemName("hatchetStone");
    swordDiamond = (new ItemSword(20, EnumToolMaterial::EMERALD))->setIconCoord(3, 4)->setItemName("swordDiamond");
    shovelDiamond = (new ItemSpade(21, EnumToolMaterial::EMERALD))->setIconCoord(3, 5)->setItemName("shovelDiamond");
    pickaxeDiamond = (new ItemPickaxe(22, EnumToolMaterial::EMERALD))->setIconCoord(3, 6)->setItemName("pickaxeDiamond");
    axeDiamond = (new ItemAxe(23, EnumToolMaterial::EMERALD))->setIconCoord(3, 7)->setItemName("hatchetDiamond");
    stick = (new Item(24))->setIconCoord(5, 3)->setFull3D()->setItemName("stick");
    bowlEmpty = (new Item(25))->setIconCoord(7, 4)->setItemName("bowl");
    bowlSoup = (new ItemSoup(26, 10))->setIconCoord(8, 4)->setItemName("mushroomStew");
    swordGold = (new ItemSword(27, EnumToolMaterial::GOLD))->setIconCoord(4, 4)->setItemName("swordGold");
    shovelGold = (new ItemSpade(28, EnumToolMaterial::GOLD))->setIconCoord(4, 5)->setItemName("shovelGold");
    pickaxeGold = (new ItemPickaxe(29, EnumToolMaterial::GOLD))->setIconCoord(4, 6)->setItemName("pickaxeGold");
    axeGold = (new ItemAxe(30, EnumToolMaterial::GOLD))->setIconCoord(4, 7)->setItemName("hatchetGold");
    silk = (new Item(31))->setIconCoord(8, 0)->setItemName("string");
    feather = (new Item(32))->setIconCoord(8, 1)->setItemName("feather");
    gunpowder = (new Item(33))->setIconCoord(8, 2)->setItemName("sulphur");
    hoeWood = (new ItemHoe(34, EnumToolMaterial::WOOD))->setIconCoord(0, 8)->setItemName("hoeWood");
    hoeStone = (new ItemHoe(35, EnumToolMaterial::STONE))->setIconCoord(1, 8)->setItemName("hoeStone");
    hoeSteel = (new ItemHoe(36, EnumToolMaterial::IRON))->setIconCoord(2, 8)->setItemName("hoeIron");
    hoeDiamond = (new ItemHoe(37, EnumToolMaterial::EMERALD))->setIconCoord(3, 8)->setItemName("hoeDiamond");
    hoeGold = (new ItemHoe(38, EnumToolMaterial::GOLD))->setIconCoord(4, 8)->setItemName("hoeGold");
    seeds = (new ItemSeeds(39, Block::crops->blockID))->setIconCoord(9, 0)->setItemName("seeds");
    wheat = (new Item(40))->setIconCoord(9, 1)->setItemName("wheat");
    bread = (new ItemFood(41, 5, false))->setIconCoord(9, 2)->setItemName("bread");
    helmetLeather = (new ItemArmor(42, 0, 0, 0))->setIconCoord(0, 0)->setItemName("helmetCloth");
    plateLeather = (new ItemArmor(43, 0, 0, 1))->setIconCoord(0, 1)->setItemName("chestplateCloth");
    legsLeather = (new ItemArmor(44, 0, 0, 2))->setIconCoord(0, 2)->setItemName("leggingsCloth");
    bootsLeather = (new ItemArmor(45, 0, 0, 3))->setIconCoord(0, 3)->setItemName("bootsCloth");
    helmetChain = (new ItemArmor(46, 1, 1, 0))->setIconCoord(1, 0)->setItemName("helmetChain");
    plateChain = (new ItemArmor(47, 1, 1, 1))->setIconCoord(1, 1)->setItemName("chestplateChain");
    legsChain = (new ItemArmor(48, 1, 1, 2))->setIconCoord(1, 2)->setItemName("leggingsChain");
    bootsChain = (new ItemArmor(49, 1, 1, 3))->setIconCoord(1, 3)->setItemName("bootsChain");
    helmetSteel = (new ItemArmor(50, 2, 2, 0))->setIconCoord(2, 0)->setItemName("helmetIron");
    plateSteel = (new ItemArmor(51, 2, 2, 1))->setIconCoord(2, 1)->setItemName("chestplateIron");
    legsSteel = (new ItemArmor(52, 2, 2, 2))->setIconCoord(2, 2)->setItemName("leggingsIron");
    bootsSteel = (new ItemArmor(53, 2, 2, 3))->setIconCoord(2, 3)->setItemName("bootsIron");
    helmetDiamond = (new ItemArmor(54, 3, 3, 0))->setIconCoord(3, 0)->setItemName("helmetDiamond");
    plateDiamond = (new ItemArmor(55, 3, 3, 1))->setIconCoord(3, 1)->setItemName("chestplateDiamond");
    legsDiamond = (new ItemArmor(56, 3, 3, 2))->setIconCoord(3, 2)->setItemName("leggingsDiamond");
    bootsDiamond = (new ItemArmor(57, 3, 3, 3))->setIconCoord(3, 3)->setItemName("bootsDiamond");
    helmetGold = (new ItemArmor(58, 1, 4, 0))->setIconCoord(4, 0)->setItemName("helmetGold");
    plateGold = (new ItemArmor(59, 1, 4, 1))->setIconCoord(4, 1)->setItemName("chestplateGold");
    legsGold = (new ItemArmor(60, 1, 4, 2))->setIconCoord(4, 2)->setItemName("leggingsGold");
    bootsGold = (new ItemArmor(61, 1, 4, 3))->setIconCoord(4, 3)->setItemName("bootsGold");
    flint = (new Item(62))->setIconCoord(6, 0)->setItemName("flint");
    porkRaw = (new ItemFood(63, 3, true))->setIconCoord(7, 5)->setItemName("porkchopRaw");
    porkCooked = (new ItemFood(64, 8, true))->setIconCoord(8, 5)->setItemName("porkchopCooked");
    painting = (new ItemPainting(65))->setIconCoord(10, 1)->setItemName("painting");
    appleGold = (new ItemFood(66, 42, false))->setIconCoord(11, 0)->setItemName("appleGold");
    sign = (new ItemSign(67))->setIconCoord(10, 2)->setItemName("sign");
    doorWood = (new ItemDoor(68, Material::wood))->setIconCoord(11, 2)->setItemName("doorWood");
    bucketEmpty = (new ItemBucket(69, 0))->setIconCoord(10, 4)->setItemName("bucket");
    bucketWater = (new ItemBucket(70, Block::waterMoving->blockID))->setIconCoord(11, 4)->setItemName("bucketWater");
    bucketLava = (new ItemBucket(71, Block::lavaMoving->blockID))->setIconCoord(12, 4)->setItemName("bucketLava");
    minecartEmpty = (new ItemMinecart(72, 0))->setIconCoord(7, 8)->setItemName("minecart");
    saddle = (new ItemSaddle(73))->setIconCoord(8, 6)->setItemName("saddle");
    doorSteel = (new ItemDoor(74, Material::iron))->setIconCoord(12, 2)->setItemName("doorIron");
    redstone = (new ItemRedstone(75))->setIconCoord(8, 3)->setItemName("redstone");
    snowball = (new ItemSnowball(76))->setIconCoord(14, 0)->setItemName("snowball");
    boat = (new ItemBoat(77))->setIconCoord(8, 8)->setItemName("boat");
    leather = (new Item(78))->setIconCoord(7, 6)->setItemName("leather");
    bucketMilk = (new ItemBucket(79, -1))->setIconCoord(13, 4)->setItemName("milk");
    brick = (new Item(80))->setIconCoord(6, 1)->setItemName("brick");
    clay = (new Item(81))->setIconCoord(9, 3)->setItemName("clay");
    reed = (new ItemReed(82, Block::reed))->setIconCoord(11, 1)->setItemName("reeds");
    paper = (new Item(83))->setIconCoord(10, 3)->setItemName("paper");
    book = (new Item(84))->setIconCoord(11, 3)->setItemName("book");
    slimeBall = (new Item(85))->setIconCoord(14, 1)->setItemName("slimeball");
    minecartCrate = (new ItemMinecart(86, 1))->setIconCoord(7, 9)->setItemName("minecartChest");
    minecartPowered = (new ItemMinecart(87, 2))->setIconCoord(7, 10)->setItemName("minecartFurnace");
    egg = (new ItemEgg(88))->setIconCoord(12, 0)->setItemName("egg");
    compass = (new Item(89))->setIconCoord(6, 3)->setItemName("compass");
    fishingRod = (new ItemFishingRod(90))->setIconCoord(5, 4)->setItemName("fishingRod");
    pocketSundial = (new Item(91))->setIconCoord(6, 4)->setItemName("clock");
    lightStoneDust = (new Item(92))->setIconCoord(9, 4)->setItemName("yellowDust");
    fishRaw = (new ItemFood(93, 2, false))->setIconCoord(9, 5)->setItemName("fishRaw");
    fishCooked = (new ItemFood(94, 5, false))->setIconCoord(10, 5)->setItemName("fishCooked");
    dyePowder = (new ItemDye(95))->setIconCoord(14, 4)->setItemName("dyePowder");
    bone = (new Item(96))->setIconCoord(12, 1)->setItemName("bone")->setFull3D();
    sugar = (new Item(97))->setIconCoord(13, 0)->setItemName("sugar")->setFull3D();
    cake = (new ItemReed(98, Block::cake))->setMaxStackSize(1)->setIconCoord(13, 1)->setItemName("cake");
    bed = (new ItemBed(99))->setMaxStackSize(1)->setIconCoord(13, 2)->setItemName("bed");
    redstoneRepeater = (new ItemReed(100, Block::redstoneRepeaterIdle))->setIconCoord(6, 5)->setItemName("diode");
    cookie = (new ItemCookie(101, 1, false, 8))->setIconCoord(12, 5)->setItemName("cookie");
    mapItem = (ItemMap*)(new ItemMap(102))->setIconCoord(12, 3)->setItemName("map");
    shears = (ItemShears*)(new ItemShears(103))->setIconCoord(13, 5)->setItemName("shears");
    record13 = (new ItemRecord(2000, "13"))->setIconCoord(0, 15)->setItemName("record");
    recordCat = (new ItemRecord(2001, "cat"))->setIconCoord(1, 15)->setItemName("record");
    StatList::initItemStats(); // func_25151_b
}
