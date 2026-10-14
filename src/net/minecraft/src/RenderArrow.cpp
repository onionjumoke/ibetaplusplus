#include "RenderArrow.h"
#include "EntityArrow.h"
#include "Tessellator.h"
#include "MathHelper.h"
#include "Entity.h"
#include <SDL2/SDL_opengl.h>

RenderArrow::RenderArrow() {
}

void RenderArrow::renderArrow(EntityArrow* entityArrow, double d, double d1, double d2, float f, float f1) {
    if (entityArrow->prevRotationYaw == 0.0f && entityArrow->prevRotationPitch == 0.0f) {
        return;
    }

    loadTexture("/item/arrows.png");

    glPushMatrix();
    glTranslatef((float)d, (float)d1, (float)d2);
    glRotatef((entityArrow->prevRotationYaw + (entityArrow->rotationYaw - entityArrow->prevRotationYaw) * f1) - 90.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(entityArrow->prevRotationPitch + (entityArrow->rotationPitch - entityArrow->prevRotationPitch) * f1, 0.0f, 0.0f, 1.0f);

    Tessellator* tessellator = &Tessellator::instance;

    int i = 0;
    float f2 = 0.0f;
    float f3 = 0.5f;
    float f4 = (float)(0 + i * 10) / 32.0f;
    float f5 = (float)(5 + i * 10) / 32.0f;
    float f6 = 0.0f;
    float f7 = 0.15625f;
    float f8 = (float)(5 + i * 10) / 32.0f;
    float f9 = (float)(10 + i * 10) / 32.0f;
    float f10 = 0.05625f;

    glEnable(GL_RESCALE_NORMAL);

    float f11 = (float)entityArrow->arrowShake - f1;
    if (f11 > 0.0f) {
        float f12 = -MathHelper::sin(f11 * 3.0f) * f11;
        glRotatef(f12, 0.0f, 0.0f, 1.0f);
    }

    glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
    glScalef(f10, f10, f10);
    glTranslatef(-4.0f, 0.0f, 0.0f);
    glNormal3f(f10, 0.0f, 0.0f);
    tessellator->startDrawingQuads();
    tessellator->addVertexWithUV(-7.0, -2.0, -2.0, f6, f8);
    tessellator->addVertexWithUV(-7.0, -2.0,  2.0, f7, f8);
    tessellator->addVertexWithUV(-7.0,  2.0,  2.0, f7, f9);
    tessellator->addVertexWithUV(-7.0,  2.0, -2.0, f6, f9);
    tessellator->draw();
    glNormal3f(-f10, 0.0f, 0.0f);
    tessellator->startDrawingQuads();
    tessellator->addVertexWithUV(-7.0,  2.0, -2.0, f6, f8);
    tessellator->addVertexWithUV(-7.0,  2.0,  2.0, f7, f8);
    tessellator->addVertexWithUV(-7.0, -2.0,  2.0, f7, f9);
    tessellator->addVertexWithUV(-7.0, -2.0, -2.0, f6, f9);
    tessellator->draw();

    for (int_t j = 0; j < 4; ++j) {
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        glNormal3f(0.0f, 0.0f, f10);
        tessellator->startDrawingQuads();
        tessellator->addVertexWithUV(-8.0, -2.0, 0.0, f2, f4);
        tessellator->addVertexWithUV( 8.0, -2.0, 0.0, f3, f4);
        tessellator->addVertexWithUV( 8.0,  2.0, 0.0, f3, f5);
        tessellator->addVertexWithUV(-8.0,  2.0, 0.0, f2, f5);
        tessellator->draw();
    }

    glDisable(GL_RESCALE_NORMAL);
    glPopMatrix();
}

void RenderArrow::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    renderArrow(static_cast<EntityArrow*>(entity), d, d1, d2, f, f1);
}
