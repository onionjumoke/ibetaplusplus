#include "RenderTNTPrimed.h"
#include "EntityTNTPrimed.h"
#include "Block.h"
#include "RenderBlocks.h"
#include "Tessellator.h"
#include <SDL2/SDL_opengl.h>

RenderTNTPrimed::RenderTNTPrimed() {
    shadowSize = 0.5f;
}

void RenderTNTPrimed::renderTNTPrimed(EntityTNTPrimed* entityTNTPrimed, double d, double d1, double d2, float f, float f1) {
    glPushMatrix();
    glTranslatef((float)d, (float)d1, (float)d2);
    if (((float)entityTNTPrimed->fuse - f1) + 1.0f < 10.0f) {
        float f2 = 1.0f - (((float)entityTNTPrimed->fuse - f1) + 1.0f) / 10.0f;
        if (f2 < 0.0f) {
            f2 = 0.0f;
        }
        if (f2 > 1.0f) {
            f2 = 1.0f;
        }
        f2 *= f2;
        f2 *= f2;
        float f3 = 1.0f + f2 * 0.3f;
        glScalef(f3, f3, f3);
    }
    float f4 = (1.0f - (((float)entityTNTPrimed->fuse - f1) + 1.0f) / 100.0f) * 0.8f;
    renderBlockOnTNT(entityTNTPrimed, f, f1);
    if (((float)entityTNTPrimed->fuse - f1) + 1.0f < 10.0f) {
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glColor4f(1.0f, 1.0f, 1.0f, f4);
        renderBlockOnTNT(entityTNTPrimed, f, f1);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
    }
    glPopMatrix();
}

void RenderTNTPrimed::renderBlockOnTNT(EntityTNTPrimed* entityTNTPrimed, float f, float f1) {
    glPushMatrix();
    glScalef(0.5f, 0.5f, 0.5f);
    int i = Block::tnt->getRenderType();
    if (i == 0 || i == 16) {
        Tessellator* tessellator = &Tessellator::instance;
        loadTexture("/terrain.png");
        float f2 = Block::tnt->getBlockTextureFromSide(0);
        int j = (int)(f2 * 16.0f) % 16;
        int k = (int)(f2 * 16.0f) / 16;
        float f3 = (float)j / 16.0f;
        float f4 = ((float)j + 0.99f) / 16.0f;
        float f5 = (float)k / 16.0f;
        float f6 = ((float)k + 0.99f) / 16.0f;
        glTranslatef(-0.5f, -0.5f, -0.5f);
        tessellator->startDrawingQuads();
        tessellator->setNormal(0.0f, -1.0f, 0.0f);
        tessellator->addVertexWithUV(0.0, 0.0, 1.0, (double)f3, (double)f6);
        tessellator->addVertexWithUV(1.0, 0.0, 1.0, (double)f4, (double)f6);
        tessellator->addVertexWithUV(1.0, 0.0, 0.0, (double)f4, (double)f5);
        tessellator->addVertexWithUV(0.0, 0.0, 0.0, (double)f3, (double)f5);
        tessellator->draw();
        tessellator->startDrawingQuads();
        tessellator->setNormal(0.0f, 1.0f, 0.0f);
        tessellator->addVertexWithUV(0.0, 1.0, 0.0, (double)f3, (double)f5);
        tessellator->addVertexWithUV(1.0, 1.0, 0.0, (double)f4, (double)f5);
        tessellator->addVertexWithUV(1.0, 1.0, 1.0, (double)f4, (double)f6);
        tessellator->addVertexWithUV(0.0, 1.0, 1.0, (double)f3, (double)f6);
        tessellator->draw();
        tessellator->startDrawingQuads();
        tessellator->setNormal(0.0f, 0.0f, -1.0f);
        tessellator->addVertexWithUV(0.0, 1.0, 0.0, (double)f3, (double)f5);
        tessellator->addVertexWithUV(0.0, 0.0, 0.0, (double)f3, (double)f6);
        tessellator->addVertexWithUV(1.0, 0.0, 0.0, (double)f4, (double)f6);
        tessellator->addVertexWithUV(1.0, 1.0, 0.0, (double)f4, (double)f5);
        tessellator->draw();
        tessellator->startDrawingQuads();
        tessellator->setNormal(0.0f, 0.0f, 1.0f);
        tessellator->addVertexWithUV(1.0, 1.0, 1.0, (double)f4, (double)f5);
        tessellator->addVertexWithUV(1.0, 0.0, 1.0, (double)f4, (double)f6);
        tessellator->addVertexWithUV(0.0, 0.0, 1.0, (double)f3, (double)f6);
        tessellator->addVertexWithUV(0.0, 1.0, 1.0, (double)f3, (double)f5);
        tessellator->draw();
        tessellator->startDrawingQuads();
        tessellator->setNormal(-1.0f, 0.0f, 0.0f);
        tessellator->addVertexWithUV(0.0, 1.0, 1.0, (double)f4, (double)f5);
        tessellator->addVertexWithUV(0.0, 0.0, 1.0, (double)f4, (double)f6);
        tessellator->addVertexWithUV(0.0, 0.0, 0.0, (double)f3, (double)f6);
        tessellator->addVertexWithUV(0.0, 1.0, 0.0, (double)f3, (double)f5);
        tessellator->draw();
        tessellator->startDrawingQuads();
        tessellator->setNormal(1.0f, 0.0f, 0.0f);
        tessellator->addVertexWithUV(1.0, 1.0, 0.0, (double)f3, (double)f5);
        tessellator->addVertexWithUV(1.0, 0.0, 0.0, (double)f3, (double)f6);
        tessellator->addVertexWithUV(1.0, 0.0, 1.0, (double)f4, (double)f6);
        tessellator->addVertexWithUV(1.0, 1.0, 1.0, (double)f4, (double)f5);
        tessellator->draw();
    } else {
        renderBlocks->renderBlockOnInventory(Block::tnt, 0, 1.0f);
    }
    glPopMatrix();
}

void RenderTNTPrimed::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    renderTNTPrimed((EntityTNTPrimed*)entity, d, d1, d2, f, f1);
}
