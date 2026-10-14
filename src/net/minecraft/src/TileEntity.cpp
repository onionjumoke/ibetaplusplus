#include "TileEntity.h"

#include <iostream>
#include <stdexcept>
#include <typeinfo>

#include "Block.h"
#include "NBTTagCompound.h"
#include "World.h"

#include "TileEntityFurnace.h"
#include "TileEntityChest.h"
#include "TileEntityRecordPlayer.h"
#include "TileEntityDispenser.h"
#include "TileEntitySign.h"
#include "TileEntityMobSpawner.h"
#include "TileEntityNote.h"
#include "TileEntityPiston.h"

std::map<std::string, std::function<TileEntity *()>> TileEntity::nameToClassMap;
std::map<std::string, std::string>                   TileEntity::classToNameMap;

// Java: static { addMapping(...); } block at the bottom of TileEntity.
// Registered the first time any TileEntity machinery is touched.
namespace
{
	struct TileEntityMappingInit
	{
		TileEntityMappingInit()
		{
			TileEntity::addMapping<TileEntityFurnace>("Furnace");
			TileEntity::addMapping<TileEntityChest>("Chest");
			TileEntity::addMapping<TileEntityRecordPlayer>("RecordPlayer");
			TileEntity::addMapping<TileEntityDispenser>("Trap");
			TileEntity::addMapping<TileEntitySign>("Sign");
			TileEntity::addMapping<TileEntityMobSpawner>("MobSpawner");
			TileEntity::addMapping<TileEntityNote>("Music");
			TileEntity::addMapping<TileEntityPiston>("Piston");
		}
	};
	TileEntityMappingInit tileEntityMappingInit;
}

TileEntity::TileEntity()
	: worldObj(nullptr),
	  xCoord(0), yCoord(0), zCoord(0),
	  invalid(false)
{
}

void TileEntity::readFromNBT(NBTTagCompound *nbttagcompound)
{
	xCoord = nbttagcompound->getInteger(u"x");
	yCoord = nbttagcompound->getInteger(u"y");
	zCoord = nbttagcompound->getInteger(u"z");
}

void TileEntity::writeToNBT(NBTTagCompound *nbttagcompound)
{
	auto it = classToNameMap.find(typeid(*this).name());
	if (it == classToNameMap.end())
		throw std::runtime_error(std::string(typeid(*this).name()) + " is missing a mapping! This is a bug!");
	nbttagcompound->setString(u"id", String::fromUTF8(it->second));
	nbttagcompound->setInteger(u"x", xCoord);
	nbttagcompound->setInteger(u"y", yCoord);
	nbttagcompound->setInteger(u"z", zCoord);
}

void TileEntity::updateEntity()
{
}

TileEntity *TileEntity::createAndLoadEntity(NBTTagCompound *nbttagcompound)
{
	TileEntity *tileentity = nullptr;
	try
	{
		auto it = nameToClassMap.find(String::toUTF8(nbttagcompound->getString(u"id")));
		if (it != nameToClassMap.end())
			tileentity = it->second();
	}
	catch (std::exception &ex)
	{
		std::cerr << ex.what() << std::endl;
	}
	if (tileentity != nullptr)
		tileentity->readFromNBT(nbttagcompound);
	else
		std::cout << "Skipping TileEntity with id "
		          << String::toUTF8(nbttagcompound->getString(u"id")) << std::endl;
	return tileentity;
}

int_t TileEntity::getBlockMetadata()
{
	return worldObj->getBlockMetadata(xCoord, yCoord, zCoord);
}

void TileEntity::onInventoryChanged()
{
	if (worldObj != nullptr)
		worldObj->markTileEntityChunkModified(xCoord, yCoord, zCoord, this);
}

double TileEntity::getDistanceFrom(double d, double d1, double d2)
{
	double d3 = ((double)xCoord + 0.5) - d;
	double d4 = ((double)yCoord + 0.5) - d1;
	double d5 = ((double)zCoord + 0.5) - d2;
	return d3 * d3 + d4 * d4 + d5 * d5;
}

Block *TileEntity::getBlockType()
{
	return Block::blocksList[worldObj->getBlockId(xCoord, yCoord, zCoord)];
}
