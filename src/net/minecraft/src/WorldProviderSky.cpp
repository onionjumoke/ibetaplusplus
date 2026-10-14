#include "WorldProviderSky.h"
#include "WorldChunkManagerHell.h"
#include "ChunkProviderSky.h"
#include "BiomeGenBase.h"
#include "Block.h"
#include "World.h"
#include "Vec3D.h"
#include "MathHelper.h"

void WorldProviderSky::registerWorldChunkManager()
{
    worldChunkMgr = new WorldChunkManagerHell(BiomeGenBase::sky, 0.5, 0.0);
    worldType = 1;
}

IChunkProvider *WorldProviderSky::getChunkProvider()
{
    return new ChunkProviderSky(worldObj, worldObj->getRandomSeed());
}

float WorldProviderSky::calculateCelestialAngle(long_t l, float f)
{
    return 0.0f;
}

float *WorldProviderSky::calcSunriseSunsetColors(float f, float f1)
{
    return nullptr;
}

Vec3D *WorldProviderSky::getFogColor(float f, float f1) // func_4096_a override — End sky color
{
    int_t i = 0x8080a0;
    float f2 = MathHelper::cos(f * 3.141593f * 2.0f) * 2.0f + 0.5f;
    if (f2 < 0.0f) f2 = 0.0f;
    if (f2 > 1.0f) f2 = 1.0f;
    float f3 = (float)((i >> 16) & 0xff) / 255.0f;
    float f4 = (float)((i >> 8) & 0xff) / 255.0f;
    float f5 = (float)(i & 0xff) / 255.0f;
    f3 *= f2 * 0.94f + 0.06f;
    f4 *= f2 * 0.94f + 0.06f;
    f5 *= f2 * 0.91f + 0.09f;
    return Vec3D::createVector(f3, f4, f5);
}

bool WorldProviderSky::hasVoidParticles()    { return false; }
float WorldProviderSky::getCloudHeight() { return 8.0f; }

bool WorldProviderSky::canCoordinateBeSpawn(int_t i, int_t j)
{
    int_t k = worldObj->getFirstUncoveredBlock(i, j);
    if (k == 0) return false;
    return Block::blocksList[k]->blockMaterial->getIsSolid();
}
