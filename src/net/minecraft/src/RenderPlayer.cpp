#include "RenderPlayer.h"
#include "EntityPlayer.h"
#include "ModelBiped.h"
#include "ItemStack.h"
#include "Item.h"
#include "Block.h"
#include "RenderBlocks.h"
#include "RenderManager.h"
#include "ItemRenderer.h"
#include "EntityLiving.h"
#include "Entity.h"
#include "FontRenderer.h"
#include "Tessellator.h"
#include "client/Minecraft.h"
#include <SDL2/SDL_opengl.h>
#include "InventoryPlayer.h"
#include "ModelRenderer.h"

RenderPlayer::RenderPlayer() : RenderLiving(new ModelBiped(0.0f), 0.5f) {
    modelBipedMain = (ModelBiped*)mainModel;
    modelArmorChestplate = new ModelBiped(1.0f);
    modelArmor = new ModelBiped(0.5f);
    field_208_f = 0;
}

void RenderPlayer::renderPlayer(EntityPlayer* entityPlayer, double d, double d1, double d2, float f, float f1) {
    ItemStack* itemStack = entityPlayer->inventory->getCurrentItem();
    modelArmorChestplate->isRightArmAiming = modelArmor->isRightArmAiming = modelBipedMain->isRightArmAiming = itemStack != nullptr;
    modelArmorChestplate->isSneak = modelArmor->isSneak = modelBipedMain->isSneak = entityPlayer->isSneaking();
    double d3 = d1 - (double)entityPlayer->yOffset;
    if (entityPlayer->isSneaking()) {
        d3 -= 0.125;
    }
    RenderLiving::doRenderLiving(entityPlayer, d, d3, d2, f, f1);
    modelArmorChestplate->isSneak = modelArmor->isSneak = modelBipedMain->isSneak = false;
    modelArmorChestplate->isRightArmAiming = modelArmor->isRightArmAiming = modelBipedMain->isRightArmAiming = false;
}

void RenderPlayer::drawFirstPersonHand()
{
	modelBipedMain->onGround = 0.0f;
	modelBipedMain->setRotationAngles(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0625f);
	modelBipedMain->bipedRightArm->render(0.0625f);
}

void RenderPlayer::doRenderLiving(EntityLiving* entityLiving, double d, double d1, double d2, float f, float f1) {
    renderPlayer((EntityPlayer*)entityLiving, d, d1, d2, f, f1);
}

void RenderPlayer::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    renderPlayer((EntityPlayer*)entity, d, d1, d2, f, f1);
}

void RenderPlayer::renderName(EntityPlayer* entityPlayer, double d, double d1, double d2) {
    if (!Minecraft::isGuiEnabled() || entityPlayer == renderManager->livingPlayer) {
        return;
    }

    float distance = entityPlayer->getDistanceToEntity(renderManager->livingPlayer);
    float maxDistance = entityPlayer->isSneaking() ? 32.0f : 64.0f;
    if (distance >= maxDistance) {
        return;
    }

    const std::string& name = entityPlayer->username;
    if (!entityPlayer->isSneaking()) {
        renderLivingLabel(entityPlayer, name, d,
                          entityPlayer->isPlayerSleeping() ? d1 - 1.5 : d1,
                          d2, 64);
        return;
    }

    FontRenderer* fontRenderer = getFontRendererFromRenderManager();
    float scale = 0.01666667f * 1.6f;
    glPushMatrix();
    glTranslatef((float)d, (float)d1 + 2.3f, (float)d2);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glRotatef(-renderManager->playerViewY, 0.0f, 1.0f, 0.0f);
    glRotatef(renderManager->playerViewX, 1.0f, 0.0f, 0.0f);
    glScalef(-scale, -scale, scale);
    glDisable(GL_LIGHTING);
    glTranslatef(0.0f, 0.25f / scale, 0.0f);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Tessellator* tessellator = &Tessellator::instance;
    glDisable(GL_TEXTURE_2D);
    tessellator->startDrawingQuads();
    int halfWidth = fontRenderer->getStringWidth(name) / 2;
    tessellator->setColorRGBA_F(0.0f, 0.0f, 0.0f, 0.25f);
    tessellator->addVertex(-halfWidth - 1, -1.0, 0.0);
    tessellator->addVertex(-halfWidth - 1, 8.0, 0.0);
    tessellator->addVertex(halfWidth + 1, 8.0, 0.0);
    tessellator->addVertex(halfWidth + 1, -1.0, 0.0);
    tessellator->draw();
    glEnable(GL_TEXTURE_2D);
    glDepthMask(GL_TRUE);
    fontRenderer->drawString(name, -fontRenderer->getStringWidth(name) / 2, 0, 0x20ffffff);
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glPopMatrix();
}

void RenderPlayer::passSpecialRender(EntityLiving* entityLiving, double d, double d1, double d2) {
    renderName((EntityPlayer*)entityLiving, d, d1, d2);
}

void RenderPlayer::renderSpecials(EntityPlayer* entityPlayer, float f) {
    ItemStack* itemStack = entityPlayer->inventory->getCurrentItem();
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
        renderManager->itemRenderer->renderItem(entityPlayer, itemStack);
        glPopMatrix();
    }
}

void RenderPlayer::renderEquippedItems(EntityLiving* entityLiving, float f) {
    renderSpecials((EntityPlayer*)entityLiving, f);
}

void RenderPlayer::preRenderCallback(EntityLiving* entityLiving, float f) {
    glScalef(0.9375f, 0.9375f, 0.9375f);
}
