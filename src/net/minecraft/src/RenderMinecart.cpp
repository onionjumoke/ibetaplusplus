#include "RenderMinecart.h"
#include "EntityMinecart.h"
#include "ModelMinecart.h"
#include "MathHelper.h"
#include "Vec3D.h"
#include "RenderBlocks.h"
#include "Block.h"
#include <SDL2/SDL_opengl.h>

RenderMinecart::RenderMinecart() {
    shadowSize = 0.5f;
    modelMinecart = new ModelMinecart();
}

void RenderMinecart::renderMinecart(EntityMinecart* entityMinecart, double d, double d1, double d2, float f, float f1) {
    glPushMatrix();
    double d3 = entityMinecart->lastTickPosX + (entityMinecart->posX - entityMinecart->lastTickPosX) * (double)f1;
    double d4 = entityMinecart->lastTickPosY + (entityMinecart->posY - entityMinecart->lastTickPosY) * (double)f1;
    double d5 = entityMinecart->lastTickPosZ + (entityMinecart->posZ - entityMinecart->lastTickPosZ) * (double)f1;
    double d6 = 0.30000001192092896;
    Vec3D* vec3d = entityMinecart->getPos(d3, d4, d5);
    float f5 = entityMinecart->prevRotationPitch + (entityMinecart->rotationPitch - entityMinecart->prevRotationPitch) * f1;
    if (vec3d != nullptr) {
        Vec3D* vec3d1 = entityMinecart->getPosOffset(d3, d4, d5, d6);
        Vec3D* vec3d2 = entityMinecart->getPosOffset(d3, d4, d5, -d6);
        if (vec3d1 == nullptr) vec3d1 = vec3d;
        if (vec3d2 == nullptr) vec3d2 = vec3d;
        d += vec3d->xCoord - d3;
        d1 += (vec3d1->yCoord + vec3d2->yCoord) / 2.0 - d4;
        d2 += vec3d->zCoord - d5;
        Vec3D* vec3d3 = vec3d2->addVector(-vec3d1->xCoord, -vec3d1->yCoord, -vec3d1->zCoord);
        if (vec3d3->lengthVector() != 0.0) {
            vec3d3 = vec3d3->normalize();
            f = (float)(atan2(vec3d3->zCoord, vec3d3->xCoord) * 180.0 / 3.1415926535897931);
            f5 = (float)(atan(vec3d3->yCoord) * 73.0);
        }
    }
    glTranslatef((float)d, (float)d1, (float)d2);
    glRotatef(180.0f - f, 0.0f, 1.0f, 0.0f);
    glRotatef(-f5, 0.0f, 0.0f, 1.0f);
    float f6 = (float)entityMinecart->rollTime - f1;
    float f7 = (float)entityMinecart->damageTaken - f1;
    if (f7 < 0.0f) f7 = 0.0f;
    if (f6 > 0.0f) {
        glRotatef(MathHelper::sin(f6) * f6 * f7 / 10.0f * (float)entityMinecart->rollDirection, 1.0f, 0.0f, 0.0f);
    }
    if (entityMinecart->getMinecartType() != 0) {
        loadTexture("/terrain.png");
        float f8 = 0.75f;
        glScalef(f8, f8, f8);
        glTranslatef(0.0f, 0.3125f, 0.0f);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        if (entityMinecart->getMinecartType() == 1) {
            (new RenderBlocks())->renderBlockOnInventory(Block::chest, 0, entityMinecart->getEntityBrightness(f1));
        } else if (entityMinecart->getMinecartType() == 2) {
            (new RenderBlocks())->renderBlockOnInventory(Block::stoneOvenIdle, 0, entityMinecart->getEntityBrightness(f1));
        }
        glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
        glTranslatef(0.0f, -0.3125f, 0.0f);
        glScalef(1.0f / f8, 1.0f / f8, 1.0f / f8);
    }
    loadTexture("/item/cart.png");
    glScalef(-1.0f, -1.0f, 1.0f);
    modelMinecart->render(0.0f, 0.0f, -0.1f, 0.0f, 0.0f, 0.0625f);
    glPopMatrix();
}

void RenderMinecart::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    renderMinecart((EntityMinecart*)entity, d, d1, d2, f, f1);
}