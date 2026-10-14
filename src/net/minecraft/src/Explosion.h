#pragma once

#include <set>
#include <random>
#include "java/Type.h"

class World;
class Entity;

struct ChunkPositionCmp
{
    bool operator()(const struct ChunkPosTuple &a, const struct ChunkPosTuple &b) const;
};

struct ChunkPosTuple
{
    int_t x, y, z;
    bool operator<(const ChunkPosTuple &o) const
    {
        if (x != o.x) return x < o.x;
        if (y != o.y) return y < o.y;
        return z < o.z;
    }
};

// net.minecraft.src.Explosion
class Explosion
{
public:
    Explosion(World *world, Entity *entity, double x, double y, double z, float size);

    void doExplosionA();
    void doExplosionB(bool flag);

    bool isFlaming;
    double explosionX;
    double explosionY;
    double explosionZ;
    Entity *exploder;
    float explosionSize;
    std::set<ChunkPosTuple> destroyedBlockPositions;

private:
    std::mt19937 explosionRNG;
    World *worldObj;
};
