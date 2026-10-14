#include "ItemSnowball.h"
#include "ItemStack.h"
#include "World.h"
#include "EntityPlayer.h"
#include "EntitySnowball.h"

ItemSnowball::ItemSnowball(int i)
    : Item(i) {
    maxStackSize = 16;
}

ItemStack* ItemSnowball::onItemRightClick(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) {
    itemstack->stackSize--;
    world->playSoundAtEntity(entityplayer, "random.bow", 0.5f, 0.4f / (itemRand.nextFloat() * 0.4f + 0.8f));
    if (!world->multiplayerWorld) {
        world->entityJoinedWorld(new EntitySnowball(world, entityplayer));
    }
    return itemstack;
}
