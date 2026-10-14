#include "RenderBoat.h"
#include "ModelBoat.h"
#include "EntityBoat.h"
#include "MathHelper.h"
#include "ModelBase.h"
#include "Entity.h"
#include <SDL2/SDL_opengl.h>

RenderBoat::RenderBoat() {
    shadowSize = 0.5f;
    modelBoat = new ModelBoat();
}

void RenderBoat::renderBoat(EntityBoat* entityBoat, double d, double d1, double d2, float f, float f1) {
    glPushMatrix();
    glTranslatef((float)d, (float)d1, (float)d2);
    glRotatef(180.0f - f, 0.0f, 1.0f, 0.0f);

    float f2 = (float)entityBoat->boatTimeSinceHit - f1;
    float f3 = (float)entityBoat->boatCurrentDamage - f1;
    if (f3 < 0.0f) {
        f3 = 0.0f;
    }
    if (f2 > 0.0f) {
        glRotatef(((MathHelper::sin(f2) * f2 * f3) / 10.0f) * (float)entityBoat->boatRockDirection, 1.0f, 0.0f, 0.0f);
    }

    loadTexture("/terrain.png");

    float f4 = 0.75f;
    glScalef(f4, f4, f4);
    glScalef(1.0f / f4, 1.0f / f4, 1.0f / f4);

    loadTexture("/item/boat.png");

    glScalef(-1.0f, -1.0f, 1.0f);
    modelBoat->render(0.0f, 0.0f, -0.1f, 0.0f, 0.0f, 0.0625f);

    glPopMatrix();
}

void RenderBoat::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    renderBoat((EntityBoat*)entity, d, d1, d2, f, f1);
}