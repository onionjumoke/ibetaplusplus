#include "BlockDoor.h"
#include "Material.h"
#include "IBlockAccess.h"
#include "World.h"
#include "AxisAlignedBB.h"
#include "EntityPlayer.h"
#include "Vec3D.h"
#include "MovingObjectPosition.h"
#include "Block.h"
#include "Item.h"

BlockDoor::BlockDoor(int_t i, Material *material) : Block(i, material)
{
    blockIndexInTexture = 97;
    if (material == Material::iron)
        blockIndexInTexture++;
    float f  = 0.5f;
    float f1 = 1.0f;
    setBlockBounds(0.5f - f, 0.0f, 0.5f - f, 0.5f + f, f1, 0.5f + f);
}

int_t BlockDoor::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
    if (i == 0 || i == 1) return blockIndexInTexture;
    int_t k = getState(j);
    if (((k == 0 || k == 2) ^ (i <= 3)))
        return blockIndexInTexture;
    int_t l = k / 2 + ((i & 1) ^ k);
    l += (j & 4) / 4;
    int_t i1 = blockIndexInTexture - (j & 8) * 2;
    if ((l & 1) != 0)
        i1 = -i1;
    return i1;
}

bool  BlockDoor::isOpaqueCube()        { return false; }
bool  BlockDoor::renderAsNormalBlock() { return false; }
int_t BlockDoor::getRenderType()       { return 7; }

AxisAlignedBB *BlockDoor::getSelectedBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
    setBlockBoundsBasedOnState(world, i, j, k);
    return Block::getSelectedBoundingBoxFromPool(world, i, j, k);
}

AxisAlignedBB *BlockDoor::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
    setBlockBoundsBasedOnState(world, i, j, k);
    return Block::getCollisionBoundingBoxFromPool(world, i, j, k);
}

void BlockDoor::setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
    setDoorRotation(getState(iblockaccess->getBlockMetadata(i, j, k)));
}

void BlockDoor::setDoorRotation(int_t i)
{
    float f = 0.1875f;
    setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 2.0f, 1.0f);
    if (i == 0) setBlockBounds(0.0f, 0.0f, 0.0f,       1.0f,     1.0f, f);
    if (i == 1) setBlockBounds(1.0f - f, 0.0f, 0.0f,   1.0f,     1.0f, 1.0f);
    if (i == 2) setBlockBounds(0.0f, 0.0f, 1.0f - f,   1.0f,     1.0f, 1.0f);
    if (i == 3) setBlockBounds(0.0f, 0.0f, 0.0f,       f,        1.0f, 1.0f);
}

void BlockDoor::onBlockClicked(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
    blockActivated(world, i, j, k, entityplayer);
}

bool BlockDoor::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
    if (blockMaterial == Material::iron) return true;
    int_t l = world->getBlockMetadata(i, j, k);
    if ((l & 8) != 0)
    {
        if (world->getBlockId(i, j - 1, k) == blockID)
            blockActivated(world, i, j - 1, k, entityplayer);
        return true;
    }
    if (world->getBlockId(i, j + 1, k) == blockID)
        world->setBlockMetadataWithNotify(i, j + 1, k, (l ^ 4) + 8);
    world->setBlockMetadataWithNotify(i, j, k, l ^ 4);
    world->markBlocksDirty(i, j - 1, k, i, j, k);
    world->getTrapdoorFacing(entityplayer, 1003, i, j, k, 0);
    return true;
}

void BlockDoor::onPoweredBlockChange(World *world, int_t i, int_t j, int_t k, bool flag)
{
    int_t l = world->getBlockMetadata(i, j, k);
    if ((l & 8) != 0)
    {
        if (world->getBlockId(i, j - 1, k) == blockID)
            onPoweredBlockChange(world, i, j - 1, k, flag);
        return;
    }
    bool flag1 = (world->getBlockMetadata(i, j, k) & 4) > 0;
    if (flag1 == flag) return;
    if (world->getBlockId(i, j + 1, k) == blockID)
        world->setBlockMetadataWithNotify(i, j + 1, k, (l ^ 4) + 8);
    world->setBlockMetadataWithNotify(i, j, k, l ^ 4);
    world->markBlocksDirty(i, j - 1, k, i, j, k);
    world->getTrapdoorFacing(nullptr, 1003, i, j, k, 0);
}

void BlockDoor::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
    int_t i1 = world->getBlockMetadata(i, j, k);
    if ((i1 & 8) != 0)
    {
        if (world->getBlockId(i, j - 1, k) != blockID)
            world->setBlockWithNotify(i, j, k, 0);
        if (l > 0 && Block::blocksList[l]->canProvidePower())
            onNeighborBlockChange(world, i, j - 1, k, l);
    }
    else
    {
        bool flag = false;
        if (world->getBlockId(i, j + 1, k) != blockID)
        {
            world->setBlockWithNotify(i, j, k, 0);
            flag = true;
        }
        if (!world->isBlockNormalCube(i, j - 1, k))
        {
            world->setBlockWithNotify(i, j, k, 0);
            flag = true;
            if (world->getBlockId(i, j + 1, k) == blockID)
                world->setBlockWithNotify(i, j + 1, k, 0);
        }
        if (flag)
        {
            if (!world->multiplayerWorld)
                dropBlockAsItem(world, i, j, k, i1);
        }
        else if (l > 0 && Block::blocksList[l]->canProvidePower())
        {
            bool flag1 = world->isBlockIndirectlyGettingPowered(i, j, k) ||
                         world->isBlockIndirectlyGettingPowered(i, j + 1, k);
            onPoweredBlockChange(world, i, j, k, flag1);
        }
    }
}

int_t BlockDoor::idDropped(int_t i, Random &random)
{
    if ((i & 8) != 0) return 0;
    if (blockMaterial == Material::iron) return Item::doorSteel->shiftedIndex;
    return Item::doorWood->shiftedIndex;
}

MovingObjectPosition *BlockDoor::collisionRayTrace(World *world, int_t i, int_t j, int_t k, Vec3D *vec3d, Vec3D *vec3d1)
{
    setBlockBoundsBasedOnState(world, i, j, k);
    return Block::collisionRayTrace(world, i, j, k, vec3d, vec3d1);
}

int_t BlockDoor::getState(int_t i)
{
    if ((i & 4) == 0) return (i - 1) & 3;
    return i & 3;
}

bool BlockDoor::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
    if (j >= 127) return false;
    return world->isBlockNormalCube(i, j - 1, k) &&
           Block::canPlaceBlockAt(world, i, j, k) &&
           Block::canPlaceBlockAt(world, i, j + 1, k);
}

bool BlockDoor::isOpen(int_t i)
{
    return (i & 4) != 0;
}

int_t BlockDoor::getMobilityFlag() { return 1; }
