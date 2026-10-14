#include "ItemFood.h"
#include "ItemStack.h"
#include "EntityPlayer.h"
#include "World.h"

ItemFood::ItemFood(int i, int j, bool flag)
    : Item(i) {
    healAmount = j;
    isWolfsFavoriteMeat = flag;
    maxStackSize = 1;
}

ItemStack* ItemFood::onItemRightClick(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) {
    itemstack->stackSize--;
    entityplayer->heal(healAmount);
    return itemstack;
}

int ItemFood::getHealAmount() {
    return healAmount;
}

bool ItemFood::getIsWolfsFavoriteMeat() {
    return isWolfsFavoriteMeat;
}
