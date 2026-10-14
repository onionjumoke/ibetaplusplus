#include "ItemDye.h"
#include "ItemStack.h"
#include "World.h"
#include "Block.h"
#include "BlockSapling.h"
#include "BlockCrops.h"
#include "BlockGrass.h"
#include "BlockTallGrass.h"
#include "BlockFlower.h"
#include "EntitySheep.h"
#include "BlockCloth.h"
#include "EntityPlayer.h"
#include "EntityLiving.h"

const std::string ItemDye::dyeColors[16] = {
    "black", "red", "green", "brown", "blue", "purple", "cyan", "silver",
    "gray", "pink", "lime", "yellow", "lightBlue", "magenta", "orange", "white"
};

const int ItemDye::worldChunkManager[16] = {
    0x1e1b1b, 0xb3312c, 0x3b511a, 0x51301a, 0x253192, 0x7b2fbe, 0x287697, 0x287697,
    0x434343, 0xd88198, 0x41cd34, 0xdecf2a, 0x6689d3, 0xc354cd, 0xeb8844, 0xf0f0f0
};

ItemDye::ItemDye(int i)
    : Item(i) {
    setHasSubtypes(true);
    setMaxDamage(0);
}

int ItemDye::getIconFromDamage(int i) {
    int j = i;
    return iconIndex + (j % 8) * 16 + j / 8;
}

std::string ItemDye::getItemNameIS(ItemStack* itemstack) {
    return Item::getItemName() + "." + dyeColors[itemstack->getItemDamage()];
}

bool ItemDye::onItemUse(ItemStack* itemstack, EntityPlayer* entityplayer, World* world, int i, int j, int k, int l) {
    if (itemstack->getItemDamage() == 15) {
        int i1 = world->getBlockId(i, j, k);
        if (i1 == Block::sapling->blockID) {
            if (!world->multiplayerWorld) {
                ((BlockSapling*)Block::sapling)->growTree(world, i, j, k, world->rand);
                itemstack->stackSize--;
            }
            return true;
        }
        if (i1 == Block::crops->blockID) {
            if (!world->multiplayerWorld) {
                ((BlockCrops*)Block::crops)->fertilize(world, i, j, k);
                itemstack->stackSize--;
            }
            return true;
        }
        if (i1 == Block::grass->blockID) {
            if (!world->multiplayerWorld) {
                itemstack->stackSize--;
                for (int j1 = 0; j1 < 128; j1++) {
                    int k1 = i;
                    int l1 = j + 1;
                    int i2 = k;
                    for (int j2 = 0; j2 < j1 / 16; j2++) {
                        k1 += itemRand.nextInt(3) - 1;
                        l1 += ((itemRand.nextInt(3) - 1) * itemRand.nextInt(3)) / 2;
                        i2 += itemRand.nextInt(3) - 1;
                        if (world->getBlockId(k1, l1 - 1, i2) != Block::grass->blockID || world->isBlockNormalCube(k1, l1, i2)) {
                            goto label0;
                        }
                    }
                    if (world->getBlockId(k1, l1, i2) != 0) {
                        continue;
                    }
                    if (itemRand.nextInt(10) != 0) {
                        world->setBlockAndMetadataWithNotify(k1, l1, i2, Block::tallGrass->blockID, 1);
                        continue;
                    }
                    if (itemRand.nextInt(3) != 0) {
                        world->setBlockWithNotify(k1, l1, i2, Block::plantYellow->blockID);
                    } else {
                        world->setBlockWithNotify(k1, l1, i2, Block::plantRed->blockID);
                    }
                    label0:;
                }
            }
            return true;
        }
    }
    return false;
}

void ItemDye::saddleEntity(ItemStack* itemstack, EntityLiving* entityliving) {
    EntitySheep* entitysheep = dynamic_cast<EntitySheep*>(entityliving);
    if (entitysheep) {
        int i = BlockCloth::getColorFromDamage(itemstack->getItemDamage());
        if (!entitysheep->getSheared() && entitysheep->getFleeceColor() != i) {
            entitysheep->setFleeceColor(i);
            itemstack->stackSize--;
        }
    }
}
