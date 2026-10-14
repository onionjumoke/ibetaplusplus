#ifndef RENDERCREEPER_H
#define RENDERCREEPER_H

#include "RenderLiving.h"

class EntityCreeper;
class ModelBase;
class ModelCreeper;

class RenderCreeper : public RenderLiving {
public:
    RenderCreeper();

protected:
    void updateCreeperScale(EntityCreeper* entityCreeper, float f);

    int updateCreeperColorMultiplier(EntityCreeper* entityCreeper, float f, float f1);

    bool shouldRenderCreeperPass(EntityCreeper* entityCreeper, int i, float f);

    bool func_27007_b(EntityCreeper* entityCreeper, int i, float f);

    virtual void preRenderCallback(EntityLiving* entityLiving, float f) override;

    virtual int getColorMultiplier(EntityLiving* entityLiving, float f, float f1) override;

    virtual bool shouldRenderPass(EntityLiving* entityLiving, int i, float f) override;

    virtual bool func_27005_b(EntityLiving* entityLiving, int i, float f) override;

private:
    ModelBase* chargedCreeperModel;
};

#endif