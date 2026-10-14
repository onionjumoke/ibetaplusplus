#ifndef RENDERSLIME_H
#define RENDERSLIME_H

#include "RenderLiving.h"

class EntitySlime;
class ModelBase;

class RenderSlime : public RenderLiving {
public:
    RenderSlime(ModelBase* modelBase, ModelBase* modelBase2, float f);

    void renderSlime(EntitySlime* entitySlime, double d, double d1, double d2, float f, float f1);

    virtual void doRenderLiving(EntityLiving* entityLiving, double d, double d1, double d2, float f, float f1) override;

    virtual void doRender(Entity* entity, double d, double d1, double d2, float f, float f1) override;

protected:
    virtual void preRenderCallback(EntityLiving* entityLiving, float f) override;

    void scaleSlime(EntitySlime* entitySlime, float f);

    virtual int getColorMultiplier(EntityLiving* entityLiving, float f, float f1) override;

    int getSlimeColorMultiplier(EntitySlime* entitySlime, float f, float f1);

private:
    ModelBase* scaleAmount;
};

#endif