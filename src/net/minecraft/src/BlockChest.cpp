#include "BlockChest.h"
#include "Material.h"
#include "IBlockAccess.h"
#include "Block.h"
#include "World.h"
#include "TileEntityChest.h"
#include "IInventory.h"
#include "ItemStack.h"
#include "EntityItem.h"
#include "InventoryLargeChest.h"
#include "EntityPlayer.h"
#include "TileEntity.h"

BlockChest::BlockChest(int_t i) : BlockContainer(i, Material::wood)
{
    blockIndexInTexture = 26;
}

int_t BlockChest::getBlockTexture(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
    if (l == 1) return blockIndexInTexture - 1;
    if (l == 0) return blockIndexInTexture - 1;
    int_t i1 = iblockaccess->getBlockId(i, j, k - 1);
    int_t j1 = iblockaccess->getBlockId(i, j, k + 1);
    int_t k1 = iblockaccess->getBlockId(i - 1, j, k);
    int_t l1 = iblockaccess->getBlockId(i + 1, j, k);
    if (i1 == blockID || j1 == blockID)
    {
        if (l == 2 || l == 3) return blockIndexInTexture;
        int_t i2 = 0;
        if (i1 == blockID) i2 = -1;
        int_t k2 = iblockaccess->getBlockId(i - 1, j, i1 != blockID ? k + 1 : k - 1);
        int_t i3 = iblockaccess->getBlockId(i + 1, j, i1 != blockID ? k + 1 : k - 1);
        if (l == 4) i2 = -1 - i2;
        byte_t byte1 = 5;
        if ((Block::opaqueCubeLookup[k1] || Block::opaqueCubeLookup[k2]) && !Block::opaqueCubeLookup[l1] && !Block::opaqueCubeLookup[i3]) byte1 = 5;
        if ((Block::opaqueCubeLookup[l1] || Block::opaqueCubeLookup[i3]) && !Block::opaqueCubeLookup[k1] && !Block::opaqueCubeLookup[k2]) byte1 = 4;
        return (l != byte1 ? blockIndexInTexture + 32 : blockIndexInTexture + 16) + i2;
    }
    if (k1 == blockID || l1 == blockID)
    {
        if (l == 4 || l == 5) return blockIndexInTexture;
        int_t j2 = 0;
        if (k1 == blockID) j2 = -1;
        int_t l2 = iblockaccess->getBlockId(k1 != blockID ? i + 1 : i - 1, j, k - 1);
        int_t j3 = iblockaccess->getBlockId(k1 != blockID ? i + 1 : i - 1, j, k + 1);
        if (l == 3) j2 = -1 - j2;
        byte_t byte2 = 3;
        if ((Block::opaqueCubeLookup[i1] || Block::opaqueCubeLookup[l2]) && !Block::opaqueCubeLookup[j1] && !Block::opaqueCubeLookup[j3]) byte2 = 3;
        if ((Block::opaqueCubeLookup[j1] || Block::opaqueCubeLookup[j3]) && !Block::opaqueCubeLookup[i1] && !Block::opaqueCubeLookup[l2]) byte2 = 2;
        return (l != byte2 ? blockIndexInTexture + 32 : blockIndexInTexture + 16) + j2;
    }
    byte_t byte0 = 3;
    if (Block::opaqueCubeLookup[i1] && !Block::opaqueCubeLookup[j1])  byte0 = 3;
    if (Block::opaqueCubeLookup[j1] && !Block::opaqueCubeLookup[i1])  byte0 = 2;
    if (Block::opaqueCubeLookup[k1] && !Block::opaqueCubeLookup[l1])  byte0 = 5;
    if (Block::opaqueCubeLookup[l1] && !Block::opaqueCubeLookup[k1])  byte0 = 4;
    return l != byte0 ? blockIndexInTexture : blockIndexInTexture + 1;
}

int_t BlockChest::getBlockTextureFromSide(int_t i)
{
    if (i == 1) return blockIndexInTexture - 1;
    if (i == 0) return blockIndexInTexture - 1;
    if (i == 3) return blockIndexInTexture + 1;
    return blockIndexInTexture;
}

bool BlockChest::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
    int_t l = 0;
    if (world->getBlockId(i - 1, j, k) == blockID) l++;
    if (world->getBlockId(i + 1, j, k) == blockID) l++;
    if (world->getBlockId(i, j, k - 1) == blockID) l++;
    if (world->getBlockId(i, j, k + 1) == blockID) l++;
    if (l > 1) return false;
    if (isThereANeighborChest(world, i - 1, j, k)) return false;
    if (isThereANeighborChest(world, i + 1, j, k)) return false;
    if (isThereANeighborChest(world, i, j, k - 1)) return false;
    return !isThereANeighborChest(world, i, j, k + 1);
}

bool BlockChest::isThereANeighborChest(World *world, int_t i, int_t j, int_t k)
{
    if (world->getBlockId(i, j, k) != blockID) return false;
    if (world->getBlockId(i - 1, j, k) == blockID) return true;
    if (world->getBlockId(i + 1, j, k) == blockID) return true;
    if (world->getBlockId(i, j, k - 1) == blockID) return true;
    return world->getBlockId(i, j, k + 1) == blockID;
}

void BlockChest::onBlockRemoval(World *world, int_t i, int_t j, int_t k)
{
    TileEntityChest *tileentitychest = (TileEntityChest*)world->getBlockTileEntity(i, j, k);
    for (int_t l = 0; l < tileentitychest->getSizeInventory(); l++)
    {
        ItemStack *itemstack = tileentitychest->getStackInSlot(l);
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

bool BlockChest::blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer)
{
    IInventory *obj = (TileEntityChest*)world->getBlockTileEntity(i, j, k);
    if (world->isBlockNormalCube(i, j + 1, k)) return true;
    if (world->getBlockId(i - 1, j, k) == blockID && world->isBlockNormalCube(i - 1, j + 1, k)) return true;
    if (world->getBlockId(i + 1, j, k) == blockID && world->isBlockNormalCube(i + 1, j + 1, k)) return true;
    if (world->getBlockId(i, j, k - 1) == blockID && world->isBlockNormalCube(i, j + 1, k - 1)) return true;
    if (world->getBlockId(i, j, k + 1) == blockID && world->isBlockNormalCube(i, j + 1, k + 1)) return true;
    if (world->getBlockId(i - 1, j, k) == blockID)
        obj = new InventoryLargeChest(std::string("Large chest"), (TileEntityChest*)world->getBlockTileEntity(i - 1, j, k), obj);
    if (world->getBlockId(i + 1, j, k) == blockID)
        obj = new InventoryLargeChest(std::string("Large chest"), obj, (TileEntityChest*)world->getBlockTileEntity(i + 1, j, k));
    if (world->getBlockId(i, j, k - 1) == blockID)
        obj = new InventoryLargeChest(std::string("Large chest"), (TileEntityChest*)world->getBlockTileEntity(i, j, k - 1), obj);
    if (world->getBlockId(i, j, k + 1) == blockID)
        obj = new InventoryLargeChest(std::string("Large chest"), obj, (TileEntityChest*)world->getBlockTileEntity(i, j, k + 1));
    if (world->multiplayerWorld) return true;
    entityplayer->displayGUIChest(obj);
    return true;
}

TileEntity *BlockChest::getBlockEntity() { return new TileEntityChest(); }
