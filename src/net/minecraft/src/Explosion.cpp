#include "Explosion.h"

#include "World.h"
#include "Entity.h"
#include "Block.h"
#include "MathHelper.h"
#include "AxisAlignedBB.h"
#include "Vec3D.h"
#include "ChunkPosition.h"
#include <cmath>
#include <vector>
#include "BlockFire.h"

Explosion::Explosion(World *world, Entity *entity, double x, double y, double z, float size) :
    isFlaming(false),
    explosionX(x),
    explosionY(y),
    explosionZ(z),
    exploder(entity),
    explosionSize(size),
    worldObj(world)
{
}

void Explosion::doExplosionA()
{
    float f = explosionSize;
    int_t i = 16;

    for (int_t j = 0; j < i; j++)
    {
        for (int_t l = 0; l < i; l++)
        {
            for (int_t j1 = 0; j1 < i; j1++)
            {
                if (j != 0 && j != i - 1 && l != 0 && l != i - 1 && j1 != 0 && j1 != i - 1)
                    continue;

                double d  = ((float)j  / ((float)i - 1.0f)) * 2.0f - 1.0f;
                double d1 = ((float)l  / ((float)i - 1.0f)) * 2.0f - 1.0f;
                double d2 = ((float)j1 / ((float)i - 1.0f)) * 2.0f - 1.0f;
                double d3 = std::sqrt(d * d + d1 * d1 + d2 * d2);
                d  /= d3;
                d1 /= d3;
                d2 /= d3;

                std::uniform_real_distribution<float> dist(0.0f, 1.0f);
                float f1 = explosionSize * (0.7f + dist(explosionRNG) * 0.6f);
                double d5 = explosionX;
                double d7 = explosionY;
                double d9 = explosionZ;
                float f2 = 0.3f;

                while (true)
                {
                    if (f1 <= 0.0f)
                        break;

                    int_t j4 = MathHelper::floor_double(d5);
                    int_t k4 = MathHelper::floor_double(d7);
                    int_t l4 = MathHelper::floor_double(d9);
                    int_t i5 = worldObj->getBlockId(j4, k4, l4);

                    if (i5 > 0)
                        f1 -= (Block::blocksList[i5]->getExplosionResistance(exploder) + 0.3f) * f2;

                    if (f1 > 0.0f)
                        destroyedBlockPositions.insert({j4, k4, l4});

                    d5 += d  * (double)f2;
                    d7 += d1 * (double)f2;
                    d9 += d2 * (double)f2;
                    f1 -= f2 * 0.75f;
                }
            }
        }
    }

    explosionSize *= 2.0f;
    int_t k  = MathHelper::floor_double(explosionX - (double)explosionSize - 1.0);
    int_t i1 = MathHelper::floor_double(explosionX + (double)explosionSize + 1.0);
    int_t k1 = MathHelper::floor_double(explosionY - (double)explosionSize - 1.0);
    int_t l1 = MathHelper::floor_double(explosionY + (double)explosionSize + 1.0);
    int_t i2 = MathHelper::floor_double(explosionZ - (double)explosionSize - 1.0);
    int_t j2 = MathHelper::floor_double(explosionZ + (double)explosionSize + 1.0);

    std::vector<Entity*> list = worldObj->getEntitiesWithinAABBExcludingEntity(
        exploder, AxisAlignedBB::getBoundingBoxFromPool((double)k, (double)k1, (double)i2, (double)i1, (double)l1, (double)j2));

    Vec3D *vec3d = Vec3D::createVector(explosionX, explosionY, explosionZ);

    for (int_t k2 = 0; k2 < (int_t)list.size(); k2++)
    {
        Entity *entity = list[k2];
        double d4 = entity->getDistance(explosionX, explosionY, explosionZ) / (double)explosionSize;
        if (d4 <= 1.0)
        {
            double d6  = entity->posX - explosionX;
            double d8  = entity->posY - explosionY;
            double d10 = entity->posZ - explosionZ;
            double d11 = MathHelper::sqrt_double(d6 * d6 + d8 * d8 + d10 * d10);
            d6  /= d11;
            d8  /= d11;
            d10 /= d11;
            double d12 = worldObj->getBlockDensity(vec3d, entity->boundingBox);
            double d13 = (1.0 - d4) * d12;
            entity->attackEntityFrom(exploder, (int_t)(((d13 * d13 + d13) / 2.0) * 8.0 * (double)explosionSize + 1.0));
            double d14 = d13;
            entity->motionX += d6  * d14;
            entity->motionY += d8  * d14;
            entity->motionZ += d10 * d14;
        }
    }

    explosionSize = f;

    std::vector<ChunkPosTuple> blockList(destroyedBlockPositions.begin(), destroyedBlockPositions.end());

    if (isFlaming)
    {
        for (int_t l2 = (int_t)blockList.size() - 1; l2 >= 0; l2--)
        {
            int_t i3 = blockList[l2].x;
            int_t j3 = blockList[l2].y;
            int_t k3 = blockList[l2].z;
            int_t l3 = worldObj->getBlockId(i3, j3, k3);
            int_t i4 = worldObj->getBlockId(i3, j3 - 1, k3);

            std::uniform_int_distribution<int_t> rng3(0, 2);
            if (l3 == 0 && Block::opaqueCubeLookup[i4] && rng3(explosionRNG) == 0)
                worldObj->setBlockWithNotify(i3, j3, k3, Block::fire->blockID);
        }
    }
}

void Explosion::doExplosionB(bool flag)
{
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    worldObj->playSoundEffect(explosionX, explosionY, explosionZ, u"random.explode", 4.0f,
        (1.0f + (dist(explosionRNG) - dist(explosionRNG)) * 0.2f) * 0.7f);

    std::vector<ChunkPosTuple> blockList(destroyedBlockPositions.begin(), destroyedBlockPositions.end());

    for (int_t i = (int_t)blockList.size() - 1; i >= 0; i--)
    {
        int_t j  = blockList[i].x;
        int_t k  = blockList[i].y;
        int_t l  = blockList[i].z;
        int_t i1 = worldObj->getBlockId(j, k, l);

        if (flag)
        {
            double d  = (float)j + dist(explosionRNG);
            double d1 = (float)k + dist(explosionRNG);
            double d2 = (float)l + dist(explosionRNG);
            double d3 = d - explosionX;
            double d4 = d1 - explosionY;
            double d5 = d2 - explosionZ;
            double d6 = MathHelper::sqrt_double(d3 * d3 + d4 * d4 + d5 * d5);
            d3 /= d6;
            d4 /= d6;
            d5 /= d6;
            double d7 = 0.5 / (d6 / (double)explosionSize + 0.1);
            d7 *= dist(explosionRNG) * dist(explosionRNG) + 0.3f;
            d3 *= d7;
            d4 *= d7;
            d5 *= d7;
            worldObj->spawnParticle(u"explode", (d + explosionX * 1.0) / 2.0, (d1 + explosionY * 1.0) / 2.0, (d2 + explosionZ * 1.0) / 2.0, d3, d4, d5);
            worldObj->spawnParticle(u"smoke", d, d1, d2, d3, d4, d5);
        }

        if (i1 > 0)
        {
            Block::blocksList[i1]->dropBlockAsItemWithChance(worldObj, j, k, l, worldObj->getBlockMetadata(j, k, l), 0.3f);
            worldObj->setBlockWithNotify(j, k, l, 0);
            Block::blocksList[i1]->onBlockDestroyedByExplosion(worldObj, j, k, l);
        }
    }
}
