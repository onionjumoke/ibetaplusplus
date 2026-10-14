#include "WorldProviderHell.h"
#include "WorldChunkManagerHell.h"
#include "ChunkProviderHell.h"
#include "ChunkProviderFlat.h"
#include "BiomeGenBase.h"
#include "Block.h"
#include "World.h"
#include "Vec3D.h"

void WorldProviderHell::registerWorldChunkManager()
{
    worldChunkMgr = new WorldChunkManagerHell(BiomeGenBase::hell, 1.0, 0.0);
    isNether    = true;
    isHellWorld = true;
    hasNoSky    = true;
    worldType   = -1;
}

void WorldProviderHell::generateLightBrightnessTable()
{
    float f = 0.1f;
    for (int_t i = 0; i <= 15; i++)
    {
        float f1 = 1.0f - (float)i / 15.0f;
        lightBrightnessTable[i] = ((1.0f - f1) / (f1 * 3.0f + 1.0f)) * (1.0f - f) + f;
    }
}

Vec3D *WorldProviderHell::getFogColor(float f, float f1) // func_4096_a — nether fog color
{
    return Vec3D::createVector(0.20000000298023224, 0.029999999329447746, 0.029999999329447746);
}

IChunkProvider *WorldProviderHell::getChunkProvider()
{
    return new ChunkProviderFlat(worldObj, worldObj->getRandomSeed(), ChunkProviderFlat::NETHER);
}

bool WorldProviderHell::canCoordinateBeSpawn(int_t i, int_t j)
{
    int_t k = worldObj->getFirstUncoveredBlock(i, j);
    if (k == Block::bedrock->blockID) return false;
    if (k == 0) return false;
    return Block::opaqueCubeLookup[k];
}

float WorldProviderHell::calculateCelestialAngle(long_t l, float f)
{
    return 0.5f;
}

bool WorldProviderHell::canRespawnHere()
{
    return false;
}
