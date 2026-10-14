#include "Material.h"

#include "MapColor.h"
#include "MaterialTransparent.h"
#include "MaterialLiquid.h"
#include "MaterialLogic.h"
#include "MaterialPortal.h"

Material::Material(MapColor *mapcolor) :
	materialMapColor(mapcolor)
{
	canHarvest = true;
}

bool Material::getIsLiquid()
{
	return false;
}

bool Material::isSolid()
{
	return true;
}

bool Material::getCanBlockGrass()
{
	return true;
}

bool Material::getIsSolid()
{
	return true;
}

Material *Material::setIsTranslucent()
{
	isOpaque = true;
	return this;
}

Material *Material::setNoHarvest()
{
	canHarvest = false;
	return this;
}

Material *Material::setBurning()
{
	canBurn = true;
	return this;
}

bool Material::getBurning()
{
	return canBurn;
}

Material *Material::setIsGroundCover()
{
	groundCover = true;
	return this;
}

bool Material::getIsGroundCover()
{
	return groundCover;
}

bool Material::getIsTranslucent()
{
	if (isOpaque)
		return false;
	else
		return getIsSolid();
}

bool Material::getIsHarvestable()
{
	return canHarvest;
}

int_t Material::getMaterialMobility()
{
	return mobilityFlag;
}

Material *Material::setNoPushMobility()
{
	mobilityFlag = 1;
	return this;
}

Material *Material::setImmovableMobility()
{
	mobilityFlag = 2;
	return this;
}

Material *Material::air = nullptr;
Material *Material::grassMaterial = nullptr;
Material *Material::ground = nullptr;
Material *Material::wood = nullptr;
Material *Material::rock = nullptr;
Material *Material::iron = nullptr;
Material *Material::water = nullptr;
Material *Material::lava = nullptr;
Material *Material::leaves = nullptr;
Material *Material::plants = nullptr;
Material *Material::sponge = nullptr;
Material *Material::cloth = nullptr;
Material *Material::fire = nullptr;
Material *Material::sand = nullptr;
Material *Material::circuits = nullptr;
Material *Material::glass = nullptr;
Material *Material::tnt = nullptr;
Material *Material::spawnableCreatureList = nullptr;
Material *Material::ice = nullptr;
Material *Material::snow = nullptr;
Material *Material::builtSnow = nullptr;
Material *Material::cactus = nullptr;
Material *Material::clay = nullptr;
Material *Material::pumpkin = nullptr;
Material *Material::portal = nullptr;
Material *Material::cakeMaterial = nullptr;
Material *Material::pistonBase = nullptr;
Material *Material::pistonSticky = nullptr;

void Material::initialize()
{
	air = new MaterialTransparent(MapColor::airColor);
	grassMaterial = new Material(MapColor::grassColor);
	ground = new Material(MapColor::dirtColor);
	wood = (new Material(MapColor::woodColor))->setBurning();
	rock = (new Material(MapColor::stoneColor))->setNoHarvest();
	iron = (new Material(MapColor::ironColor))->setNoHarvest();
	water = (new MaterialLiquid(MapColor::waterColor))->setNoPushMobility();
	lava = (new MaterialLiquid(MapColor::tntColor))->setNoPushMobility();
	leaves = (new Material(MapColor::foliageColor))->setBurning()->setIsTranslucent()->setNoPushMobility();
	plants = (new MaterialLogic(MapColor::foliageColor))->setNoPushMobility();
	sponge = new Material(MapColor::clothColor);
	cloth = (new Material(MapColor::clothColor))->setBurning();
	fire = (new MaterialTransparent(MapColor::airColor))->setNoPushMobility();
	sand = new Material(MapColor::sandColor);
	circuits = (new MaterialLogic(MapColor::airColor))->setNoPushMobility();
	glass = (new Material(MapColor::airColor))->setIsTranslucent();
	tnt = (new Material(MapColor::tntColor))->setBurning()->setIsTranslucent();
	spawnableCreatureList = (new Material(MapColor::foliageColor))->setNoPushMobility();
	ice = (new Material(MapColor::iceColor))->setIsTranslucent();
	snow = (new MaterialLogic(MapColor::snowColor))->setIsGroundCover()->setIsTranslucent()->setNoHarvest()->setNoPushMobility();
	builtSnow = (new Material(MapColor::snowColor))->setNoHarvest();
	cactus = (new Material(MapColor::foliageColor))->setIsTranslucent()->setNoPushMobility();
	clay = new Material(MapColor::clayColor);
	pumpkin = (new Material(MapColor::foliageColor))->setNoPushMobility();
	portal = (new MaterialPortal(MapColor::airColor))->setImmovableMobility();
	cakeMaterial = (new Material(MapColor::airColor))->setNoPushMobility();
	pistonBase = (new Material(MapColor::clothColor))->setNoHarvest()->setNoPushMobility();
	pistonSticky = (new Material(MapColor::stoneColor))->setImmovableMobility();
}
