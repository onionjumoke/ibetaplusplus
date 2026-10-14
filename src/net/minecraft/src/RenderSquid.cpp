#include "RenderSquid.h"
#include "EntitySquid.h"
#include "ModelBase.h"
#include "EntityLiving.h"
#include "Entity.h"
#include "MathHelper.h"

RenderSquid::RenderSquid(ModelBase* modelBase, float f) : RenderLiving(modelBase, f) {
}

void RenderSquid::renderSquid(EntitySquid* entitySquid, double d, double d1, double d2, float f, float f1) {
    RenderLiving::doRenderLiving(entitySquid, d, d1, d2, f, f1);
}

void RenderSquid::doRenderLiving(EntityLiving* entityLiving, double d, double d1, double d2, float f, float f1) {
    renderSquid((EntitySquid*)entityLiving, d, d1, d2, f, f1);
}

void RenderSquid::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    renderSquid((EntitySquid*)entity, d, d1, d2, f, f1);
}

void RenderSquid::rotateCorpse(EntityLiving* entityLiving, float f, float f1, float f2) {
    rotateSquidCorpse((EntitySquid*)entityLiving, f, f1, f2);
}

void RenderSquid::rotateSquidCorpse(EntitySquid* entitySquid, float f, float f1, float f2) {
    float f3 = entitySquid->field_21088_b + (entitySquid->field_21089_a - entitySquid->field_21088_b) * f2;
    float f4 = entitySquid->field_21086_f + (entitySquid->field_21087_c - entitySquid->field_21086_f) * f2;
    glTranslatef(0.0f, 0.5f, 0.0f);
    glRotatef(180.0f - f1, 0.0f, 1.0f, 0.0f);
    glRotatef(f4, 1.0f, 0.0f, 0.0f);
    glRotatef(f3 * 10.0f, 1.0f, 0.0f, 0.0f);
    glTranslatef(0.0f, -1.2f, 0.0f);
}
