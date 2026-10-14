#include "ItemEgg.h"
#include "ItemStack.h"
#include "World.h"
#include "EntityEgg.h"
#include "EntityPlayer.h"

ItemEgg::ItemEgg(int i)
    : Item(i) {
    maxStackSize = 16;
}

ItemStack* ItemEgg::onItemRightClick(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) {
    itemstack->stackSize--;
    world->playSoundAtEntity(entityplayer, "random.bow", 0.5f, 0.4f / (itemRand.nextFloat() * 0.4f + 0.8f));
    if (!world->multiplayerWorld) {
        world->entityJoinedWorld(new EntityEgg(world, entityplayer));
    }
    return itemstack;
}
