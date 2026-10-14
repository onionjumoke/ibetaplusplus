#include "BiomeGenBase.h"
#include "Block.h"
#include "WorldGenerator.h"
#include "WorldGenBigTree.h"
#include "WorldGenTrees.h"
#include "BiomeGenRainforest.h"
#include "BiomeGenSwamp.h"
#include "BiomeGenForest.h"
#include "BiomeGenDesert.h"
#include "BiomeGenTaiga.h"
#include "BiomeGenHell.h"
#include "BiomeGenSky.h"
#include <cmath>
#include <algorithm>
#include <typeindex>
#include "BlockGrass.h"
#include "EntityChicken.h"
#include "EntityCow.h"
#include "EntityCreeper.h"
#include "EntityPig.h"
#include "EntitySheep.h"
#include "EntitySkeleton.h"
#include "EntitySlime.h"
#include "EntitySpider.h"
#include "EntitySquid.h"
#include "EntityZombie.h"

class EntitySpider; class EntityZombie; class EntitySkeleton;
class EntityCreeper; class EntitySlime; class EntitySheep;
class EntityPig; class EntityChicken; class EntityCow; class EntitySquid;

BiomeGenBase *BiomeGenBase::rainforest    = nullptr;
BiomeGenBase *BiomeGenBase::swampland     = nullptr;
BiomeGenBase *BiomeGenBase::seasonalForest = nullptr;
BiomeGenBase *BiomeGenBase::forest        = nullptr;
BiomeGenBase *BiomeGenBase::savanna       = nullptr;
BiomeGenBase *BiomeGenBase::shrubland     = nullptr;
BiomeGenBase *BiomeGenBase::taiga         = nullptr;
BiomeGenBase *BiomeGenBase::desert        = nullptr;
BiomeGenBase *BiomeGenBase::plains        = nullptr;
BiomeGenBase *BiomeGenBase::iceDesert     = nullptr;
BiomeGenBase *BiomeGenBase::tundra        = nullptr;
BiomeGenBase *BiomeGenBase::hell          = nullptr;
BiomeGenBase *BiomeGenBase::sky           = nullptr;
BiomeGenBase *BiomeGenBase::biomeLookupTable[4096] = {};

BiomeGenBase::BiomeGenBase()
    : topBlock((byte_t)Block::grass->blockID),
      fillerBlock((byte_t)Block::dirt->blockID),
      texturePackSelected(0x4ee031),
      color(0),
      enableSnow(false),
      enableRain(true)
{
    spawnableMonsterList.push_back(SpawnListEntry(std::type_index(typeid(EntitySpider)), 10));
    spawnableMonsterList.push_back(SpawnListEntry(std::type_index(typeid(EntityZombie)), 10));
    spawnableMonsterList.push_back(SpawnListEntry(std::type_index(typeid(EntitySkeleton)), 10));
    spawnableMonsterList.push_back(SpawnListEntry(std::type_index(typeid(EntityCreeper)), 10));
    spawnableMonsterList.push_back(SpawnListEntry(std::type_index(typeid(EntitySlime)), 10));
    spawnableCreatureList.push_back(SpawnListEntry(std::type_index(typeid(EntitySheep)), 12));
    spawnableCreatureList.push_back(SpawnListEntry(std::type_index(typeid(EntityPig)), 10));
    spawnableCreatureList.push_back(SpawnListEntry(std::type_index(typeid(EntityChicken)), 10));
    spawnableCreatureList.push_back(SpawnListEntry(std::type_index(typeid(EntityCow)), 8));
    spawnableWaterCreatureList.push_back(SpawnListEntry(std::type_index(typeid(EntitySquid)), 10));
}

BiomeGenBase *BiomeGenBase::setDisableRain()
{
    enableRain = false;
    return this;
}

BiomeGenBase *BiomeGenBase::setEnableSnow()
{
    enableSnow = true;
    return this;
}

BiomeGenBase *BiomeGenBase::setBiomeName(const jstring &s)
{
    biomeName = s;
    return this;
}

BiomeGenBase *BiomeGenBase::getBiomeAtCoords(int_t i)
{
    texturePackSelected = i;
    return this;
}

BiomeGenBase *BiomeGenBase::setColor(int_t i)
{
    color = i;
    return this;
}

static int_t hsbToRgb(float h, float s, float b)
{
    int_t r = 0, g = 0, bl = 0;
    if (s == 0.0f)
    {
        r = g = bl = (int_t)(b * 255.0f + 0.5f);
    }
    else
    {
        float hh = (h - (float)std::floor(h)) * 6.0f;
        int_t i = (int_t)hh;
        float f = hh - i;
        float p = b * (1.0f - s);
        float q = b * (1.0f - s * f);
        float t = b * (1.0f - s * (1.0f - f));
        float rv, gv, bv;
        switch (i) {
            case 0: rv=b; gv=t; bv=p; break;
            case 1: rv=q; gv=b; bv=p; break;
            case 2: rv=p; gv=b; bv=t; break;
            case 3: rv=p; gv=q; bv=b; break;
            case 4: rv=t; gv=p; bv=b; break;
            default:rv=b; gv=p; bv=q; break;
        }
        r  = (int_t)(rv * 255.0f + 0.5f);
        g  = (int_t)(gv * 255.0f + 0.5f);
        bl = (int_t)(bv * 255.0f + 0.5f);
    }
    return (int_t)0xff000000 | (r << 16) | (g << 8) | bl;
}

int_t BiomeGenBase::getSkyColorByTemp(float f)
{
    f /= 3.0f;
    if (f < -1.0f) f = -1.0f;
    if (f > 1.0f)  f = 1.0f;
    return hsbToRgb(0.6222222f - f * 0.05f, 0.5f + f * 0.1f, 1.0f);
}

WorldGenerator *BiomeGenBase::getRandomWorldGenForTrees(Random &random)
{
    if (random.nextInt(10) == 0)
        return new WorldGenBigTree();
    else
        return new WorldGenTrees();
}

std::vector<SpawnListEntry> *BiomeGenBase::getSpawnableList(const EnumCreatureType &enumcreaturetype)
{
    // Java enums are singletons, but the C++ value can be copied. Compare the
    // stable category tag instead of object addresses so a valid creature type
    // never loses its biome spawn list merely because it was passed by value.
    if (enumcreaturetype.getCreatureTag() == EnumCreatureTypeTag::monster_tag)
        return &spawnableMonsterList;
    if (enumcreaturetype.getCreatureTag() == EnumCreatureTypeTag::creature_tag)
        return &spawnableCreatureList;
    if (enumcreaturetype.getCreatureTag() == EnumCreatureTypeTag::waterCreature_tag)
        return &spawnableWaterCreatureList;
    return nullptr;
}

void BiomeGenBase::generateBiomeLookup()
{
    for (int_t i = 0; i < 64; i++)
        for (int_t j = 0; j < 64; j++)
            biomeLookupTable[i + j * 64] = getBiome((float)i / 63.0f, (float)j / 63.0f);

    desert->topBlock = desert->fillerBlock = (byte_t)Block::sand->blockID;
    iceDesert->topBlock = iceDesert->fillerBlock = (byte_t)Block::sand->blockID;
}

BiomeGenBase *BiomeGenBase::getBiomeFromLookup(double d, double d1)
{
    int_t i = (int_t)(d * 63.0);
    int_t j = (int_t)(d1 * 63.0);
    return biomeLookupTable[i + j * 64];
}

BiomeGenBase *BiomeGenBase::getBiome(float f, float f1)
{
    f1 *= f;
    if (f < 0.1f) return tundra;
    if (f1 < 0.2f)
    {
        if (f < 0.5f)  return tundra;
        if (f < 0.95f) return savanna;
        else           return desert;
    }
    if (f1 > 0.5f && f < 0.7f) return swampland;
    if (f < 0.5f) return taiga;
    if (f < 0.97f)
    {
        if (f1 < 0.35f) return shrubland;
        else            return forest;
    }
    if (f1 < 0.45f) return plains;
    if (f1 < 0.9f)  return seasonalForest;
    else            return rainforest;
}

void BiomeGenBase::initialize()
{
    rainforest     = ((new BiomeGenRainforest())->setColor(0x8fa36)->setBiomeName(u"Rainforest")->getBiomeAtCoords(0x1ff458));
    swampland      = ((new BiomeGenSwamp())->setColor(0x7f9b2)->setBiomeName(u"Swampland")->getBiomeAtCoords(0x8baf48));
    seasonalForest = ((new BiomeGenBase())->setColor(0x9be023)->setBiomeName(u"Seasonal Forest"));
    forest         = ((new BiomeGenForest())->setColor(0x56621)->setBiomeName(u"Forest")->getBiomeAtCoords(0x4eba31));
    savanna        = ((new BiomeGenDesert())->setColor(0xd9e023)->setBiomeName(u"Savanna"));
    shrubland      = ((new BiomeGenBase())->setColor(0xa1ad20)->setBiomeName(u"Shrubland"));
    taiga          = ((new BiomeGenTaiga())->setColor(0x2eb153)->setBiomeName(u"Taiga")->setEnableSnow()->getBiomeAtCoords(0x7bb731));
    desert         = ((new BiomeGenDesert())->setColor(0xfa9418)->setBiomeName(u"Desert")->setDisableRain());
    plains         = ((new BiomeGenDesert())->setColor(0xffd910)->setBiomeName(u"Plains"));
    iceDesert      = ((new BiomeGenDesert())->setColor(0xffed93)->setBiomeName(u"Ice Desert")->setEnableSnow()->setDisableRain()->getBiomeAtCoords(0xc4d339));
    tundra         = ((new BiomeGenBase())->setColor(0x57ebf9)->setBiomeName(u"Tundra")->setEnableSnow()->getBiomeAtCoords(0xc4d339));
    hell           = ((new BiomeGenHell())->setColor(0xff0000)->setBiomeName(u"Hell")->setDisableRain());
    sky            = ((new BiomeGenSky())->setColor(0x8080ff)->setBiomeName(u"Sky")->setDisableRain());
    generateBiomeLookup();
}
