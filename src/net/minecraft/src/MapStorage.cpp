#include "MapStorage.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include "MapDataBase.h"
#include "ISaveHandler.h"
#include "NBTTagCompound.h"
#include "NBTBase.h"
#include "CompressedStreamTools.h"
#include "MapData.h"
#include "NBTTagShort.h"
#include "java/String.h"

MapStorage::MapStorage(ISaveHandler *isavehandler) :
	saveHandler(isavehandler)
{
	loadIdCounts();
}

MapStorage::~MapStorage()
{
	for (MapDataBase *data : loadedDataList)
		delete data;
}

MapDataBase* MapStorage::loadData(const std::type_info &classType, const std::string &s)
{
	auto it = loadedDataMap.find(s);
	if (it != loadedDataMap.end())
		return it->second;

	MapDataBase *mapdatabase = nullptr;
	if (saveHandler != nullptr)
	{
		try
		{
			std::string file = saveHandler->getMapFile(s);
			if (!file.empty() && classType == typeid(MapData))
			{
				std::ifstream input(file, std::ios::binary);
				if (input.good())
				{
					NBTTagCompound *root = CompressedStreamTools::readGzippedCompound(input);
					mapdatabase = new MapData(s);
					mapdatabase->readFromNBT(root->getCompoundTag(u"data"));
				}
			}
		}
		catch (...)
		{
		}
	}
	if (mapdatabase != nullptr)
	{
		loadedDataMap[s] = mapdatabase;
		loadedDataList.push_back(mapdatabase);
	}
	return mapdatabase;
}

void MapStorage::setData(const std::string &s, MapDataBase *mapdatabase)
{
	if (mapdatabase == nullptr)
		throw std::runtime_error("Can't set null data");

	auto it = loadedDataMap.find(s);
	if (it != loadedDataMap.end())
	{
		auto lit = std::find(loadedDataList.begin(), loadedDataList.end(), it->second);
		if (lit != loadedDataList.end()) loadedDataList.erase(lit);
		loadedDataMap.erase(it);
	}
	loadedDataMap[s] = mapdatabase;
	loadedDataList.push_back(mapdatabase);
}

void MapStorage::saveAllData()
{
	for (auto *mapdatabase : loadedDataList)
	{
		if (mapdatabase->isDirty())
		{
			saveData(mapdatabase);
			mapdatabase->setDirty(false);
		}
	}
}

void MapStorage::saveData(MapDataBase *mapdatabase)
{
	if (saveHandler == nullptr) return;
	try
	{
		std::string file = saveHandler->getMapFile(mapdatabase->mapName);
		if (!file.empty())
		{
			NBTTagCompound *data = new NBTTagCompound();
			mapdatabase->writeToNBT(data);
			NBTTagCompound *root = new NBTTagCompound();
			root->setCompoundTag(u"data", data);
			std::ofstream output(file, std::ios::binary | std::ios::trunc);
			CompressedStreamTools::writeGzippedCompoundToOutputStream(root, output);
		}
	}
	catch (...)
	{
	}
}

void MapStorage::loadIdCounts()
{
	try
	{
		idCounts.clear();
		if (saveHandler == nullptr) return;
		std::string file = saveHandler->getMapFile("idcounts");
		std::ifstream input(file, std::ios::binary);
		if (input.good())
		{
			NBTTagCompound *counts = CompressedStreamTools::readCompound(input);
			for (NBTBase *tag : counts->getTags())
			{
				if (auto *shortTag = dynamic_cast<NBTTagShort *>(tag))
					idCounts[String::toUTF8(shortTag->getKey())] = shortTag->shortValue;
			}
		}
	}
	catch (...)
	{
	}
}

int_t MapStorage::getUniqueDataId(const std::string &s)
{
	auto it = idCounts.find(s);
	short_t short1 = 0;
	if (it == idCounts.end())
		short1 = 0;
	else
		short1 = it->second + 1;

	idCounts[s] = short1;

	if (saveHandler == nullptr)
		return short1;

	try
	{
		std::string file = saveHandler->getMapFile("idcounts");
		if (!file.empty())
		{
			NBTTagCompound *counts = new NBTTagCompound();
			for (const auto &entry : idCounts)
				counts->setShort(entry.first, entry.second);
			std::ofstream output(file, std::ios::binary | std::ios::trunc);
			CompressedStreamTools::writeCompound(counts, output);
		}
	}
	catch (...)
	{
	}
	return short1;
}

MapDataBase* MapStorage::loadDataFromFile(const std::string &file, const std::string &file1, const std::string &file2)
{
	std::ifstream input(file, std::ios::binary);
	if (!input.good()) return nullptr;
	NBTTagCompound *root = CompressedStreamTools::readGzippedCompound(input);
	MapData *data = new MapData(file1);
	data->readFromNBT(root->getCompoundTag(file2));
	return data;
}
