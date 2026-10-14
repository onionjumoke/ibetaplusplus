#include "ItemSoup.h"
#include "ItemStack.h"
#include "World.h"
#include "EntityPlayer.h"
#include "Item.h"

ItemSoup::ItemSoup(int i, int j)
    : ItemFood(i, j, false) {
}

ItemStack* ItemSoup::onItemRightClick(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) {
    ItemFood::onItemRightClick(itemstack, world, entityplayer);
    return new ItemStack(Item::bowlEmpty);
}
