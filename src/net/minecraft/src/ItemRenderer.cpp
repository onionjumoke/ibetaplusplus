#include "ItemRenderer.h"
#include "Minecraft.h"
#include "ItemStack.h"
#include "Item.h"
#include "Block.h"
#include "Material.h"
#include "MathHelper.h"
#include "RenderBlocks.h"
#include "MapItemRenderer.h"
#include "Tessellator.h"
#include "RenderHelper.h"
#include "RenderManager.h"
#include "RenderPlayer.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "EntityPlayerSP.h"
#include "MapData.h"
#include "ItemMap.h"
#include "InventoryPlayer.h"
#include "pc/OpenGL.h"

ItemRenderer::ItemRenderer(Minecraft* minecraft)
    : itemToRender(nullptr)
    , equippedProgress(0.0f)
    , prevEquippedProgress(0.0f)
    , field_20099_f(-1) {
    renderBlocksInstance = new RenderBlocks();
    mc = minecraft;
    field_28131_f = new MapItemRenderer(minecraft->fontRenderer, minecraft->gameSettings, minecraft->renderEngine);
}

void ItemRenderer::renderItem(EntityLiving* entityliving, ItemStack* itemstack) {
    glPushMatrix();
    if (itemstack->itemID < 256 && RenderBlocks::renderItemIn3d(Block::blocksList[itemstack->itemID]->getRenderType())) {
        glBindTexture(GL_TEXTURE_2D, mc->renderEngine->getTexture("/terrain.png"));
        renderBlocksInstance->renderBlockOnInventory(Block::blocksList[itemstack->itemID], itemstack->getItemDamage(), entityliving->getEntityBrightness(1.0f));
    } else {
        if (itemstack->itemID < 256) {
            glBindTexture(GL_TEXTURE_2D, mc->renderEngine->getTexture("/terrain.png"));
        } else {
            glBindTexture(GL_TEXTURE_2D, mc->renderEngine->getTexture("/gui/items.png"));
        }
        Tessellator* tessellator = &Tessellator::instance;
        int i = entityliving->getItemIcon(itemstack);
        float f  = ((float)((i % 16) * 16) + 0.0f) / 256.0f;
        float f1 = ((float)((i % 16) * 16) + 15.99f) / 256.0f;
        float f2 = ((float)((i / 16) * 16) + 0.0f) / 256.0f;
        float f3 = ((float)((i / 16) * 16) + 15.99f) / 256.0f;
        float f4 = 1.0f;
        float f5 = 0.0f;
        float f6 = 0.3f;
        glEnable(0x803A /* GL_RESCALE_NORMAL_EXT */);
        glTranslatef(-f5, -f6, 0.0f);
        float f7 = 1.5f;
        glScalef(f7, f7, f7);
        glRotatef(50.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(335.0f, 0.0f, 0.0f, 1.0f);
        glTranslatef(-0.9375f, -0.0625f, 0.0f);
        float f8 = 0.0625f;
        tessellator->startDrawingQuads();
        tessellator->setNormal(0.0f, 0.0f, 1.0f);
        tessellator->addVertexWithUV(0.0, 0.0, 0.0, f1, f3);
        tessellator->addVertexWithUV(f4, 0.0, 0.0, f, f3);
        tessellator->addVertexWithUV(f4, 1.0, 0.0, f, f2);
        tessellator->addVertexWithUV(0.0, 1.0, 0.0, f1, f2);
        tessellator->draw();
        tessellator->startDrawingQuads();
        tessellator->setNormal(0.0f, 0.0f, -1.0f);
        tessellator->addVertexWithUV(0.0, 1.0, 0.0f - f8, f1, f2);
        tessellator->addVertexWithUV(f4, 1.0, 0.0f - f8, f, f2);
        tessellator->addVertexWithUV(f4, 0.0, 0.0f - f8, f, f3);
        tessellator->addVertexWithUV(0.0, 0.0, 0.0f - f8, f1, f3);
        tessellator->draw();
        tessellator->startDrawingQuads();
        tessellator->setNormal(-1.0f, 0.0f, 0.0f);
        for (int j = 0; j < 16; j++) {
            float f9  = (float)j / 16.0f;
            float f13 = (f1 + (f - f1) * f9) - 0.001953125f;
            float f17 = f4 * f9;
            tessellator->addVertexWithUV(f17, 0.0, 0.0f - f8, f13, f3);
            tessellator->addVertexWithUV(f17, 0.0, 0.0,       f13, f3);
            tessellator->addVertexWithUV(f17, 1.0, 0.0,       f13, f2);
            tessellator->addVertexWithUV(f17, 1.0, 0.0f - f8, f13, f2);
        }
        tessellator->draw();
        tessellator->startDrawingQuads();
        tessellator->setNormal(1.0f, 0.0f, 0.0f);
        for (int k = 0; k < 16; k++) {
            float f10 = (float)k / 16.0f;
            float f14 = (f1 + (f - f1) * f10) - 0.001953125f;
            float f18 = f4 * f10 + 0.0625f;
            tessellator->addVertexWithUV(f18, 1.0, 0.0f - f8, f14, f2);
            tessellator->addVertexWithUV(f18, 1.0, 0.0,       f14, f2);
            tessellator->addVertexWithUV(f18, 0.0, 0.0,       f14, f3);
            tessellator->addVertexWithUV(f18, 0.0, 0.0f - f8, f14, f3);
        }
        tessellator->draw();
        tessellator->startDrawingQuads();
        tessellator->setNormal(0.0f, 1.0f, 0.0f);
        for (int l = 0; l < 16; l++) {
            float f11 = (float)l / 16.0f;
            float f15 = (f3 + (f2 - f3) * f11) - 0.001953125f;
            float f19 = f4 * f11 + 0.0625f;
            tessellator->addVertexWithUV(0.0, f19, 0.0,       f1, f15);
            tessellator->addVertexWithUV(f4,  f19, 0.0,       f,  f15);
            tessellator->addVertexWithUV(f4,  f19, 0.0f - f8, f,  f15);
            tessellator->addVertexWithUV(0.0, f19, 0.0f - f8, f1, f15);
        }
        tessellator->draw();
        tessellator->startDrawingQuads();
        tessellator->setNormal(0.0f, -1.0f, 0.0f);
        for (int i1 = 0; i1 < 16; i1++) {
            float f12 = (float)i1 / 16.0f;
            float f16 = (f3 + (f2 - f3) * f12) - 0.001953125f;
            float f20 = f4 * f12;
            tessellator->addVertexWithUV(f4,  f20, 0.0,       f,  f16);
            tessellator->addVertexWithUV(0.0, f20, 0.0,       f1, f16);
            tessellator->addVertexWithUV(0.0, f20, 0.0f - f8, f1, f16);
            tessellator->addVertexWithUV(f4,  f20, 0.0f - f8, f,  f16);
        }
        tessellator->draw();
        glDisable(0x803A /* GL_RESCALE_NORMAL_EXT */);
    }
    glPopMatrix();
}

void ItemRenderer::renderItemInFirstPerson(float f) {
    float f1 = prevEquippedProgress + (equippedProgress - prevEquippedProgress) * f;
    EntityPlayerSP* entityplayersp = mc->thePlayer;
    EntityPlayer* entityplayer = (EntityPlayer*)entityplayersp;
    float f2 = entityplayer->prevRotationPitch + (entityplayer->rotationPitch - entityplayer->prevRotationPitch) * f;
    glPushMatrix();
    glRotatef(f2, 1.0f, 0.0f, 0.0f);
    glRotatef(entityplayer->prevRotationYaw + (entityplayer->rotationYaw - entityplayer->prevRotationYaw) * f, 0.0f, 1.0f, 0.0f);
    RenderHelper::enableStandardItemLighting();
    glPopMatrix();
    ItemStack* itemstack = itemToRender;
    float f3 = mc->theWorld->getLightBrightness(
        MathHelper::floor_double(entityplayer->posX),
        MathHelper::floor_double(entityplayer->posY),
        MathHelper::floor_double(entityplayer->posZ));
    if (itemstack != nullptr) {
        int i = Item::itemsList[itemstack->itemID]->getColorFromDamage(itemstack->getItemDamage());
        float f7  = (float)(i >> 16 & 0xff) / 255.0f;
        float f11 = (float)(i >> 8  & 0xff) / 255.0f;
        float f15 = (float)(i       & 0xff) / 255.0f;
        glColor4f(f3 * f7, f3 * f11, f3 * f15, 1.0f);
    } else {
        glColor4f(f3, f3, f3, 1.0f);
    }
    if (itemstack != nullptr && itemstack->itemID == Item::mapItem->shiftedIndex) {
        glPushMatrix();
        float f4  = 0.8f;
        float f8  = entityplayersp->getSwingProgress(f);
        float f12 = MathHelper::sin(f8 * 3.141593f);
        float f16 = MathHelper::sin(MathHelper::sqrt_float(f8) * 3.141593f);
        glTranslatef(-f16 * 0.4f, MathHelper::sin(MathHelper::sqrt_float(f8) * 3.141593f * 2.0f) * 0.2f, -f12 * 0.2f);
        f8 = (1.0f - f2 / 45.0f) + 0.1f;
        if (f8 < 0.0f) f8 = 0.0f;
        if (f8 > 1.0f) f8 = 1.0f;
        f8 = -MathHelper::cos(f8 * 3.141593f) * 0.5f + 0.5f;
        glTranslatef(0.0f, (0.0f * f4 - (1.0f - f1) * 1.2f - f8 * 0.5f) + 0.04f, -0.9f * f4);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(f8 * -85.0f, 0.0f, 0.0f, 1.0f);
        glEnable(0x803A /* GL_RESCALE_NORMAL_EXT */);
        glBindTexture(GL_TEXTURE_2D, mc->renderEngine->getTextureForDownloadableImage(mc->thePlayer->skinUrl, mc->thePlayer->getEntityTexture()));
        for (float f_arm = 0; f_arm < 2; f_arm++) {
            float f_side = f_arm * 2 - 1;
            glPushMatrix();
            glTranslatef(-0.0f, -0.6f, 1.1f * f_side);
            glRotatef(-45.0f * f_side, 1.0f, 0.0f, 0.0f);
            glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
            glRotatef(59.0f, 0.0f, 0.0f, 1.0f);
            glRotatef(-65.0f * f_side, 0.0f, 1.0f, 0.0f);
            Render* render1 = RenderManager::instance->getEntityRenderObject(mc->thePlayer);
            RenderPlayer* renderplayer1 = (RenderPlayer*)render1;
            float f20 = 1.0f;
            glScalef(f20, f20, f20);
            renderplayer1->drawFirstPersonHand();
            glPopMatrix();
        }
        f12 = entityplayersp->getSwingProgress(f);
        f16 = MathHelper::sin(f12 * f12 * 3.141593f);
        float f19 = MathHelper::sin(MathHelper::sqrt_float(f12) * 3.141593f);
        glRotatef(-f16 * 20.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(-f19 * 20.0f, 0.0f, 0.0f, 1.0f);
        glRotatef(-f19 * 80.0f, 1.0f, 0.0f, 0.0f);
        f12 = 0.38f;
        glScalef(f12, f12, f12);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
        glTranslatef(-1.0f, -1.0f, 0.0f);
        f16 = 0.015625f;
        glScalef(f16, f16, f16);
        mc->renderEngine->bindTexture(mc->renderEngine->getTexture("/misc/mapbg.png"));
        Tessellator* tessellator = &Tessellator::instance;
        glNormal3f(0.0f, 0.0f, -1.0f);
        tessellator->startDrawingQuads();
        int byte0 = 7;
        tessellator->addVertexWithUV(0 - byte0, 128 + byte0, 0.0, 0.0, 1.0);
        tessellator->addVertexWithUV(128 + byte0, 128 + byte0, 0.0, 1.0, 1.0);
        tessellator->addVertexWithUV(128 + byte0, 0 - byte0, 0.0, 1.0, 0.0);
        tessellator->addVertexWithUV(0 - byte0, 0 - byte0, 0.0, 0.0, 0.0);
        tessellator->draw();
        MapData* mapdata = static_cast<ItemMap*>(Item::mapItem)->getAverageGroundLevel(itemstack, mc->theWorld);
        field_28131_f->renderMap(mc->thePlayer, mc->renderEngine, mapdata);
        glPopMatrix();
    } else if (itemstack != nullptr) {
        glPushMatrix();
        float f5  = 0.8f;
        float f9  = entityplayersp->getSwingProgress(f);
        float f13 = MathHelper::sin(f9 * 3.141593f);
        float f17 = MathHelper::sin(MathHelper::sqrt_float(f9) * 3.141593f);
        glTranslatef(-f17 * 0.4f, MathHelper::sin(MathHelper::sqrt_float(f9) * 3.141593f * 2.0f) * 0.2f, -f13 * 0.2f);
        glTranslatef(0.7f * f5, -0.65f * f5 - (1.0f - f1) * 0.6f, -0.9f * f5);
        glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
        glEnable(0x803A /* GL_RESCALE_NORMAL_EXT */);
        f9  = entityplayersp->getSwingProgress(f);
        f13 = MathHelper::sin(f9 * f9 * 3.141593f);
        f17 = MathHelper::sin(MathHelper::sqrt_float(f9) * 3.141593f);
        glRotatef(-f13 * 20.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(-f17 * 20.0f, 0.0f, 0.0f, 1.0f);
        glRotatef(-f17 * 80.0f, 1.0f, 0.0f, 0.0f);
        f9 = 0.4f;
        glScalef(f9, f9, f9);
        if (itemstack->getItem()->shouldRotateAroundWhenRendering()) {
            glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
        }
        renderItem(entityplayersp, itemstack);
        glPopMatrix();
    } else {
        glPushMatrix();
        float f6  = 0.8f;
        float f10 = entityplayersp->getSwingProgress(f);
        float f14 = MathHelper::sin(f10 * 3.141593f);
        float f18 = MathHelper::sin(MathHelper::sqrt_float(f10) * 3.141593f);
        glTranslatef(-f18 * 0.3f, MathHelper::sin(MathHelper::sqrt_float(f10) * 3.141593f * 2.0f) * 0.4f, -f14 * 0.4f);
        glTranslatef(0.8f * f6, -0.75f * f6 - (1.0f - f1) * 0.6f, -0.9f * f6);
        glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
        glEnable(0x803A /* GL_RESCALE_NORMAL_EXT */);
        f10 = entityplayersp->getSwingProgress(f);
        f14 = MathHelper::sin(f10 * f10 * 3.141593f);
        f18 = MathHelper::sin(MathHelper::sqrt_float(f10) * 3.141593f);
        glRotatef(f18 * 70.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(-f14 * 20.0f, 0.0f, 0.0f, 1.0f);
        glBindTexture(GL_TEXTURE_2D, mc->renderEngine->getTextureForDownloadableImage(mc->thePlayer->skinUrl, mc->thePlayer->getEntityTexture()));
        glTranslatef(-1.0f, 3.6f, 3.5f);
        glRotatef(120.0f, 0.0f, 0.0f, 1.0f);
        glRotatef(200.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(-135.0f, 0.0f, 1.0f, 0.0f);
        glScalef(1.0f, 1.0f, 1.0f);
        glTranslatef(5.6f, 0.0f, 0.0f);
        Render* render = RenderManager::instance->getEntityRenderObject(mc->thePlayer);
        RenderPlayer* renderplayer = (RenderPlayer*)render;
        float f18s = 1.0f;
        glScalef(f18s, f18s, f18s);
        renderplayer->drawFirstPersonHand();
        glPopMatrix();
    }
    glDisable(0x803A /* GL_RESCALE_NORMAL_EXT */);
    RenderHelper::disableStandardItemLighting();
}

void ItemRenderer::renderOverlays(float f) {
    glDisable(GL_ALPHA_TEST);
    if (mc->thePlayer->isBurning()) {
        int i = mc->renderEngine->getTexture("/terrain.png");
        glBindTexture(GL_TEXTURE_2D, i);
        renderFireInFirstPerson(f);
    }
    if (mc->thePlayer->isEntityInsideOpaqueBlock()) {
        int j  = MathHelper::floor_double(mc->thePlayer->posX);
        int l  = MathHelper::floor_double(mc->thePlayer->posY);
        int i1 = MathHelper::floor_double(mc->thePlayer->posZ);
        int j1 = mc->renderEngine->getTexture("/terrain.png");
        glBindTexture(GL_TEXTURE_2D, j1);
        int k1 = mc->theWorld->getBlockId(j, l, i1);
        if (mc->theWorld->isBlockNormalCube(j, l, i1)) {
            renderInsideOfBlock(f, Block::blocksList[k1]->getBlockTextureFromSide(2));
        } else {
            for (int l1 = 0; l1 < 8; l1++) {
                float f1 = ((float)((l1 >> 0) % 2) - 0.5f) * mc->thePlayer->width * 0.9f;
                float f2 = ((float)((l1 >> 1) % 2) - 0.5f) * mc->thePlayer->height * 0.2f;
                float f3 = ((float)((l1 >> 2) % 2) - 0.5f) * mc->thePlayer->width * 0.9f;
                int i2 = MathHelper::floor_float((float)j + f1);
                int j2 = MathHelper::floor_float((float)l + f2);
                int k2 = MathHelper::floor_float((float)i1 + f3);
                if (mc->theWorld->isBlockNormalCube(i2, j2, k2)) {
                    k1 = mc->theWorld->getBlockId(i2, j2, k2);
                }
            }
        }
        if (Block::blocksList[k1] != nullptr) {
            renderInsideOfBlock(f, Block::blocksList[k1]->getBlockTextureFromSide(2));
        }
    }
    if (mc->thePlayer->isInsideOfMaterial(Material::water)) {
        int k = mc->renderEngine->getTexture("/misc/water.png");
        glBindTexture(GL_TEXTURE_2D, k);
        renderWarpedTextureOverlay(f);
    }
    glEnable(GL_ALPHA_TEST);
}

void ItemRenderer::renderInsideOfBlock(float f, int i) {
    Tessellator* tessellator = &Tessellator::instance;
    float f1 = mc->thePlayer->getEntityBrightness(f);
    f1 = 0.1f;
    glColor4f(f1, f1, f1, 0.5f);
    glPushMatrix();
    float f2 = -1.0f;
    float f3 =  1.0f;
    float f4 = -1.0f;
    float f5 =  1.0f;
    float f6 = -0.5f;
    float f7 = 0.0078125f;
    float f8  = (float)(i % 16) / 256.0f - f7;
    float f9  = ((float)(i % 16) + 15.99f) / 256.0f + f7;
    float f10 = (float)(i / 16) / 256.0f - f7;
    float f11 = ((float)(i / 16) + 15.99f) / 256.0f + f7;
    tessellator->startDrawingQuads();
    tessellator->addVertexWithUV(f2, f4, f6, f9, f11);
    tessellator->addVertexWithUV(f3, f4, f6, f8, f11);
    tessellator->addVertexWithUV(f3, f5, f6, f8, f10);
    tessellator->addVertexWithUV(f2, f5, f6, f9, f10);
    tessellator->draw();
    glPopMatrix();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void ItemRenderer::renderWarpedTextureOverlay(float f) {
    Tessellator* tessellator = &Tessellator::instance;
    float f1 = mc->thePlayer->getEntityBrightness(f);
    glColor4f(f1, f1, f1, 0.5f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPushMatrix();
    float f2 =  4.0f;
    float f3 = -1.0f;
    float f4 =  1.0f;
    float f5 = -1.0f;
    float f6 =  1.0f;
    float f7 = -0.5f;
    float f8 = -mc->thePlayer->rotationYaw / 64.0f;
    float f9 =  mc->thePlayer->rotationPitch / 64.0f;
    tessellator->startDrawingQuads();
    tessellator->addVertexWithUV(f3, f5, f7, f2 + f8, f2 + f9);
    tessellator->addVertexWithUV(f4, f5, f7, 0.0 + f8, f2 + f9);
    tessellator->addVertexWithUV(f4, f6, f7, 0.0 + f8, 0.0 + f9);
    tessellator->addVertexWithUV(f3, f6, f7, f2 + f8, 0.0 + f9);
    tessellator->draw();
    glPopMatrix();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDisable(GL_BLEND);
}

void ItemRenderer::renderFireInFirstPerson(float f) {
    Tessellator* tessellator = &Tessellator::instance;
    glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float f1 = 1.0f;
    for (int i = 0; i < 2; i++) {
        glPushMatrix();
        int j = Block::fire->blockIndexInTexture + i * 16;
        int k = (j & 0xf) << 4;
        int l = j & 0xf0;
        float f2 = (float)k / 256.0f;
        float f3 = ((float)k + 15.99f) / 256.0f;
        float f4 = (float)l / 256.0f;
        float f5 = ((float)l + 15.99f) / 256.0f;
        float f6 = (0.0f - f1) / 2.0f;
        float f7 = f6 + f1;
        float f8 = 0.0f - f1 / 2.0f;
        float f9 = f8 + f1;
        float f10 = -0.5f;
        glTranslatef((float)(-(i * 2 - 1)) * 0.24f, -0.3f, 0.0f);
        glRotatef((float)(i * 2 - 1) * 10.0f, 0.0f, 1.0f, 0.0f);
        tessellator->startDrawingQuads();
        tessellator->addVertexWithUV(f6, f8, f10, f3, f5);
        tessellator->addVertexWithUV(f7, f8, f10, f2, f5);
        tessellator->addVertexWithUV(f7, f9, f10, f2, f4);
        tessellator->addVertexWithUV(f6, f9, f10, f3, f4);
        tessellator->draw();
        glPopMatrix();
    }
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDisable(GL_BLEND);
}

void ItemRenderer::updateEquippedItem() {
    prevEquippedProgress = equippedProgress;
    EntityPlayerSP* entityplayersp = mc->thePlayer;
    EntityPlayer* entityplayer = (EntityPlayer*)entityplayersp;
    ItemStack* itemstack1 = entityplayer->inventory->getCurrentItem();
    bool flag = field_20099_f == entityplayer->inventory->currentItem && itemstack1 == itemToRender;
    if (itemToRender == nullptr && itemstack1 == nullptr) {
        flag = true;
    }
    if (itemstack1 != nullptr && itemToRender != nullptr && itemstack1 != itemToRender &&
        itemstack1->itemID == itemToRender->itemID && itemstack1->getItemDamage() == itemToRender->getItemDamage()) {
        itemToRender = itemstack1;
        flag = true;
    }
    float f = 0.4f;
    float f1 = flag ? 1.0f : 0.0f;
    float f2 = f1 - equippedProgress;
    if (f2 < -f) f2 = -f;
    if (f2 >  f) f2 =  f;
    equippedProgress += f2;
    if (equippedProgress < 0.1f) {
        itemToRender = itemstack1;
        field_20099_f = entityplayer->inventory->currentItem;
    }
}

void ItemRenderer::resetEquippedProgress() {
    equippedProgress = 0.0f;
}

void ItemRenderer::func_9449_b() {
    equippedProgress = 0.0f;
}

void ItemRenderer::func_9450_c() {
    equippedProgress = 0.0f;
}
