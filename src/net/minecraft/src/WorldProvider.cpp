#include "WorldProvider.h"
#include "World.h"
#include "WorldChunkManager.h"
#include "BiomeGenBase.h"
#include "ChunkProviderGenerate.h"
#include "ChunkProviderFlat.h"
#include "BlockGrass.h"

#include "WorldInfo.h"
#include "Block.h"
#include "WorldProviderHell.h"
#include "WorldProviderSurface.h"
#include "WorldProviderSky.h"
#include <cmath>

WorldProvider::WorldProvider()
    : isNether(false), isHellWorld(false), hasNoSky(false), worldType(0)
{
    for (int_t i = 0; i < 16; i++) lightBrightnessTable[i] = 0.0f;
    for (int_t i = 0; i < 4; i++) colorsSunriseSunset[i] = 0.0f;
}

WorldProvider::~WorldProvider()
{
    delete worldChunkMgr;
}

void WorldProvider::registerWorld(World *world)
{
    worldObj = world;
    registerWorldChunkManager();
    generateLightBrightnessTable();
}

void WorldProvider::generateLightBrightnessTable()
{
    float f = 0.05f;
    for (int_t i = 0; i <= 15; i++)
    {
        float f1 = 1.0f - (float)i / 15.0f;
        lightBrightnessTable[i] = ((1.0f - f1) / (f1 * 3.0f + 1.0f)) * (1.0f - f) + f;
    }
}

void WorldProvider::registerWorldChunkManager()
{
    worldChunkMgr = new WorldChunkManager(worldObj);
    // Superflat is always Forest.
    if (worldObj != nullptr && worldObj->getWorldInfo() != nullptr &&
        worldObj->getWorldInfo()->generatorType == 1)
        worldChunkMgr->forcedBiome = BiomeGenBase::forest;
}

IChunkProvider *WorldProvider::getChunkProvider()
{
    if (worldObj->getWorldInfo() != nullptr && worldObj->getWorldInfo()->generatorType == 1)
        return new ChunkProviderFlat(worldObj, worldObj->getRandomSeed(), ChunkProviderFlat::SUPERFLAT);
    return new ChunkProviderGenerate(worldObj, worldObj->getRandomSeed());
}

bool WorldProvider::canCoordinateBeSpawn(int_t i, int_t j)
{
    int_t k = worldObj->getFirstUncoveredBlock(i, j);
    if (worldObj->getWorldInfo() != nullptr && worldObj->getWorldInfo()->generatorType == 1)
        return k == Block::grass->blockID;
    return k == Block::sand->blockID;
}

float WorldProvider::calculateCelestialAngle(long_t l, float f)
{
    int_t i = (int_t)(l % 24000LL);
    float f1 = ((float)i + f) / 24000.0f - 0.25f;
    if (f1 < 0.0f) f1++;
    if (f1 > 1.0f) f1--;
    float f2 = f1;
    f1 = 1.0f - (float)((std::cos((double)f1 * 3.14159265358979) + 1.0) / 2.0);
    f1 = f2 + (f1 - f2) / 3.0f;
    return f1;
}

float *WorldProvider::calcSunriseSunsetColors(float f, float f1)
{
    float f2 = 0.4f;
    float f3 = MathHelper::cos(f * 3.141593f) - 0.0f;
    if (f3 >= -f2 && f3 <= f2)
    {
        float f5 = (f3 / f2) * 0.5f + 0.5f;
        float f6 = 1.0f - (1.0f - MathHelper::sin(f5 * 3.141593f)) * 0.99f;
        f6 *= f6;
        colorsSunriseSunset[0] = f5 * 0.3f + 0.7f;
        colorsSunriseSunset[1] = f5 * f5 * 0.7f + 0.2f;
        colorsSunriseSunset[2] = f5 * f5 * 0.0f + 0.2f;
        colorsSunriseSunset[3] = f6;
        return colorsSunriseSunset;
    }
    return nullptr;
}

Vec3D *WorldProvider::getFogColor(float f, float f1)
{
    float f2 = MathHelper::cos(f * 3.141593f * 2.0f) * 2.0f + 0.5f;
    if (f2 < 0.0f) f2 = 0.0f;
    if (f2 > 1.0f) f2 = 1.0f;
    float f3 = 0.7529412f, f4 = 0.8470588f, f5 = 1.0f;
    f3 *= f2 * 0.94f + 0.06f;
    f4 *= f2 * 0.94f + 0.06f;
    f5 *= f2 * 0.91f + 0.09f;
    return Vec3D::createVector(f3, f4, f5);
}

bool WorldProvider::canRespawnHere()  { return true; }
float WorldProvider::getCloudHeight() { return 108.0f; }
bool WorldProvider::hasVoidParticles()    { return true; }

WorldProvider *WorldProvider::getProviderForDimension(int_t i)
{
    if (i == -1) return new WorldProviderHell();
    if (i ==  0) return new WorldProviderSurface();
    if (i ==  1) return new WorldProviderSky();
    return nullptr;
}
