#include "ChunkLoader.h"

#include <cstdio>
#include <sstream>
#include <algorithm>

#include "World.h"
#include "WorldInfo.h"
#include "Chunk.h"
#include "NibbleArray.h"
#include "NBTTagCompound.h"
#include "NBTTagList.h"
#include "CompressedStreamTools.h"
#include "Entity.h"
#include "EntityList.h"
#include "TileEntity.h"
#include "java/String.h"

// ---- base-36 helpers (Java Integer.toString(n, 36)) ----

static jstring intToBase36(int_t n)
{
	const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	bool negative = (n < 0);
	if (negative) n = -n;
	jstring result;
	do
	{
		result = jstring(1, (char16_t)digits[n % 36]) + result;
		n /= 36;
	} while (n > 0);
	if (negative) result = u"-" + result;
	return result;
}

// --------------------------------------------------------

ChunkLoader::ChunkLoader(File *file, bool flag)
	: saveDir(file)
	, createIfNecessary(flag)
{
}

File *ChunkLoader::chunkFileForXZ(int_t i, int_t j)
{
	jstring s = u"c." + intToBase36(i) + u"." + intToBase36(j) + u".dat";
	jstring s1 = intToBase36(i & 0x3f);
	jstring s2 = intToBase36(j & 0x3f);

	File *dir1 = File::open(*saveDir, s1);
	if (!dir1->exists())
	{
		if (createIfNecessary)
			dir1->mkdir();
		else
			return nullptr;
	}
	File *dir2 = File::open(*dir1, s2);
	if (!dir2->exists())
	{
		if (createIfNecessary)
			dir2->mkdir();
		else
			return nullptr;
	}
	File *file = File::open(*dir2, s);
	if (!file->exists() && !createIfNecessary)
		return nullptr;
	return file;
}

Chunk *ChunkLoader::loadChunk(World *world, int_t i, int_t j)
{
	File *file = chunkFileForXZ(i, j);
	if (file != nullptr && file->exists())
	{
		try
		{
			std::istream *is = file->toStreamIn();
			NBTTagCompound *nbttagcompound = CompressedStreamTools::readCompressed(*is);
			delete is;

			if (!nbttagcompound->hasKey(u"Level"))
			{
				printf("Chunk file at %d,%d is missing level data, skipping\n", i, j);
				return nullptr;
			}
			if (!nbttagcompound->getCompoundTag(u"Level")->hasKey(u"Blocks"))
			{
				printf("Chunk file at %d,%d is missing block data, skipping\n", i, j);
				return nullptr;
			}
			Chunk *chunk = loadChunkIntoWorldFromCompound(world, nbttagcompound->getCompoundTag(u"Level"));
			if (!chunk->isAtLocation(i, j))
			{
				printf("Chunk file at %d,%d is in the wrong location; relocating. (Expected %d, %d, got %d, %d)\n",
					i, j, i, j, chunk->xPosition, chunk->zPosition);
				nbttagcompound->setInteger(u"xPos", i);
				nbttagcompound->setInteger(u"zPos", j);
				chunk = loadChunkIntoWorldFromCompound(world, nbttagcompound->getCompoundTag(u"Level"));
			}
			chunk->remapBlocks();
			return chunk;
		}
		catch (...)
		{
			printf("ChunkLoader::loadChunk – exception reading chunk %d,%d\n", i, j);
		}
	}
	return nullptr;
}

void ChunkLoader::saveChunk(World *world, Chunk *chunk)
{
	world->checkSessionLock();
	File *file = chunkFileForXZ(chunk->xPosition, chunk->zPosition);
	if (file->exists())
	{
		WorldInfo *worldinfo = world->getWorldInfo();
		worldinfo->setSizeOnDisk(worldinfo->getSizeOnDisk() - file->length());
	}
	try
	{
		File *tmpFile = File::open(*saveDir, u"tmp_chunk.dat");
		std::ostream *os = tmpFile->toStreamOut();
		NBTTagCompound *nbttagcompound  = new NBTTagCompound();
		NBTTagCompound *nbttagcompound1 = new NBTTagCompound();
		nbttagcompound->setTag(u"Level", nbttagcompound1);
		storeChunkInCompound(chunk, world, nbttagcompound1);
		CompressedStreamTools::writeGzippedCompoundToOutputStream(nbttagcompound, *os);
		os->flush();
		delete os;
		if (file->exists())
			file->remove();
		tmpFile->renameTo(*file);
		WorldInfo *worldinfo1 = world->getWorldInfo();
		worldinfo1->setSizeOnDisk(worldinfo1->getSizeOnDisk() + file->length());
	}
	catch (...)
	{
		printf("ChunkLoader::saveChunk – exception saving chunk %d,%d\n",
			chunk->xPosition, chunk->zPosition);
	}
}

void ChunkLoader::storeChunkInCompound(Chunk *chunk, World *world, NBTTagCompound *nbttagcompound)
{
	world->checkSessionLock();
	nbttagcompound->setInteger(u"xPos", chunk->xPosition);
	nbttagcompound->setInteger(u"zPos", chunk->zPosition);
	nbttagcompound->setLong(u"LastUpdate", world->getWorldTime());
	nbttagcompound->setByteArray(u"Blocks", chunk->blocks);
	nbttagcompound->setByteArray(u"Data", chunk->data.data);
	nbttagcompound->setByteArray(u"SkyLight", chunk->skylightMap.data);
	nbttagcompound->setByteArray(u"BlockLight", chunk->blocklightMap.data);
	nbttagcompound->setByteArray(u"HeightMap",
		std::vector<byte_t>(chunk->heightMap, chunk->heightMap + 256));
	nbttagcompound->setBoolean(u"TerrainPopulated", chunk->isTerrainPopulated);

	chunk->hasEntities = false;
	NBTTagList *nbttaglist = new NBTTagList();
	for (int_t i = 0; i < 8; i++)
	{
		for (Entity *entity : chunk->entities[i])
		{
			chunk->hasEntities = true;
			NBTTagCompound *nbttagcompound1 = new NBTTagCompound();
			if (entity->addEntityID(nbttagcompound1))
				nbttaglist->setTag(nbttagcompound1);
		}
	}
	nbttagcompound->setTag(u"Entities", nbttaglist);

	NBTTagList *nbttaglist1 = new NBTTagList();
	for (auto &kv : chunk->chunkTileEntityMap)
	{
		TileEntity *tileentity = kv.second;
		NBTTagCompound *nbttagcompound2 = new NBTTagCompound();
		tileentity->writeToNBT(nbttagcompound2);
		nbttaglist1->setTag(nbttagcompound2);
	}
	nbttagcompound->setTag(u"TileEntities", nbttaglist1);
}

Chunk *ChunkLoader::loadChunkIntoWorldFromCompound(World *world, NBTTagCompound *nbttagcompound)
{
	int_t i = nbttagcompound->getInteger(u"xPos");
	int_t j = nbttagcompound->getInteger(u"zPos");
	Chunk *chunk = new Chunk(world, i, j);

	chunk->blocks = nbttagcompound->getByteArray(u"Blocks");
	if ((int_t)chunk->blocks.size() != 16 * 16 * 128)
	{
		fprintf(stderr, "Beta++: corrupt chunk (%d,%d) reset to air\n", i, j);
		chunk->blocks.assign(16 * 16 * 128, 0);
	}
	chunk->data   = NibbleArray(nbttagcompound->getByteArray(u"Data"));
	chunk->skylightMap   = NibbleArray(nbttagcompound->getByteArray(u"SkyLight"));
	chunk->blocklightMap = NibbleArray(nbttagcompound->getByteArray(u"BlockLight"));

	std::vector<byte_t> hm = nbttagcompound->getByteArray(u"HeightMap");
	if (!hm.empty())
		std::copy(hm.begin(), hm.end(), chunk->heightMap);

	chunk->isTerrainPopulated = nbttagcompound->getBoolean(u"TerrainPopulated");
	chunk->hasRealData = true;

	if (!chunk->data.isValid())
		chunk->data = NibbleArray((int_t)chunk->blocks.size());

	if (hm.empty() || !chunk->skylightMap.isValid())
	{
		std::fill(chunk->heightMap, chunk->heightMap + 256, (byte_t)0);
		chunk->skylightMap = NibbleArray((int_t)chunk->blocks.size());
		chunk->generateSkylightMap();
	}

	if (!chunk->blocklightMap.isValid())
	{
		chunk->blocklightMap = NibbleArray((int_t)chunk->blocks.size());
		chunk->initBlockLight();
	}

	NBTTagList *nbttaglist = nbttagcompound->getTagList(u"Entities");
	if (nbttaglist != nullptr)
	{
		for (int_t k = 0; k < nbttaglist->tagCount(); k++)
		{
			NBTTagCompound *nbttagcompound1 = static_cast<NBTTagCompound *>(nbttaglist->tagAt(k));
			Entity *entity = EntityList::createEntityFromNBT(nbttagcompound1, world);
			chunk->hasEntities = true;
			if (entity != nullptr)
				chunk->addEntity(entity);
		}
	}

	NBTTagList *nbttaglist1 = nbttagcompound->getTagList(u"TileEntities");
	if (nbttaglist1 != nullptr)
	{
		for (int_t l = 0; l < nbttaglist1->tagCount(); l++)
		{
			NBTTagCompound *nbttagcompound2 = static_cast<NBTTagCompound *>(nbttaglist1->tagAt(l));
			TileEntity *tileentity = TileEntity::createAndLoadEntity(nbttagcompound2);
			if (tileentity != nullptr)
				chunk->addTileEntity(tileentity);
		}
	}

	return chunk;
}

void ChunkLoader::addRandomArmor()
{
}

void ChunkLoader::saveExtraData()
{
}

void ChunkLoader::saveExtraChunkData(World *world, Chunk *chunk)
{
}
