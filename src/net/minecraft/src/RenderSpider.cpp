#include "RenderSpider.h"
#include "EntitySpider.h"
#include "ModelSpider.h"
#include "EntityLiving.h"
#include "Entity.h"
#include "MathHelper.h"

RenderSpider::RenderSpider() : RenderLiving(new ModelSpider(), 1.0f) {
}

void RenderSpider::renderSpider(EntitySpider* entitySpider, double d, double d1, double d2, float f, float f1) {
    RenderLiving::doRenderLiving(entitySpider, d, d1, d2, f, f1);
}

void RenderSpider::doRenderLiving(EntityLiving* entityLiving, double d, double d1, double d2, float f, float f1) {
    renderSpider((EntitySpider*)entityLiving, d, d1, d2, f, f1);
}

void RenderSpider::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    renderSpider((EntitySpider*)entity, d, d1, d2, f, f1);
}

float RenderSpider::getDeathMaxRotation(EntityLiving* entityLiving) {
    return getSpiderDeathMaxRotation((EntitySpider*)entityLiving);
}

float RenderSpider::getSpiderDeathMaxRotation(EntitySpider* entitySpider) {
    return 180.0f;
}

int RenderSpider::getColorMultiplier(EntityLiving* entityLiving, float f, float f1) {
    return getSpiderColorMultiplier((EntitySpider*)entityLiving, f, f1);
}

int RenderSpider::getSpiderColorMultiplier(EntitySpider* entitySpider, float f, float f1) {
    return RenderLiving::getColorMultiplier(entitySpider, f, f1);
}
