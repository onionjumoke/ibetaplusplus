#include "WorldInfo.h"

#include "EntityPlayer.h"
#include "NBTTagCompound.h"
#include "java/System.h"

WorldInfo::WorldInfo(NBTTagCompound *nbttagcompound)
{
	randomSeed = nbttagcompound->getLong(u"RandomSeed");
	generatorType = nbttagcompound->hasKey(u"GeneratorType") ? nbttagcompound->getInteger(u"GeneratorType") : 0;
	spawnX = nbttagcompound->getInteger(u"SpawnX");
	spawnY = nbttagcompound->getInteger(u"SpawnY");
	spawnZ = nbttagcompound->getInteger(u"SpawnZ");
	worldTime = nbttagcompound->getLong(u"Time");
	lastTimePlayed = nbttagcompound->getLong(u"LastPlayed");
	sizeOnDisk = nbttagcompound->getLong(u"SizeOnDisk");
	levelName = nbttagcompound->getString(u"LevelName");
	saveVersion = nbttagcompound->getInteger(u"version");
	rainTime = nbttagcompound->getInteger(u"rainTime");
	raining = nbttagcompound->getBoolean(u"raining");
	thunderTime = nbttagcompound->getInteger(u"thunderTime");
	thundering = nbttagcompound->getBoolean(u"thundering");
	playerTag = nullptr;
	dimension = 0;
	if (nbttagcompound->hasKey(u"Player"))
	{
		playerTag = nbttagcompound->getCompoundTag(u"Player");
		dimension = playerTag->getInteger(u"Dimension");
	}
}

WorldInfo::WorldInfo(long_t l, const jstring &s)
{
	randomSeed = l;
	levelName = s;
	spawnX = 0;
	spawnY = 0;
	spawnZ = 0;
	worldTime = 0;
	lastTimePlayed = 0;
	sizeOnDisk = 0;
	playerTag = nullptr;
	dimension = 0;
	// Java Beta 1.7.3 writes 19132 for the McRegion save format. Leaving this
	// at 0 made SaveConverterMcRegion::isOldMapFormat() report a brand-new
	// region world as an old-format map, so every reload ran a pointless full
	// conversion scan (and startWorld(..., 0L) after it).
	saveVersion = 19132;
	generatorType = 0;
	raining = false;
	rainTime = 0;
	thundering = false;
	thunderTime = 0;
}

WorldInfo::WorldInfo(WorldInfo *worldinfo)
{
	randomSeed = worldinfo->randomSeed;
	spawnX = worldinfo->spawnX;
	spawnY = worldinfo->spawnY;
	spawnZ = worldinfo->spawnZ;
	worldTime = worldinfo->worldTime;
	lastTimePlayed = worldinfo->lastTimePlayed;
	sizeOnDisk = worldinfo->sizeOnDisk;
	playerTag = worldinfo->playerTag;
	dimension = worldinfo->dimension;
	levelName = worldinfo->levelName;
	saveVersion = worldinfo->saveVersion;
	rainTime = worldinfo->rainTime;
	raining = worldinfo->raining;
	thunderTime = worldinfo->thunderTime;
	thundering = worldinfo->thundering;
}

NBTTagCompound *WorldInfo::getNBTTagCompound()
{
	NBTTagCompound *nbttagcompound = new NBTTagCompound();
	updateTagCompound(nbttagcompound, playerTag);
	return nbttagcompound;
}

NBTTagCompound *WorldInfo::getNBTTagCompoundWithPlayer(const std::vector<EntityPlayer *> &list)
{
	NBTTagCompound *nbttagcompound = new NBTTagCompound();
	EntityPlayer *entityplayer = nullptr;
	NBTTagCompound *nbttagcompound1 = nullptr;
	if (!list.empty())
		entityplayer = list[0];
	if (entityplayer != nullptr)
	{
		nbttagcompound1 = new NBTTagCompound();
		entityplayer->writeToNBT(nbttagcompound1);
	}
	updateTagCompound(nbttagcompound, nbttagcompound1);
	return nbttagcompound;
}

void WorldInfo::updateTagCompound(NBTTagCompound *nbttagcompound, NBTTagCompound *nbttagcompound1)
{
	nbttagcompound->setLong(u"RandomSeed", randomSeed);
	nbttagcompound->setInteger(u"GeneratorType", generatorType);
	nbttagcompound->setInteger(u"SpawnX", spawnX);
	nbttagcompound->setInteger(u"SpawnY", spawnY);
	nbttagcompound->setInteger(u"SpawnZ", spawnZ);
	nbttagcompound->setLong(u"Time", worldTime);
	nbttagcompound->setLong(u"SizeOnDisk", sizeOnDisk);
	nbttagcompound->setLong(u"LastPlayed", System::currentTimeMillis());
	nbttagcompound->setString(u"LevelName", levelName);
	nbttagcompound->setInteger(u"version", saveVersion);
	nbttagcompound->setInteger(u"rainTime", rainTime);
	nbttagcompound->setBoolean(u"raining", raining);
	nbttagcompound->setInteger(u"thunderTime", thunderTime);
	nbttagcompound->setBoolean(u"thundering", thundering);
	if (nbttagcompound1 != nullptr)
		nbttagcompound->setCompoundTag(u"Player", nbttagcompound1);
}

long_t WorldInfo::getRandomSeed() { return randomSeed; }
int_t WorldInfo::getSpawnX() { return spawnX; }
int_t WorldInfo::getSpawnY() { return spawnY; }
int_t WorldInfo::getSpawnZ() { return spawnZ; }
long_t WorldInfo::getWorldTime() { return worldTime; }
long_t WorldInfo::getSizeOnDisk() { return sizeOnDisk; }
NBTTagCompound *WorldInfo::getPlayerNBTTagCompound() { return playerTag; }
int_t WorldInfo::getDimension() { return dimension; }
void WorldInfo::setSpawnX(int_t i) { spawnX = i; }
void WorldInfo::setSpawnY(int_t i) { spawnY = i; }
void WorldInfo::setSpawnZ(int_t i) { spawnZ = i; }
void WorldInfo::setWorldTime(long_t l) { worldTime = l; }
void WorldInfo::setSizeOnDisk(long_t l) { sizeOnDisk = l; }
void WorldInfo::setPlayerNBTTagCompound(NBTTagCompound *nbttagcompound) { playerTag = nbttagcompound; }
void WorldInfo::setSpawn(int_t i, int_t j, int_t k) { spawnX = i; spawnY = j; spawnZ = k; }
jstring WorldInfo::getWorldName() { return levelName; }
void WorldInfo::setWorldName(const jstring &s) { levelName = s; }
int_t WorldInfo::getSaveVersion() { return saveVersion; }
void WorldInfo::setSaveVersion(int_t i) { saveVersion = i; }
long_t WorldInfo::getLastTimePlayed() { return lastTimePlayed; }
bool WorldInfo::getThundering() { return thundering; }
void WorldInfo::setThundering(bool flag) { thundering = flag; }
int_t WorldInfo::getThunderTime() { return thunderTime; }
void WorldInfo::setThunderTime(int_t i) { thunderTime = i; }
bool WorldInfo::getRaining() { return raining; }
void WorldInfo::setRaining(bool flag) { raining = flag; }
int_t WorldInfo::getRainTime() { return rainTime; }
void WorldInfo::setRainTime(int_t i) { rainTime = i; }
