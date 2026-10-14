#include "RenderBiped.h"
#include "ModelBiped.h"
#include "EntityLiving.h"
#include "ItemStack.h"
#include "Block.h"
#include "RenderBlocks.h"
#include "Item.h"
#include "RenderManager.h"
#include "ItemRenderer.h"
#include <SDL2/SDL_opengl.h>
#include "ModelRenderer.h"

RenderBiped::RenderBiped(ModelBiped* modelBiped, float f) : RenderLiving(modelBiped, f) {
    modelBipedMain = modelBiped;
}

void RenderBiped::renderEquippedItems(EntityLiving* entityLiving, float f) {
    ItemStack* itemStack = entityLiving->getHeldItem();
    if (itemStack != nullptr) {
        glPushMatrix();
        modelBipedMain->bipedRightArm->postRender(0.0625f);
        glTranslatef(-0.0625f, 0.4375f, 0.0625f);

        if (itemStack->itemID < 256 && RenderBlocks::renderItemIn3d(Block::blocksList[itemStack->itemID]->getRenderType())) {
            float f1 = 0.5f;
            glTranslatef(0.0f, 0.1875f, -0.3125f);
            f1 *= 0.75f;
            glRotatef(20.0f, 1.0f, 0.0f, 0.0f);
            glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
            glScalef(f1, -f1, f1);
        } else if (Item::itemsList[itemStack->itemID]->isFull3D()) {
            float f2 = 0.625f;
            glTranslatef(0.0f, 0.1875f, 0.0f);
            glScalef(f2, -f2, f2);
            glRotatef(-100.0f, 1.0f, 0.0f, 0.0f);
            glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
        } else {
            float f3 = 0.375f;
            glTranslatef(0.25f, 0.1875f, -0.1875f);
            glScalef(f3, f3, f3);
            glRotatef(60.0f, 0.0f, 0.0f, 1.0f);
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            glRotatef(20.0f, 0.0f, 0.0f, 1.0f);
        }

        renderManager->itemRenderer->renderItem(entityLiving, itemStack);
        glPopMatrix();
    }
}