#include "BlockDispenser.h"
#include "Material.h"
#include "Block.h"
#include "World.h"
#include "IBlockAccess.h"
#include "TileEntityDispenser.h"
#include "EntityPlayer.h"
#include "EntityLiving.h"
#include "ItemStack.h"
#include "Item.h"
#include "EntityArrow.h"
#include "EntityEgg.h"
#include "EntitySnowball.h"
#include "EntityItem.h"
#include "MathHelper.h"
#include "TileEntity.h"

BlockDispenser::BlockDispenser(int_t i) : BlockContainer(i, Material::rock)
{
    blockIndexInTexture = 45;
}

int_t BlockDispenser::tickRate() { return 4; }

int_t BlockDispenser::idDropped(int_t i, Random &random)
{
    return Block::dispenser->blockID;
}

void BlockDispenser::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
    BlockContainer::onBlockAdded(world, i, j, k);
    setDispenserDefaultDirection(world, i, j, k);
}

void BlockDispenser::setDispenserDefaultDirection(World *world, int_t i, int_t j, int_t k)
{
    if (world->multiplayerWorld) return;
    int_t l  = world->getBlockId(i, j, k - 1);
    int_t i1 = world->getBlockId(i, j, k + 1);
    int_t j1 = world->getBlockId(i - 1, j, k);
    int_t k1 = world->getBlockId(i + 1, j, k);
    byte_t byte0 = 3;
    if (Block::opaqueCubeLookup[l]  && !Block::opaqueCubeLookup[i1]) byte0 = 3;
    if (Block::opaqueCubeLookup[i1] && !Block::opaqueCubeLookup[l])  byte0 = 2;
    if (Block::opaqueCubeLookup[j1] && !Block::opaqueCubeLookup[k1]) byte0 = 5;
    if (Block::opaqueCubeLookup[k1] && !Block::opaqueCubeLookup[j1]) byte0 = 4;
    world->setBlockMetadataWithNotify(i, j, k, byte0);
}

int_t BlockDispenser::getBlockTexture(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
    if (l == 1) return blockIndexInTexture + 17;
    if (l == 0) return blockIndexInTexture + 17;
    int_t i1 = iblockaccess->getBlockMetadata(i, j, k);
    if (l != i1) return blockIndexInTexture;
    return blockIndexInTexture + 1;
}

int_t BlockDispenser::getBlockTextureFromSide(int_t i)
{
    if (i == 1) return blockIndexInTexture + 17;
    if (i == 0) return blockIndexInTexture + 17;
    if (i == 3) return blockIndexInTexture + 1;
    return blockIndexInTexture;
}

bool BlockDispenser::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
    if (world->multiplayerWorld) return true;
    TileEntityDispenser *tileentitydispenser = (TileEntityDispenser*)world->getBlockTileEntity(i, j, k);
    entityplayer->displayGUIDispenser(tileentitydispenser);
    return true;
}

void BlockDispenser::dispenseItem(World *world, int_t i, int_t j, int_t k, Random &random)
{
    int_t l  = world->getBlockMetadata(i, j, k);
    int_t i1 = 0;
    int_t j1 = 0;
    if (l == 3)      j1 =  1;
    else if (l == 2) j1 = -1;
    else if (l == 5) i1 =  1;
    else             i1 = -1;
    TileEntityDispenser *tileentitydispenser = (TileEntityDispenser*)world->getBlockTileEntity(i, j, k);
    ItemStack *itemstack = tileentitydispenser->getRandomStackFromInventory();
    double d  = (double)i + (double)i1 * 0.6 + 0.5;
    double d1 = (double)j + 0.5;
    double d2 = (double)k + (double)j1 * 0.6 + 0.5;
    if (itemstack == nullptr)
    {
        world->isTrapdoorOpen(1001, i, j, k, 0);
    }
    else
    {
        if (itemstack->itemID == Item::arrow->shiftedIndex)
        {
            EntityArrow *entityarrow = new EntityArrow(world, d, d1, d2);
            entityarrow->setArrowHeading(i1, 0.1, j1, 1.1f, 6.0f);
            entityarrow->doesArrowBelongToPlayer = true;
            world->entityJoinedWorld(entityarrow);
            world->isTrapdoorOpen(1002, i, j, k, 0);
        }
        else if (itemstack->itemID == Item::egg->shiftedIndex)
        {
            EntityEgg *entityegg = new EntityEgg(world, d, d1, d2);
            entityegg->setEggHeading(i1, 0.1, j1, 1.1f, 6.0f);
            world->entityJoinedWorld(entityegg);
            world->isTrapdoorOpen(1002, i, j, k, 0);
        }
        else if (itemstack->itemID == Item::snowball->shiftedIndex)
        {
            EntitySnowball *entitysnowball = new EntitySnowball(world, d, d1, d2);
            entitysnowball->setSnowballHeading(i1, 0.1, j1, 1.1f, 6.0f);
            world->entityJoinedWorld(entitysnowball);
            world->isTrapdoorOpen(1002, i, j, k, 0);
        }
        else
        {
            EntityItem *entityitem = new EntityItem(world, d, d1 - 0.3, d2, itemstack);
            double d3 = random.nextDouble() * 0.1 + 0.2;
            entityitem->motionX = (double)i1 * d3;
            entityitem->motionY = 0.2;
            entityitem->motionZ = (double)j1 * d3;
            entityitem->motionX += random.nextGaussian() * 0.007499999832361937 * 6.0;
            entityitem->motionY += random.nextGaussian() * 0.007499999832361937 * 6.0;
            entityitem->motionZ += random.nextGaussian() * 0.007499999832361937 * 6.0;
            world->entityJoinedWorld(entityitem);
            world->isTrapdoorOpen(1000, i, j, k, 0);
        }
        world->isTrapdoorOpen(2000, i, j, k, i1 + 1 + (j1 + 1) * 3);
    }
}

void BlockDispenser::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
    if (l > 0 && Block::blocksList[l]->canProvidePower())
    {
        bool flag = world->isBlockIndirectlyGettingPowered(i, j, k) || world->isBlockIndirectlyGettingPowered(i, j + 1, k);
        if (flag) world->scheduleBlockUpdate(i, j, k, blockID, tickRate());
    }
}

void BlockDispenser::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
    if (world->isBlockIndirectlyGettingPowered(i, j, k) || world->isBlockIndirectlyGettingPowered(i, j + 1, k))
        dispenseItem(world, i, j, k, random);
}

TileEntity *BlockDispenser::getBlockEntity() { return new TileEntityDispenser(); }

void BlockDispenser::onBlockPlacedBy(World *world, int_t i, int_t j, int_t k, EntityLiving *entityliving)
{
    int_t l = MathHelper::floor_double((double)((entityliving->rotationYaw * 4.0f) / 360.0f) + 0.5) & 3;
    if (l == 0) world->setBlockMetadataWithNotify(i, j, k, 2);
    if (l == 1) world->setBlockMetadataWithNotify(i, j, k, 5);
    if (l == 2) world->setBlockMetadataWithNotify(i, j, k, 3);
    if (l == 3) world->setBlockMetadataWithNotify(i, j, k, 4);
}

void BlockDispenser::onBlockRemoval(World *world, int_t i, int_t j, int_t k)
{
    TileEntityDispenser *tileentitydispenser = (TileEntityDispenser*)world->getBlockTileEntity(i, j, k);
    for (int_t l = 0; l < tileentitydispenser->getSizeInventory(); l++)
    {
        ItemStack *itemstack = tileentitydispenser->getStackInSlot(l);
        if (itemstack == nullptr) continue;
        float f  = random.nextFloat() * 0.8f + 0.1f;
        float f1 = random.nextFloat() * 0.8f + 0.1f;
        float f2 = random.nextFloat() * 0.8f + 0.1f;
        while (itemstack->stackSize > 0)
        {
            int_t i1 = random.nextInt(21) + 10;
            if (i1 > itemstack->stackSize) i1 = itemstack->stackSize;
            itemstack->stackSize -= i1;
            EntityItem *entityitem = new EntityItem(world,
                (float)i + f, (float)j + f1, (float)k + f2,
                new ItemStack(itemstack->itemID, i1, itemstack->getItemDamage()));
            float f3 = 0.05f;
            entityitem->motionX = (float)random.nextGaussian() * f3;
            entityitem->motionY = (float)random.nextGaussian() * f3 + 0.2f;
            entityitem->motionZ = (float)random.nextGaussian() * f3;
            world->entityJoinedWorld(entityitem);
        }
    }
    BlockContainer::onBlockRemoval(world, i, j, k);
}
