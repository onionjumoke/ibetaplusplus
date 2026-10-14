#include "RenderCreeper.h"
#include "ModelCreeper.h"
#include "EntityCreeper.h"
#include "MathHelper.h"
#include "ModelBase.h"
#include "EntityLiving.h"
#include <SDL2/SDL_opengl.h>

RenderCreeper::RenderCreeper() : RenderLiving(new ModelCreeper(), 0.5f) {
    chargedCreeperModel = new ModelCreeper(2.0f);
}

void RenderCreeper::updateCreeperScale(EntityCreeper* entityCreeper, float f) {
    float f1 = entityCreeper->setCreeperFlashTime(f);
    float f2 = 1.0f + MathHelper::sin(f1 * 100.0f) * f1 * 0.01f;
    if (f1 < 0.0f) {
        f1 = 0.0f;
    }
    if (f1 > 1.0f) {
        f1 = 1.0f;
    }
    f1 *= f1;
    f1 *= f1;
    float f3 = (1.0f + f1 * 0.4f) * f2;
    float f4 = (1.0f + f1 * 0.1f) / f2;
    glScalef(f3, f4, f3);
}

int RenderCreeper::updateCreeperColorMultiplier(EntityCreeper* entityCreeper, float f, float f1) {
    float f2 = entityCreeper->setCreeperFlashTime(f1);
    if ((int)(f2 * 10.0f) % 2 == 0) {
        return 0;
    }
    int i = (int)(f2 * 0.2f * 255.0f);
    if (i < 0) {
        i = 0;
    }
    if (i > 255) {
        i = 255;
    }
    char c = '\377';
    char c1 = '\377';
    char c2 = '\377';
    return i << 24 | c << 16 | c1 << 8 | c2;
}

bool RenderCreeper::shouldRenderCreeperPass(EntityCreeper* entityCreeper, int i, float f) {
    if (entityCreeper->getPowered()) {
        if (i == 1) {
            float f1 = (float)entityCreeper->ticksExisted + f;
            loadTexture("/armor/power.png");
            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
            float f2 = f1 * 0.01f;
            float f3 = f1 * 0.01f;
            glTranslatef(f2, f3, 0.0f);
            setRenderPassModel(chargedCreeperModel);
            glMatrixMode(GL_MODELVIEW);
            glEnable(GL_BLEND);
            float f4 = 0.5f;
            glColor4f(f4, f4, f4, 1.0f);
            glDisable(GL_LIGHTING);
            glBlendFunc(GL_ONE, GL_ONE);
            return true;
        }
        if (i == 2) {
            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
            glMatrixMode(GL_MODELVIEW);
            glEnable(GL_LIGHTING);
            glDisable(GL_BLEND);
        }
    }
    return false;
}

bool RenderCreeper::func_27007_b(EntityCreeper* entityCreeper, int i, float f) {
    return false;
}

void RenderCreeper::preRenderCallback(EntityLiving* entityLiving, float f) {
    updateCreeperScale((EntityCreeper*)entityLiving, f);
}

int RenderCreeper::getColorMultiplier(EntityLiving* entityLiving, float f, float f1) {
    return updateCreeperColorMultiplier((EntityCreeper*)entityLiving, f, f1);
}

bool RenderCreeper::shouldRenderPass(EntityLiving* entityLiving, int i, float f) {
    return shouldRenderCreeperPass((EntityCreeper*)entityLiving, i, f);
}

bool RenderCreeper::func_27005_b(EntityLiving* entityLiving, int i, float f) {
    return func_27007_b((EntityCreeper*)entityLiving, i, f);
}