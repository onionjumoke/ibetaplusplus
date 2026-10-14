#include "ItemBow.h"
#include "ItemStack.h"
#include "EntityPlayer.h"
#include "InventoryPlayer.h"
#include "World.h"
#include "EntityArrow.h"

ItemBow::ItemBow(int i)
    : Item(i) {
    maxStackSize = 1;
}

ItemStack* ItemBow::onItemRightClick(ItemStack* itemstack, World* world, EntityPlayer* entityplayer) {
    if (entityplayer->inventory->consumeInventoryItem(Item::arrow->shiftedIndex)) {
        world->playSoundAtEntity(entityplayer, "random.bow", 1.0f, 1.0f / (itemRand.nextFloat() * 0.4f + 0.8f));
        if (!world->multiplayerWorld) {
            world->entityJoinedWorld(new EntityArrow(world, entityplayer));
        }
    }
    return itemstack;
}
