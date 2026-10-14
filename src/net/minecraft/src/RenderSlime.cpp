#include "RenderSlime.h"
#include "EntitySlime.h"
#include "ModelBase.h"
#include "EntityLiving.h"
#include "Entity.h"
#include "MathHelper.h"
#include <SDL2/SDL_opengl.h>

RenderSlime::RenderSlime(ModelBase* modelBase, ModelBase* modelBase2, float f) : RenderLiving(modelBase, f) {
    scaleAmount = modelBase2;
}

void RenderSlime::renderSlime(EntitySlime* entitySlime, double d, double d1, double d2, float f, float f1) {
    RenderLiving::doRenderLiving(entitySlime, d, d1, d2, f, f1);
}

void RenderSlime::doRenderLiving(EntityLiving* entityLiving, double d, double d1, double d2, float f, float f1) {
    renderSlime((EntitySlime*)entityLiving, d, d1, d2, f, f1);
}

void RenderSlime::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    renderSlime((EntitySlime*)entity, d, d1, d2, f, f1);
}

void RenderSlime::preRenderCallback(EntityLiving* entityLiving, float f) {
    scaleSlime((EntitySlime*)entityLiving, f);
}

void RenderSlime::scaleSlime(EntitySlime* entitySlime, float f) {
    float f1 = (float)entitySlime->getSlimeSize();
    float f2 = (entitySlime->field_767_b + (entitySlime->field_768_a - entitySlime->field_767_b) * f) / (f1 * 0.5f + 1.0f);
    float f3 = 1.0f / (f2 + 1.0f);
    glScalef(f3 * f1, 1.0f / f3 * f1, f3 * f1);
}

int RenderSlime::getColorMultiplier(EntityLiving* entityLiving, float f, float f1) {
    return getSlimeColorMultiplier((EntitySlime*)entityLiving, f, f1);
}

int RenderSlime::getSlimeColorMultiplier(EntitySlime* entitySlime, float f, float f1) {
    return RenderLiving::getColorMultiplier(entitySlime, f, f1);
}
