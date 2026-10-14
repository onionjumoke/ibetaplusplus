#include "ItemBucket.h"
#include "ItemStack.h"
#include "EntityPlayer.h"
#include "Vec3D.h"
#include "MathHelper.h"
#include "World.h"
#include "MovingObjectPosition.h"
#include "EnumMovingObjectType.h"
#include "Material.h"
#include "WorldProvider.h"
#include "Block.h"
#include "EntityCow.h"

ItemBucket::ItemBucket(int i, int j)
    : Item(i), isFull(j) {
    maxStackSize = 1;
}

ItemStack* ItemBucket::onItemRightClick(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) {
    float f = 1.0f;
    float f1 = entityplayer->prevRotationPitch + (entityplayer->rotationPitch - entityplayer->prevRotationPitch) * f;
    float f2 = entityplayer->prevRotationYaw + (entityplayer->rotationYaw - entityplayer->prevRotationYaw) * f;
    double d = entityplayer->prevPosX + (entityplayer->posX - entityplayer->prevPosX) * (double)f;
    double d1 = (entityplayer->prevPosY + (entityplayer->posY - entityplayer->prevPosY) * (double)f + 1.6200000000000001) - (double)entityplayer->yOffset;
    double d2 = entityplayer->prevPosZ + (entityplayer->posZ - entityplayer->prevPosZ) * (double)f;
    Vec3D* vec3d = Vec3D::createVector(d, d1, d2);
    float f3 = MathHelper::cos(-f2 * 0.01745329f - 3.141593f);
    float f4 = MathHelper::sin(-f2 * 0.01745329f - 3.141593f);
    float f5 = -MathHelper::cos(-f1 * 0.01745329f);
    float f6 = MathHelper::sin(-f1 * 0.01745329f);
    float f7 = f4 * f5;
    float f8 = f6;
    float f9 = f3 * f5;
    double d3 = 5.0;
    Vec3D* vec3d1 = vec3d->addVector((double)f7 * d3, (double)f8 * d3, (double)f9 * d3);
    MovingObjectPosition* movingobjectposition = world->rayTraceBlocks_do(vec3d, vec3d1, isFull == 0);
    if (movingobjectposition == nullptr) {
        return itemstack;
    }
    if (movingobjectposition->typeOfHit == EnumMovingObjectType::TILE) {
        int i = movingobjectposition->blockX;
        int j = movingobjectposition->blockY;
        int k = movingobjectposition->blockZ;
        if (!world->loadTexture(entityplayer, i, j, k)) {
            return itemstack;
        }
        if (isFull == 0) {
            if (world->getBlockMaterial(i, j, k) == Material::water && world->getBlockMetadata(i, j, k) == 0) {
                world->setBlockWithNotify(i, j, k, 0);
                return new ItemStack(Item::bucketWater);
            }
            if (world->getBlockMaterial(i, j, k) == Material::lava && world->getBlockMetadata(i, j, k) == 0) {
                world->setBlockWithNotify(i, j, k, 0);
                return new ItemStack(Item::bucketLava);
            }
        } else {
            if (isFull < 0) {
                return new ItemStack(Item::bucketEmpty);
            }
            if (movingobjectposition->sideHit == 0) j--;
            if (movingobjectposition->sideHit == 1) j++;
            if (movingobjectposition->sideHit == 2) k--;
            if (movingobjectposition->sideHit == 3) k++;
            if (movingobjectposition->sideHit == 4) i--;
            if (movingobjectposition->sideHit == 5) i++;
            if (world->isAirBlock(i, j, k) || !world->getBlockMaterial(i, j, k)->isSolid()) {
                if (world->worldProvider->isHellWorld && isFull == Block::waterMoving->blockID) {
                    world->playSoundEffect(d + 0.5, d1 + 0.5, d2 + 0.5, "random.fizz", 0.5f, 2.6f + (world->rand.nextFloat() - world->rand.nextFloat()) * 0.8f);
                    for (int l = 0; l < 8; l++) {
                        world->spawnParticle("largesmoke", (double)i + world->rand.nextDouble(), (double)j + world->rand.nextDouble(), (double)k + world->rand.nextDouble(), 0.0, 0.0, 0.0);
                    }
                } else {
                    world->setBlockAndMetadataWithNotify(i, j, k, isFull, 0);
                }
                return new ItemStack(Item::bucketEmpty);
            }
        }
    } else if (isFull == 0 && dynamic_cast<EntityCow*>(movingobjectposition->entityHit)) {
        return new ItemStack(Item::bucketMilk);
    }
    return itemstack;
}
