#include "RenderPig.h"
#include "EntityPig.h"
#include "ModelBase.h"
#include "EntityLiving.h"
#include "Entity.h"

RenderPig::RenderPig(ModelBase* modelBase, ModelBase* modelBase2, float shadowSize) : RenderLiving(modelBase, shadowSize) {
    setRenderPassModel(modelBase2);
}

void RenderPig::renderPig(EntityPig* entityPig, double d, double d1, double d2, float f, float f1) {
    RenderLiving::doRenderLiving(entityPig, d, d1, d2, f, f1);
}

void RenderPig::doRenderLiving(EntityLiving* entityLiving, double d, double d1, double d2, float f, float f1) {
    renderPig((EntityPig*)entityLiving, d, d1, d2, f, f1);
}

void RenderPig::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    renderPig((EntityPig*)entity, d, d1, d2, f, f1);
}

void RenderPig::preRenderCallback(EntityLiving* entityLiving, float f) {
    glScalef(0.9375f, 0.9375f, 0.9375f);
}