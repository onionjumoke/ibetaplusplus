#include "EntityArrow.h"

#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "World.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "Item.h"
#include "ItemStack.h"
#include "NBTTagCompound.h"
#include "MathHelper.h"
#include "Block.h"
#include "MovingObjectPosition.h"
#include "Vec3D.h"
#include "AxisAlignedBB.h"

EntityArrow::EntityArrow(World *world)
    : Entity(world)
    , xTile(-1), yTile(-1), zTile(-1)
    , inTile(0), inData(0), inGround(false)
    , doesArrowBelongToPlayer(false)
    , arrowShake(0)
    , ticksInGround(0), ticksInAir(0)
    , owner(nullptr)
{
    setSize(0.5f, 0.5f);
}

EntityArrow::EntityArrow(World *world, double x, double y, double z)
    : EntityArrow(world)
{
    setPosition(x, y, z);
    yOffset = 0.0f;
}

EntityArrow::EntityArrow(World *world, EntityLiving *entityliving)
    : EntityArrow(world)
{
    owner = entityliving;
    doesArrowBelongToPlayer = (dynamic_cast<EntityPlayer*>(entityliving) != nullptr);
    setLocationAndAngles(entityliving->posX, entityliving->posY + entityliving->getEyeHeight(),
                         entityliving->posZ, entityliving->rotationYaw, entityliving->rotationPitch);
    posX -= MathHelper::cos(rotationYaw / 180.0f * (float)M_PI) * 0.16f;
    posY -= 0.10000000149011612;
    posZ -= MathHelper::sin(rotationYaw / 180.0f * (float)M_PI) * 0.16f;
    setPosition(posX, posY, posZ);
    yOffset = 0.0f;
    motionX = -MathHelper::sin(rotationYaw / 180.0f * (float)M_PI)
            * MathHelper::cos(rotationPitch / 180.0f * (float)M_PI);
    motionZ =  MathHelper::cos(rotationYaw / 180.0f * (float)M_PI)
            * MathHelper::cos(rotationPitch / 180.0f * (float)M_PI);
    motionY = -MathHelper::sin(rotationPitch / 180.0f * (float)M_PI);
    setArrowHeading(motionX, motionY, motionZ, 1.5f, 1.0f);
}

void EntityArrow::entityInit()
{
}

void EntityArrow::setArrowHeading(double dx, double dy, double dz, float speed, float spread)
{
    float len = MathHelper::sqrt_float((float)(dx * dx + dy * dy + dz * dz));
    dx /= len;
    dy /= len;
    dz /= len;
    dx += rand.nextGaussian() * 0.0075 * (double)spread;
    dy += rand.nextGaussian() * 0.0075 * (double)spread;
    dz += rand.nextGaussian() * 0.0075 * (double)spread;
    dx *= speed;
    dy *= speed;
    dz *= speed;
    motionX = dx;
    motionY = dy;
    motionZ = dz;
    float horiz = MathHelper::sqrt_float((float)(dx * dx + dz * dz));
    prevRotationYaw   = rotationYaw   = (float)(std::atan2(dx, dz) * 180.0 / M_PI);
    prevRotationPitch = rotationPitch = (float)(std::atan2(dy, (double)horiz) * 180.0 / M_PI);
    ticksInGround = 0;
}

void EntityArrow::setVelocity(double dx, double dy, double dz)
{
    motionX = dx;
    motionY = dy;
    motionZ = dz;
    if (prevRotationPitch == 0.0f && prevRotationYaw == 0.0f)
    {
        float horiz = MathHelper::sqrt_float((float)(dx * dx + dz * dz));
        prevRotationYaw   = rotationYaw   = (float)(std::atan2(dx, dz) * 180.0 / M_PI);
        prevRotationPitch = rotationPitch = (float)(std::atan2(dy, (double)horiz) * 180.0 / M_PI);
    }
}

void EntityArrow::onUpdate()
{
    Entity::onUpdate();

    if (prevRotationPitch == 0.0f && prevRotationYaw == 0.0f)
    {
        float horiz = MathHelper::sqrt_float((float)(motionX * motionX + motionZ * motionZ));
        prevRotationYaw   = rotationYaw   = (float)(std::atan2(motionX, motionZ) * 180.0 / M_PI);
        prevRotationPitch = rotationPitch = (float)(std::atan2(motionY, (double)horiz) * 180.0 / M_PI);
    }

    int_t blockId = worldObj->getBlockId(xTile, yTile, zTile);
    if (blockId > 0)
    {
        Block::blocksList[blockId]->setBlockBoundsBasedOnState(worldObj, xTile, yTile, zTile);
        AxisAlignedBB *aabb = Block::blocksList[blockId]->getCollisionBoundingBoxFromPool(worldObj, xTile, yTile, zTile);
        if (aabb != nullptr && aabb->isVecInside(Vec3D::createVector(posX, posY, posZ)))
            inGround = true;
    }

    if (arrowShake > 0)
        arrowShake--;

    if (inGround)
    {
        int_t j = worldObj->getBlockId(xTile, yTile, zTile);
        int_t k = worldObj->getBlockMetadata(xTile, yTile, zTile);
        if (j != inTile || k != inData)
        {
            inGround = false;
            motionX *= rand.nextFloat() * 0.2f;
            motionY *= rand.nextFloat() * 0.2f;
            motionZ *= rand.nextFloat() * 0.2f;
            ticksInGround = 0;
            ticksInAir = 0;
        }
        else
        {
            ticksInGround++;
            if (ticksInGround == 1200)
                setEntityDead();
        }
        return;
    }

    ticksInAir++;
    Vec3D *vec3d  = Vec3D::createVector(posX, posY, posZ);
    Vec3D *vec3d1 = Vec3D::createVector(posX + motionX, posY + motionY, posZ + motionZ);

    MovingObjectPosition *mop = worldObj->rayTraceBlocks(vec3d, vec3d1, false, true);

    vec3d  = Vec3D::createVector(posX, posY, posZ);
    vec3d1 = Vec3D::createVector(posX + motionX, posY + motionY, posZ + motionZ);

    if (mop != nullptr)
        vec3d1 = Vec3D::createVector(mop->hitVec->xCoord, mop->hitVec->yCoord, mop->hitVec->zCoord);

    Entity *hitEntity = nullptr;
    std::vector<Entity *> &candidates = worldObj->getEntitiesWithinAABBExcludingEntity(
        this, boundingBox->addCoord(motionX, motionY, motionZ)->expand(1.0, 1.0, 1.0));
    double minDist = 0.0;

    for (Entity *candidate : candidates)
    {
        if (!candidate->canBeCollidedWith() || (candidate == owner && ticksInAir < 5))
            continue;
        AxisAlignedBB *expanded = candidate->boundingBox->expand(0.3, 0.3, 0.3);
        MovingObjectPosition *intercept = expanded->calculateIntercept(vec3d, vec3d1);
        if (intercept == nullptr)
            continue;
        double dist = vec3d->distanceTo(intercept->hitVec);
        if (dist < minDist || minDist == 0.0)
        {
            hitEntity = candidate;
            minDist = dist;
        }
    }

    if (hitEntity != nullptr)
        mop = new MovingObjectPosition(hitEntity);

    if (mop != nullptr)
    {
        if (mop->entityHit != nullptr)
        {
            if (mop->entityHit->attackEntityFrom(owner, 4))
            {
                worldObj->playSoundAtEntity(this, u"random.drr", 1.0f, 1.2f / (rand.nextFloat() * 0.2f + 0.9f));
                setEntityDead();
            }
            else
            {
                motionX *= -0.1;
                motionY *= -0.1;
                motionZ *= -0.1;
                rotationYaw     += 180.0f;
                prevRotationYaw += 180.0f;
                ticksInAir = 0;
            }
        }
        else
        {
            xTile = mop->blockX;
            yTile = mop->blockY;
            zTile = mop->blockZ;
            inTile = worldObj->getBlockId(xTile, yTile, zTile);
            inData = worldObj->getBlockMetadata(xTile, yTile, zTile);
            motionX = (float)(mop->hitVec->xCoord - posX);
            motionY = (float)(mop->hitVec->yCoord - posY);
            motionZ = (float)(mop->hitVec->zCoord - posZ);
            float speed = MathHelper::sqrt_float((float)(motionX * motionX + motionY * motionY + motionZ * motionZ));
            posX -= (motionX / (double)speed) * 0.05;
            posY -= (motionY / (double)speed) * 0.05;
            posZ -= (motionZ / (double)speed) * 0.05;
            worldObj->playSoundAtEntity(this, u"random.drr", 1.0f, 1.2f / (rand.nextFloat() * 0.2f + 0.9f));
            inGround  = true;
            arrowShake = 7;
        }
    }

    posX += motionX;
    posY += motionY;
    posZ += motionZ;

    float horizSpeed = MathHelper::sqrt_float((float)(motionX * motionX + motionZ * motionZ));
    rotationYaw = (float)(std::atan2(motionX, motionZ) * 180.0 / M_PI);
    for (rotationPitch = (float)(std::atan2(motionY, (double)horizSpeed) * 180.0 / M_PI);
         rotationPitch - prevRotationPitch < -180.0f; prevRotationPitch -= 360.0f) {}
    for (; rotationPitch - prevRotationPitch >= 180.0f; prevRotationPitch += 360.0f) {}
    for (; rotationYaw - prevRotationYaw < -180.0f; prevRotationYaw -= 360.0f) {}
    for (; rotationYaw - prevRotationYaw >= 180.0f; prevRotationYaw += 360.0f) {}
    rotationPitch = prevRotationPitch + (rotationPitch - prevRotationPitch) * 0.2f;
    rotationYaw   = prevRotationYaw   + (rotationYaw   - prevRotationYaw)   * 0.2f;

    float drag  = 0.99f;
    float gravity = 0.03f;
    if (isInWater())
    {
        for (int_t i = 0; i < 4; i++)
            worldObj->spawnParticle(u"bubble",
                posX - motionX * 0.25, posY - motionY * 0.25, posZ - motionZ * 0.25,
                motionX, motionY, motionZ);
        drag = 0.8f;
    }
    motionX *= drag;
    motionY *= drag;
    motionZ *= drag;
    motionY -= gravity;
    setPosition(posX, posY, posZ);
}

void EntityArrow::writeEntityToNBT(NBTTagCompound *nbt)
{
    nbt->setShort("xTile",  (short_t)xTile);
    nbt->setShort("yTile",  (short_t)yTile);
    nbt->setShort("zTile",  (short_t)zTile);
    nbt->setByte("inTile",  (byte_t)inTile);
    nbt->setByte("shake",   (byte_t)arrowShake);
    nbt->setByte("inGround",(byte_t)(inGround ? 1 : 0));
    nbt->setByte("player",  (byte_t)(doesArrowBelongToPlayer ? 1 : 0));
}

void EntityArrow::readEntityFromNBT(NBTTagCompound *nbt)
{
    xTile    = nbt->getShort("xTile");
    yTile    = nbt->getShort("yTile");
    zTile    = nbt->getShort("zTile");
    inTile   = nbt->getByte("inTile") & 0xff;
    arrowShake = nbt->getByte("shake") & 0xff;
    inGround = nbt->getByte("inGround") == 1;
    doesArrowBelongToPlayer = nbt->getByte("player") == 1;
}

void EntityArrow::onCollideWithPlayer(EntityPlayer *entityplayer)
{
    if (!worldObj->multiplayerWorld && inGround && arrowShake <= 0 && doesArrowBelongToPlayer)
    {
        if (entityplayer->inventory->addItemStackToInventory(new ItemStack(Item::arrow, 1)))
        {
            worldObj->playSoundAtEntity(entityplayer, u"random.pop", 0.2f, ((rand.nextFloat() - rand.nextFloat()) * 0.7f + 1.0f) * 2.0f);
            entityplayer->onItemPickup(this, 1);
            isDead = true;
        }
    }
}

float EntityArrow::getShadowSize()
{
    return 0.0f;
}
