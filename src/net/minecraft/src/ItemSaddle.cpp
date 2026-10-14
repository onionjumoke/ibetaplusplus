#include "ItemSaddle.h"
#include "ItemStack.h"
#include "EntityLiving.h"
#include "EntityPig.h"

ItemSaddle::ItemSaddle(int i)
    : Item(i) {
    maxStackSize = 1;
}

void ItemSaddle::saddleEntity(ItemStack* itemstack, EntityLiving* entityliving) {
    EntityPig* entitypig = dynamic_cast<EntityPig*>(entityliving);
    if (entitypig != nullptr) {
        if (!entitypig->getSaddled()) {
            entitypig->setSaddled(true);
            itemstack->stackSize--;
        }
    }
}

bool ItemSaddle::hitEntity(ItemStack* itemstack, EntityLiving* entityliving, EntityLiving* entityliving1) {
    saddleEntity(itemstack, entityliving);
    return true;
}
