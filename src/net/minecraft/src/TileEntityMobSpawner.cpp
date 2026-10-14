#include "TileEntityMobSpawner.h"

#include <cmath>

#include "NBTTagCompound.h"
#include "World.h"
#include "EntityList.h"
#include "EntityLiving.h"
#include "AxisAlignedBB.h"
#include "java/Random.h"

TileEntityMobSpawner::TileEntityMobSpawner()
    : delay(-1)
    , yaw(0.0)
    , yaw2(0.0)
    , mobID("Pig")
{
    delay = 20;
}

std::string TileEntityMobSpawner::getMobID()
{
    return mobID;
}

void TileEntityMobSpawner::setMobID(const std::string &s)
{
    mobID = s;
}

bool TileEntityMobSpawner::anyPlayerInRange()
{
    return worldObj->getClosestPlayer((double)xCoord + 0.5, (double)yCoord + 0.5, (double)zCoord + 0.5, 16.0) != nullptr;
}

void TileEntityMobSpawner::readFromNBT(NBTTagCompound* nbttagcompound)
{
    TileEntity::readFromNBT(nbttagcompound);
    mobID = nbttagcompound->getString("EntityId");
    delay = nbttagcompound->getShort("Delay");
}

void TileEntityMobSpawner::writeToNBT(NBTTagCompound* nbttagcompound)
{
    TileEntity::writeToNBT(nbttagcompound);
    nbttagcompound->setString("EntityId", mobID);
    nbttagcompound->setShort("Delay", (short)delay);
}

void TileEntityMobSpawner::updateEntity()
{
    yaw2 = yaw;
    if (!anyPlayerInRange())
        return;
    double d  = (float)xCoord + worldObj->rand.nextFloat();
    double d2 = (float)yCoord + worldObj->rand.nextFloat();
    double d4 = (float)zCoord + worldObj->rand.nextFloat();
    worldObj->spawnParticle("smoke", d, d2, d4, 0.0, 0.0, 0.0);
    worldObj->spawnParticle("flame", d, d2, d4, 0.0, 0.0, 0.0);
    for (yaw += 1000.0f / ((float)delay + 200.0f); yaw > 360.0;)
    {
        yaw  -= 360.0;
        yaw2 -= 360.0;
    }

    if (!worldObj->multiplayerWorld)
    {
        if (delay == -1)
            updateDelay();
        if (delay > 0)
        {
            delay--;
            return;
        }
        const int_t byte0 = 4;
        for (int_t i = 0; i < byte0; i++)
        {
            EntityLiving *entityliving = (EntityLiving *)EntityList::createEntityInWorld(mobID, worldObj);
            if (entityliving == nullptr)
                return;
            int_t j = (int_t)worldObj->getEntitiesWithinAABB(typeid(*entityliving),
                AxisAlignedBB::getBoundingBoxFromPool(xCoord, yCoord, zCoord, xCoord + 1, yCoord + 1, zCoord + 1)->expand(8.0, 4.0, 8.0)).size();
            if (j >= 6)
            {
                updateDelay();
                return;
            }
            if (entityliving == nullptr)
                continue;
            double d6 = (double)xCoord + (worldObj->rand.nextDouble() - worldObj->rand.nextDouble()) * 4.0;
            double d7 = (yCoord + worldObj->rand.nextInt(3)) - 1;
            double d8 = (double)zCoord + (worldObj->rand.nextDouble() - worldObj->rand.nextDouble()) * 4.0;
            entityliving->setLocationAndAngles(d6, d7, d8, worldObj->rand.nextFloat() * 360.0f, 0.0f);
            if (!entityliving->getCanSpawnHere())
                continue;
            worldObj->entityJoinedWorld(entityliving);
            for (int_t k = 0; k < 20; k++)
            {
                double d1 = (double)xCoord + 0.5 + ((double)worldObj->rand.nextFloat() - 0.5) * 2.0;
                double d3 = (double)yCoord + 0.5 + ((double)worldObj->rand.nextFloat() - 0.5) * 2.0;
                double d5 = (double)zCoord + 0.5 + ((double)worldObj->rand.nextFloat() - 0.5) * 2.0;
                worldObj->spawnParticle("smoke", d1, d3, d5, 0.0, 0.0, 0.0);
                worldObj->spawnParticle("flame", d1, d3, d5, 0.0, 0.0, 0.0);
            }
            entityliving->spawnExplosionParticle();
            updateDelay();
        }
    }
    TileEntity::updateEntity();
}

void TileEntityMobSpawner::updateDelay()
{
    delay = 200 + worldObj->rand.nextInt(600);
}
