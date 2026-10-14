#include "RenderPainting.h"
#include "EntityPainting.h"
#include "EnumArt.h"
#include "Tessellator.h"
#include <SDL2/SDL_opengl.h>

RenderPainting::RenderPainting() {
}

void RenderPainting::renderPainting(EntityPainting* entityPainting, double d, double d1, double d2, float f, float f1) {
    glPushMatrix();
    glTranslatef((float)d, (float)d1, (float)d2);
    glRotatef(f, 0.0f, 1.0f, 0.0f);
    glEnable(GL_RESCALE_NORMAL);
    EnumArt* enumart = entityPainting->art;
    float f2 = 0.0625f;
    glScalef(f2, f2, f2);
    loadTexture("/art/kz.png");
    Tessellator* tessellator = &Tessellator::instance;
    
    if (enumart != nullptr) {
        int k = enumart->offsetX;
        int l = enumart->offsetY;
        int i1 = enumart->sizeX;
        int j1 = enumart->sizeY;
        
        float f3 = (float)(-i1) / 2.0f;
        float f4 = (float)(-j1) / 2.0f;
        
        float f5 = 1.0f / 256.0f;
        float f6 = 1.0f / 256.0f;
        
        float f7 = (float)k * f5;
        float f8 = (float)(k + i1) * f5;
        float f9 = (float)l * f6;
        float f10 = (float)(l + j1) * f6;
        
        for (int k1 = 0; k1 < i1 / 16; k1++) {
            for (int l1 = 0; l1 < j1 / 16; l1++) {
                float f11 = f3 + (float)((k1 + 1) * 16);
                float f12 = f3 + (float)(k1 * 16);
                float f13 = f4 + (float)((l1 + 1) * 16);
                float f14 = f4 + (float)(l1 * 16);
                
                float f15 = f7 + (float)(k1 * 16) * f5;
                float f16 = f7 + (float)((k1 + 1) * 16) * f5;
                float f17 = f9 + (float)(l1 * 16) * f6;
                float f18 = f9 + (float)((l1 + 1) * 16) * f6;
                
                glNormal3f(0.0f, 0.0f, -1.0f);
                tessellator->startDrawingQuads();
                tessellator->addVertexWithUV((double)f11, (double)f14, 0.0, (double)f16, (double)f17);
                tessellator->addVertexWithUV((double)f12, (double)f14, 0.0, (double)f15, (double)f17);
                tessellator->addVertexWithUV((double)f12, (double)f13, 0.0, (double)f15, (double)f18);
                tessellator->addVertexWithUV((double)f11, (double)f13, 0.0, (double)f16, (double)f18);
                tessellator->draw();
                
                glNormal3f(0.0f, 0.0f, 1.0f);
                tessellator->startDrawingQuads();
                tessellator->addVertexWithUV((double)f11, (double)f13, 1.0, (double)f16, (double)f18);
                tessellator->addVertexWithUV((double)f12, (double)f13, 1.0, (double)f15, (double)f18);
                tessellator->addVertexWithUV((double)f12, (double)f14, 1.0, (double)f15, (double)f17);
                tessellator->addVertexWithUV((double)f11, (double)f14, 1.0, (double)f16, (double)f17);
                tessellator->draw();
            }
        }
        
        // Render back
        glNormal3f(0.0f, 1.0f, 0.0f);
        tessellator->startDrawingQuads();
        tessellator->addVertexWithUV((double)f3, (double)f4, 0.0, 0.0, 0.0);
        tessellator->addVertexWithUV((double)(f3 + i1), (double)f4, 0.0, 1.0, 0.0);
        tessellator->addVertexWithUV((double)(f3 + i1), (double)f4, 1.0, 1.0, 1.0);
        tessellator->addVertexWithUV((double)f3, (double)f4, 1.0, 0.0, 1.0);
        tessellator->draw();
        
        glNormal3f(0.0f, -1.0f, 0.0f);
        tessellator->startDrawingQuads();
        tessellator->addVertexWithUV((double)f3, (double)(f4 + j1), 1.0, 0.0, 1.0);
        tessellator->addVertexWithUV((double)(f3 + i1), (double)(f4 + j1), 1.0, 1.0, 1.0);
        tessellator->addVertexWithUV((double)(f3 + i1), (double)(f4 + j1), 0.0, 1.0, 0.0);
        tessellator->addVertexWithUV((double)f3, (double)(f4 + j1), 0.0, 0.0, 0.0);
        tessellator->draw();
        
        glNormal3f(1.0f, 0.0f, 0.0f);
        tessellator->startDrawingQuads();
        tessellator->addVertexWithUV((double)f3, (double)f4, 1.0, 0.0, 0.0);
        tessellator->addVertexWithUV((double)f3, (double)(f4 + j1), 1.0, 0.0, 1.0);
        tessellator->addVertexWithUV((double)f3, (double)(f4 + j1), 0.0, 1.0, 1.0);
        tessellator->addVertexWithUV((double)f3, (double)f4, 0.0, 1.0, 0.0);
        tessellator->draw();
        
        glNormal3f(-1.0f, 0.0f, 0.0f);
        tessellator->startDrawingQuads();
        tessellator->addVertexWithUV((double)(f3 + i1), (double)f4, 0.0, 0.0, 0.0);
        tessellator->addVertexWithUV((double)(f3 + i1), (double)(f4 + j1), 0.0, 0.0, 1.0);
        tessellator->addVertexWithUV((double)(f3 + i1), (double)(f4 + j1), 1.0, 1.0, 1.0);
        tessellator->addVertexWithUV((double)(f3 + i1), (double)f4, 1.0, 1.0, 0.0);
        tessellator->draw();
    }
    
    glDisable(GL_RESCALE_NORMAL);
    glPopMatrix();
}

void RenderPainting::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    renderPainting((EntityPainting*)entity, d, d1, d2, f, f1);
}
