#include "RenderFireball.h"
#include "EntityFireball.h"
#include "Item.h"
#include "Tessellator.h"
#include <SDL2/SDL_opengl.h>

RenderFireball::RenderFireball() {
}

void RenderFireball::doRenderFireball(EntityFireball* entityfireball, double d, double d1, double d2, float f, float f1) {
    glPushMatrix();
    glTranslatef((float)d, (float)d1, (float)d2);
    glEnable(GL_RESCALE_NORMAL);
    float f2 = 2.0f;
    glScalef(f2 / 1.0f, f2 / 1.0f, f2 / 1.0f);
    int i = Item::snowball->getIconFromDamage(0);
    loadTexture("/gui/items.png");
    Tessellator* tessellator = &Tessellator::instance;
    float f3 = (float)((i % 16) * 16 + 0) / 256.0f;
    float f4 = (float)((i % 16) * 16 + 16) / 256.0f;
    float f5 = (float)((i / 16) * 16 + 0) / 256.0f;
    float f6 = (float)((i / 16) * 16 + 16) / 256.0f;
    float f7 = 1.0f;
    float f8 = 0.5f;
    float f9 = 0.25f;
    glRotatef(180.0f - renderManager->playerViewY, 0.0f, 1.0f, 0.0f);
    glRotatef(-renderManager->playerViewX, 1.0f, 0.0f, 0.0f);
    tessellator->startDrawingQuads();
    tessellator->setNormal(0.0f, 1.0f, 0.0f);
    tessellator->addVertexWithUV(0.0f - f8, 0.0f - f9, 0.0, f3, f6);
    tessellator->addVertexWithUV(f7 - f8, 0.0f - f9, 0.0, f4, f6);
    tessellator->addVertexWithUV(f7 - f8, 1.0f - f9, 0.0, f4, f5);
    tessellator->addVertexWithUV(0.0f - f8, 1.0f - f9, 0.0, f3, f5);
    tessellator->draw();
    glDisable(GL_RESCALE_NORMAL);
    glPopMatrix();
}

void RenderFireball::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    doRenderFireball((EntityFireball*)entity, d, d1, d2, f, f1);
}
