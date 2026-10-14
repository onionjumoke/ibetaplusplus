#include "ItemDoor.h"
#include "ItemStack.h"
#include "Material.h"
#include "Block.h"
#include "EntityPlayer.h"
#include "MathHelper.h"
#include "World.h"

ItemDoor::ItemDoor(int i, Material* material)
    : Item(i), doorMaterial(material) {
    maxStackSize = 1;
}

bool ItemDoor::onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
    if (l != 1) {
        return false;
    }
    j++;
    Block* block;
    if (doorMaterial == Material::wood) {
        block = Block::doorWood;
    } else {
        block = Block::doorSteel;
    }
    if (!block->canPlaceBlockAt(world, i, j, k)) {
        return false;
    }
    int i1 = MathHelper::floor_double((double)(((entityplayer->rotationYaw + 180.0f) * 4.0f) / 360.0f) - 0.5) & 3;
    char byte0 = 0;
    char byte1 = 0;
    if (i1 == 0) byte1 = 1;
    if (i1 == 1) byte0 = -1;
    if (i1 == 2) byte1 = -1;
    if (i1 == 3) byte0 = 1;
    int j1 = (world->isBlockNormalCube(i - byte0, j, k - byte1) ? 1 : 0) + (world->isBlockNormalCube(i - byte0, j + 1, k - byte1) ? 1 : 0);
    int k1 = (world->isBlockNormalCube(i + byte0, j, k + byte1) ? 1 : 0) + (world->isBlockNormalCube(i + byte0, j + 1, k + byte1) ? 1 : 0);
    bool flag = world->getBlockId(i - byte0, j, k - byte1) == block->blockID || world->getBlockId(i - byte0, j + 1, k - byte1) == block->blockID;
    bool flag1 = world->getBlockId(i + byte0, j, k + byte1) == block->blockID || world->getBlockId(i + byte0, j + 1, k + byte1) == block->blockID;
    bool flag2 = false;
    if (flag && !flag1) {
        flag2 = true;
    } else if (k1 > j1) {
        flag2 = true;
    }
    if (flag2) {
        i1 = i1 - 1 & 3;
        i1 += 4;
    }
    world->editingBlocks = true;
    world->setBlockAndMetadataWithNotify(i, j, k, block->blockID, i1);
    world->setBlockAndMetadataWithNotify(i, j + 1, k, block->blockID, i1 + 8);
    world->editingBlocks = false;
    world->notifyBlocksOfNeighborChange(i, j, k, block->blockID);
    world->notifyBlocksOfNeighborChange(i, j + 1, k, block->blockID);
    itemstack->stackSize--;
    return true;
}
