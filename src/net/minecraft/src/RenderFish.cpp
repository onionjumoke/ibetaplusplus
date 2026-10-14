#include "RenderFish.h"
#include "EntityFish.h"
#include "EntityPlayer.h"
#include "Tessellator.h"
#include "MathHelper.h"
#include "Vec3D.h"
#include "GameSettings.h"
#include <SDL2/SDL_opengl.h>

RenderFish::RenderFish() {
}

void RenderFish::doRenderFish(EntityFish* entityfish, double d, double d1, double d2, float f, float f1) {
    glPushMatrix();
    glTranslatef((float)d, (float)d1, (float)d2);
    glEnable(GL_RESCALE_NORMAL);
    glScalef(0.5f, 0.5f, 0.5f);
    int i = 1;
    int byte0 = 2;
    loadTexture("/particles.png");
    Tessellator* tessellator = &Tessellator::instance;
    float f2 = (float)(i * 8 + 0) / 128.0f;
    float f3 = (float)(i * 8 + 8) / 128.0f;
    float f4 = (float)(byte0 * 8 + 0) / 128.0f;
    float f5 = (float)(byte0 * 8 + 8) / 128.0f;
    float f6 = 1.0f;
    float f7 = 0.5f;
    float f8 = 0.5f;
    glRotatef(180.0f - renderManager->playerViewY, 0.0f, 1.0f, 0.0f);
    glRotatef(-renderManager->playerViewX, 1.0f, 0.0f, 0.0f);
    tessellator->startDrawingQuads();
    tessellator->setNormal(0.0f, 1.0f, 0.0f);
    tessellator->addVertexWithUV(0.0f - f7, 0.0f - f8, 0.0, f2, f5);
    tessellator->addVertexWithUV(f6 - f7, 0.0f - f8, 0.0, f3, f5);
    tessellator->addVertexWithUV(f6 - f7, 1.0f - f8, 0.0, f3, f4);
    tessellator->addVertexWithUV(0.0f - f7, 1.0f - f8, 0.0, f2, f4);
    tessellator->draw();
    glDisable(GL_RESCALE_NORMAL);
    glPopMatrix();

    if (entityfish->angler != nullptr) {
        float f9 = ((entityfish->angler->prevRotationYaw + (entityfish->angler->rotationYaw - entityfish->angler->prevRotationYaw) * f1) * 3.141593f) / 180.0f;
        double d3 = MathHelper::sin(f9);
        double d5 = MathHelper::cos(f9);
        float f11 = entityfish->angler->getSwingProgress(f1);
        float f12 = MathHelper::sin(MathHelper::sqrt_float(f11) * 3.141593f);
        Vec3D* vec3d = Vec3D::createVector(-0.5, 0.03, 0.8);
        vec3d->rotateAroundX((-(entityfish->angler->prevRotationPitch + (entityfish->angler->rotationPitch - entityfish->angler->prevRotationPitch) * f1) * 3.141593f) / 180.0f);
        vec3d->rotateAroundY((-(entityfish->angler->prevRotationYaw + (entityfish->angler->rotationYaw - entityfish->angler->prevRotationYaw) * f1) * 3.141593f) / 180.0f);
        vec3d->rotateAroundY(f12 * 0.5f);
        vec3d->rotateAroundX(-f12 * 0.7f);
        double d7 = entityfish->angler->prevPosX + (entityfish->angler->posX - entityfish->angler->prevPosX) * (double)f1 + vec3d->xCoord;
        double d8 = entityfish->angler->prevPosY + (entityfish->angler->posY - entityfish->angler->prevPosY) * (double)f1 + vec3d->yCoord;
        double d9 = entityfish->angler->prevPosZ + (entityfish->angler->posZ - entityfish->angler->prevPosZ) * (double)f1 + vec3d->zCoord;
        if (renderManager->options->thirdPersonView) {
            float f10 = ((entityfish->angler->prevRenderYawOffset + (entityfish->angler->renderYawOffset - entityfish->angler->prevRenderYawOffset) * f1) * 3.141593f) / 180.0f;
            double d4 = MathHelper::sin(f10);
            double d6 = MathHelper::cos(f10);
            d7 = (entityfish->angler->prevPosX + (entityfish->angler->posX - entityfish->angler->prevPosX) * (double)f1) - d6 * 0.35 - d4 * 0.85;
            d8 = (entityfish->angler->prevPosY + (entityfish->angler->posY - entityfish->angler->prevPosY) * (double)f1) - 0.45;
            d9 = ((entityfish->angler->prevPosZ + (entityfish->angler->posZ - entityfish->angler->prevPosZ) * (double)f1) - d4 * 0.35) + d6 * 0.85;
        }
        double d10 = entityfish->prevPosX + (entityfish->posX - entityfish->prevPosX) * (double)f1;
        double d11 = entityfish->prevPosY + (entityfish->posY - entityfish->prevPosY) * (double)f1 + 0.25;
        double d12 = entityfish->prevPosZ + (entityfish->posZ - entityfish->prevPosZ) * (double)f1;
        double d13 = d7 - d10;
        double d14 = d8 - d11;
        double d15 = d9 - d12;
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
        tessellator->startDrawing(3);
        tessellator->setColorOpaque_I(0);
        int j = 16;
        for (int k = 0; k <= j; k++) {
            float f13 = (float)k / (float)j;
            tessellator->addVertex(d + d13 * (double)f13, d1 + d14 * (double)(f13 * f13 + f13) * 0.5 + 0.25, d2 + d15 * (double)f13);
        }
        tessellator->draw();
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
    }
}

void RenderFish::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    doRenderFish((EntityFish*)entity, d, d1, d2, f, f1);
}
