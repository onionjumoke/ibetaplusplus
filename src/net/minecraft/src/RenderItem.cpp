#include "RenderItem.h"
#include "AxisAlignedBB.h"
#include "EntityItem.h"
#include "ItemStack.h"
#include "Block.h"
#include "Tessellator.h"
#include "Item.h"
#include "RenderManager.h"
#include "RenderEngine.h"
#include "FontRenderer.h"
#include "MathHelper.h"
#include "pc/OpenGL.h"

void RenderItem::renderAABB(AxisAlignedBB *aabb)
{
    if (aabb == nullptr) return;
    double x0 = aabb->minX, y0 = aabb->minY, z0 = aabb->minZ;
    double x1 = aabb->maxX, y1 = aabb->maxY, z1 = aabb->maxZ;

    Tessellator *t = &Tessellator::instance;
    t->startDrawingQuads();
    // -Y face
    t->addVertex(x0, y0, z0); t->addVertex(x1, y0, z0);
    t->addVertex(x1, y0, z1); t->addVertex(x0, y0, z1);
    // +Y face
    t->addVertex(x0, y1, z0); t->addVertex(x0, y1, z1);
    t->addVertex(x1, y1, z1); t->addVertex(x1, y1, z0);
    // -Z face
    t->addVertex(x0, y0, z0); t->addVertex(x0, y1, z0);
    t->addVertex(x1, y1, z0); t->addVertex(x1, y0, z0);
    // +Z face
    t->addVertex(x0, y0, z1); t->addVertex(x1, y0, z1);
    t->addVertex(x1, y1, z1); t->addVertex(x0, y1, z1);
    // -X face
    t->addVertex(x0, y0, z0); t->addVertex(x0, y0, z1);
    t->addVertex(x0, y1, z1); t->addVertex(x0, y1, z0);
    // +X face
    t->addVertex(x1, y0, z0); t->addVertex(x1, y1, z0);
    t->addVertex(x1, y1, z1); t->addVertex(x1, y0, z1);
    t->draw();
}

RenderItem::RenderItem() {
    renderBlocks = new RenderBlocks();
    random = Random();
    field_27004_a = true;
    shadowSize = 0.15f;
    shadowOpaque = 0.75f;
}

void RenderItem::doRenderItem(EntityItem* entityitem, double d, double d1, double d2, float f, float f1) {
    random.setSeed(187LL);
    ItemStack* itemstack = entityitem->item;
    glPushMatrix();
    float f2 = MathHelper::sin(((float)entityitem->age + f1) / 10.0f + entityitem->hoverStart) * 0.1f + 0.1f;
    float f3 = (((float)entityitem->age + f1) / 20.0f + entityitem->hoverStart) * 57.29578f;
    int byte0 = 1;
    if (entityitem->item->stackSize > 1) {
        byte0 = 2;
    }
    if (entityitem->item->stackSize > 5) {
        byte0 = 3;
    }
    if (entityitem->item->stackSize > 20) {
        byte0 = 4;
    }
    glTranslatef((float)d, (float)d1 + f2, (float)d2);
    glEnable(GL_RESCALE_NORMAL);
    if (itemstack->itemID < 256 && RenderBlocks::renderItemIn3d(Block::blocksList[itemstack->itemID]->getRenderType())) {
        glRotatef(f3, 0.0f, 1.0f, 0.0f);
        loadTexture("/terrain.png");
        float f4 = 0.25f;
        if (!Block::blocksList[itemstack->itemID]->renderAsNormalBlock() && itemstack->itemID != Block::stairSingle->blockID && Block::blocksList[itemstack->itemID]->getRenderType() != 16) {
            f4 = 0.5f;
        }
        glScalef(f4, f4, f4);
        for (int j = 0; j < byte0; j++) {
            glPushMatrix();
            if (j > 0) {
                float f5 = ((random.nextFloat() * 2.0f - 1.0f) * 0.2f) / f4;
                float f7 = ((random.nextFloat() * 2.0f - 1.0f) * 0.2f) / f4;
                float f9 = ((random.nextFloat() * 2.0f - 1.0f) * 0.2f) / f4;
                glTranslatef(f5, f7, f9);
            }
            renderBlocks->renderBlockOnInventory(Block::blocksList[itemstack->itemID], itemstack->getItemDamage(), entityitem->getEntityBrightness(f1));
            glPopMatrix();
        }
    } else {
        glScalef(0.5f, 0.5f, 0.5f);
        int i = itemstack->getIconIndex();
        if (itemstack->itemID < 256) {
            loadTexture("/terrain.png");
        } else {
            loadTexture("/gui/items.png");
        }
        Tessellator* tessellator = &Tessellator::instance;
        float f6 = (float)((i % 16) * 16 + 0) / 256.0f;
        float f8 = (float)((i % 16) * 16 + 16) / 256.0f;
        float f10 = (float)((i / 16) * 16 + 0) / 256.0f;
        float f11 = (float)((i / 16) * 16 + 16) / 256.0f;
        float f12 = 1.0f;
        float f13 = 0.5f;
        float f14 = 0.25f;
        if (field_27004_a) {
            int k = Item::itemsList[itemstack->itemID]->getColorFromDamage(itemstack->getItemDamage());
            float f15 = (float)(k >> 16 & 0xff) / 255.0f;
            float f17 = (float)(k >> 8 & 0xff) / 255.0f;
            float f19 = (float)(k & 0xff) / 255.0f;
            float f21 = entityitem->getEntityBrightness(f1);
            glColor4f(f15 * f21, f17 * f21, f19 * f21, 1.0f);
        }
        for (int l = 0; l < byte0; l++) {
            glPushMatrix();
            if (l > 0) {
                float f16 = (random.nextFloat() * 2.0f - 1.0f) * 0.3f;
                float f18 = (random.nextFloat() * 2.0f - 1.0f) * 0.3f;
                float f20 = (random.nextFloat() * 2.0f - 1.0f) * 0.3f;
                glTranslatef(f16, f18, f20);
            }
            glRotatef(180.0f - renderManager->playerViewY, 0.0f, 1.0f, 0.0f);
            tessellator->startDrawingQuads();
            tessellator->setNormal(0.0f, 1.0f, 0.0f);
            tessellator->addVertexWithUV(0.0f - f13, 0.0f - f14, 0.0, f6, f11);
            tessellator->addVertexWithUV(f12 - f13, 0.0f - f14, 0.0, f8, f11);
            tessellator->addVertexWithUV(f12 - f13, 1.0f - f14, 0.0, f8, f10);
            tessellator->addVertexWithUV(0.0f - f13, 1.0f - f14, 0.0, f6, f10);
            tessellator->draw();
            glPopMatrix();
        }
    }
    glDisable(GL_RESCALE_NORMAL);
    glPopMatrix();
}

void RenderItem::drawItemIntoGui(FontRenderer* fontrenderer, RenderEngine* renderengine, int i, int j, int k, int l, int i1) {
    if (i < 256 && RenderBlocks::renderItemIn3d(Block::blocksList[i]->getRenderType())) {
        int j1 = i;
        renderengine->bindTexture(renderengine->getTexture("/terrain.png"));
        Block* block = Block::blocksList[j1];
        glPushMatrix();
        glTranslatef(l - 2, i1 + 3, -3.0f);
        glScalef(10.0f, 10.0f, 10.0f);
        glTranslatef(1.0f, 0.5f, 1.0f);
        glScalef(1.0f, 1.0f, -1.0f);
        glRotatef(210.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
        int l1 = Item::itemsList[i]->getColorFromDamage(j);
        float f2 = (float)(l1 >> 16 & 0xff) / 255.0f;
        float f4 = (float)(l1 >> 8 & 0xff) / 255.0f;
        float f5 = (float)(l1 & 0xff) / 255.0f;
        if (field_27004_a) {
            glColor4f(f2, f4, f5, 1.0f);
        }
        glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
        renderBlocks->field_31088_b = field_27004_a;
        renderBlocks->renderBlockOnInventory(block, j, 1.0f);
        renderBlocks->field_31088_b = true;
        glPopMatrix();
    } else if (k >= 0) {
        glDisable(GL_LIGHTING);
        if (i < 256) {
            renderengine->bindTexture(renderengine->getTexture("/terrain.png"));
        } else {
            renderengine->bindTexture(renderengine->getTexture("/gui/items.png"));
        }
        int k1 = Item::itemsList[i]->getColorFromDamage(j);
        float f = (float)(k1 >> 16 & 0xff) / 255.0f;
        float f1 = (float)(k1 >> 8 & 0xff) / 255.0f;
        float f3 = (float)(k1 & 0xff) / 255.0f;
        if (field_27004_a) {
            glColor4f(f, f1, f3, 1.0f);
        }
        renderTexturedQuad(l, i1, (k % 16) * 16, (k / 16) * 16, 16, 16);
        glEnable(GL_LIGHTING);
    }
    glEnable(GL_CULL_FACE);
}

void RenderItem::renderItemIntoGUI(FontRenderer* fontrenderer, RenderEngine* renderengine, ItemStack* itemstack, int i, int j) {
    if (itemstack == nullptr) {
        return;
    } else {
        drawItemIntoGui(fontrenderer, renderengine, itemstack->itemID, itemstack->getItemDamage(), itemstack->getIconIndex(), i, j);
        return;
    }
}

void RenderItem::renderItemOverlayIntoGUI(FontRenderer* fontrenderer, RenderEngine* renderengine, ItemStack* itemstack, int i, int j) {
    if (itemstack == nullptr) {
        return;
    }
    if (itemstack->stackSize > 1) {
        std::string s = std::to_string(itemstack->stackSize);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        fontrenderer->drawStringWithShadow(s, (i + 19) - 2 - fontrenderer->getStringWidth(s), j + 6 + 3, 0xffffff);
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
    }
    if (itemstack->isItemDamaged()) {
        int k = (int)round(13.0 - ((double)itemstack->getItemDamageForDisplay() * 13.0) / (double)itemstack->getMaxDamage());
        int l = (int)round(255.0 - ((double)itemstack->getItemDamageForDisplay() * 255.0) / (double)itemstack->getMaxDamage());
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);
        Tessellator* tessellator = &Tessellator::instance;
        int i1 = (255 - l) << 16 | l << 8;
        int j1 = ((255 - l) / 4) << 16 | 0x3f00;
        renderQuad(tessellator, i + 2, j + 13, 13, 2, 0);
        renderQuad(tessellator, i + 2, j + 13, 12, 1, j1);
        renderQuad(tessellator, i + 2, j + 13, k, 1, i1);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

void RenderItem::renderQuad(Tessellator* tessellator, int i, int j, int k, int l, int i1) {
    tessellator->startDrawingQuads();
    tessellator->setColorOpaque_I(i1);
    tessellator->addVertex(i + 0, j + 0, 0.0);
    tessellator->addVertex(i + 0, j + l, 0.0);
    tessellator->addVertex(i + k, j + l, 0.0);
    tessellator->addVertex(i + k, j + 0, 0.0);
    tessellator->draw();
}

void RenderItem::renderItemIcon2D(RenderEngine* renderengine, FontRenderer* fontrenderer, ItemStack* itemstack, int i, int j, float scale) {
    if (itemstack == nullptr)
        return;
    int id = itemstack->itemID;
    int icon = itemstack->getIconIndex();
    if (id < 256)
        renderengine->bindTexture(renderengine->getTexture("/terrain.png"));
    else
        renderengine->bindTexture(renderengine->getTexture("/gui/items.png"));

    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    // Explicit-size quad instead of a matrix scale: the GUI's Tessellator
    // path must not depend on the fixed-function matrix here.
    {
        const float f = 0.0f;
        const float f1 = 0.00390625f;
        float dst = 16.0f * scale;
        float u = (float)((icon % 16) * 16);
        float v = (float)((icon / 16) * 16);
        Tessellator *t = &Tessellator::instance;
        t->startDrawingQuads();
        t->addVertexWithUV((double)i,          (double)(j + dst), f, u * f1,          (v + 16.0f) * f1);
        t->addVertexWithUV((double)(i + dst),  (double)(j + dst), f, (u + 16.0f) * f1, (v + 16.0f) * f1);
        t->addVertexWithUV((double)(i + dst),  (double)j,         f, (u + 16.0f) * f1, v * f1);
        t->addVertexWithUV((double)i,          (double)j,         f, u * f1,          v * f1);
        t->draw();
    }
    // NOTE: leave lighting DISABLED — the GUI draws text right after and glyphs
    // rendered with lighting on came out black.
    if (fontrenderer != nullptr && itemstack->stackSize > 1) {
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        fontrenderer->drawStringWithShadow(std::to_string(itemstack->stackSize),
                                           i + (int)(16.0f * scale) - 4,
                                           j + (int)(16.0f * scale) - 6, 0xffffff);
        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
    }
}

void RenderItem::renderTexturedQuad(int i, int j, int k, int l, int i1, int j1) {
    float f = 0.0f;
    float f1 = 0.00390625f;
    float f2 = 0.00390625f;
    Tessellator* tessellator = &Tessellator::instance;
    tessellator->startDrawingQuads();
    tessellator->addVertexWithUV(i + 0, j + j1, f, (float)(k + 0) * f1, (float)(l + j1) * f2);
    tessellator->addVertexWithUV(i + i1, j + j1, f, (float)(k + i1) * f1, (float)(l + j1) * f2);
    tessellator->addVertexWithUV(i + i1, j + 0, f, (float)(k + i1) * f1, (float)(l + 0) * f2);
    tessellator->addVertexWithUV(i + 0, j + 0, f, (float)(k + 0) * f1, (float)(l + 0) * f2);
    tessellator->draw();
}

void RenderItem::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    doRenderItem((EntityItem*)entity, d, d1, d2, f, f1);
}
