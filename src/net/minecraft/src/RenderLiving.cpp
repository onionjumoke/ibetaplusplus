#include "RenderLiving.h"
#include "ModelBase.h"
#include "EntityLiving.h"
#include "MathHelper.h"
#include "RenderManager.h"
#include "Tessellator.h"
#include "FontRenderer.h"
#include "client/Minecraft.h"
#include <SDL2/SDL_opengl.h>

RenderLiving::RenderLiving(ModelBase* modelbase, float f) {
    mainModel = modelbase;
    shadowSize = f;
    renderPassModel = nullptr;
}

void RenderLiving::setRenderPassModel(ModelBase* modelbase) {
    renderPassModel = modelbase;
}

void RenderLiving::doRenderLiving(EntityLiving* entityliving, double d, double d1, double d2, float f, float f1) {
    glPushMatrix();
    glDisable(GL_CULL_FACE);
    mainModel->onGround = getSwingProgress(entityliving, f1);
    if (renderPassModel != nullptr) {
        renderPassModel->onGround = mainModel->onGround;
    }
    mainModel->isRiding = entityliving->isRiding();
    if (renderPassModel != nullptr) {
        renderPassModel->isRiding = mainModel->isRiding;
    }
    try {
        float f2 = entityliving->prevRenderYawOffset + (entityliving->renderYawOffset - entityliving->prevRenderYawOffset) * f1;
        float f3 = entityliving->prevRotationYaw + (entityliving->rotationYaw - entityliving->prevRotationYaw) * f1;
        float f4 = entityliving->prevRotationPitch + (entityliving->rotationPitch - entityliving->prevRotationPitch) * f1;
        translateEntity(entityliving, d, d1, d2);
        float f5 = getTicksExisted(entityliving, f1);
        rotateCorpse(entityliving, f5, f2, f1);
        float f6 = 0.0625f;
        glEnable(GL_RESCALE_NORMAL);
        glScalef(-1.0f, -1.0f, 1.0f);
        preRenderCallback(entityliving, f1);
        glTranslatef(0.0f, -24.0f * f6 - 0.0078125f, 0.0f);
        float f7 = entityliving->field_705_Q + (entityliving->field_704_R - entityliving->field_705_Q) * f1;
        float f8 = entityliving->field_703_S - entityliving->field_704_R * (1.0f - f1);
        if (f7 > 1.0f) {
            f7 = 1.0f;
        }
        loadDownloadableImageTexture(entityliving->skinUrl, entityliving->getEntityTexture());
        glEnable(GL_ALPHA_TEST);
        mainModel->setLivingAnimations(entityliving, f8, f7, f1);
        mainModel->render(f8, f7, f5, f3 - f2, f4, f6);
        for (int i = 0; i < 4; i++) {
            if (shouldRenderPass(entityliving, i, f1)) {
                renderPassModel->render(f8, f7, f5, f3 - f2, f4, f6);
                glDisable(GL_BLEND);
                glEnable(GL_ALPHA_TEST);
            }
        }

        renderEquippedItems(entityliving, f1);
        float f9 = entityliving->getEntityBrightness(f1);
        int j = getColorMultiplier(entityliving, f9, f1);
        if ((j >> 24 & 0xff) > 0 || entityliving->hurtTime > 0 || entityliving->deathTime > 0) {
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_ALPHA_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDepthFunc(GL_EQUAL);
            if (entityliving->hurtTime > 0 || entityliving->deathTime > 0) {
                glColor4f(f9, 0.0f, 0.0f, 0.4f);
                mainModel->render(f8, f7, f5, f3 - f2, f4, f6);
                for (int k = 0; k < 4; k++) {
                    if (func_27005_b(entityliving, k, f1)) {
                        glColor4f(f9, 0.0f, 0.0f, 0.4f);
                        renderPassModel->render(f8, f7, f5, f3 - f2, f4, f6);
                    }
                }
            }
            if ((j >> 24 & 0xff) > 0) {
                float f10 = (float)(j >> 16 & 0xff) / 255.0f;
                float f11 = (float)(j >> 8 & 0xff) / 255.0f;
                float f12 = (float)(j & 0xff) / 255.0f;
                float f13 = (float)(j >> 24 & 0xff) / 255.0f;
                glColor4f(f10, f11, f12, f13);
                mainModel->render(f8, f7, f5, f3 - f2, f4, f6);
                for (int l = 0; l < 4; l++) {
                    if (func_27005_b(entityliving, l, f1)) {
                        glColor4f(f10, f11, f12, f13);
                        renderPassModel->render(f8, f7, f5, f3 - f2, f4, f6);
                    }
                }
            }
            glDepthFunc(GL_LEQUAL);
            glDisable(GL_BLEND);
            glEnable(GL_ALPHA_TEST);
            glEnable(GL_TEXTURE_2D);
        }
        glDisable(GL_RESCALE_NORMAL);
    }
    catch (...) {
        // exception.printStackTrace();
    }
    glEnable(GL_CULL_FACE);
    glPopMatrix();
    passSpecialRender(entityliving, d, d1, d2);
}

void RenderLiving::translateEntity(EntityLiving* entityliving, double d, double d1, double d2) {
    glTranslatef((float)d, (float)d1, (float)d2);
}

void RenderLiving::rotateCorpse(EntityLiving* entityliving, float f, float f1, float f2) {
    glRotatef(180.0f - f1, 0.0f, 1.0f, 0.0f);
    if (entityliving->deathTime > 0) {
        float f3 = ((((float)entityliving->deathTime + f2) - 1.0f) / 20.0f) * 1.6f;
        f3 = MathHelper::sqrt_float(f3);
        if (f3 > 1.0f) {
            f3 = 1.0f;
        }
        glRotatef(f3 * getDeathMaxRotation(entityliving), 0.0f, 0.0f, 1.0f);
    }
}

float RenderLiving::getSwingProgress(EntityLiving* entityliving, float f) {
    return entityliving->getSwingProgress(f);
}

float RenderLiving::getTicksExisted(EntityLiving* entityliving, float f) {
    return (float)entityliving->ticksExisted + f;
}

void RenderLiving::renderEquippedItems(EntityLiving* entityliving, float f) {
}

bool RenderLiving::func_27005_b(EntityLiving* entityliving, int i, float f) {
    return shouldRenderPass(entityliving, i, f);
}

bool RenderLiving::shouldRenderPass(EntityLiving* entityliving, int i, float f) {
    return false;
}

float RenderLiving::getDeathMaxRotation(EntityLiving* entityliving) {
    return 90.0f;
}

int RenderLiving::getColorMultiplier(EntityLiving* entityliving, float f, float f1) {
    return 0;
}

void RenderLiving::preRenderCallback(EntityLiving* entityliving, float f) {
}

void RenderLiving::passSpecialRender(EntityLiving* entityliving, double d, double d1, double d2) {
    if (Minecraft::isDebugInfoEnabled()) {
        renderLivingLabel(entityliving, std::to_string(entityliving->entityId), d, d1, d2, 64);
    }
}

void RenderLiving::renderLivingLabel(EntityLiving* entityliving, std::string s, double d, double d1, double d2, int i) {
    float f = entityliving->getDistanceToEntity(renderManager->livingPlayer);
    if (f > (float)i) {
        return;
    }
    FontRenderer* fontrenderer = getFontRendererFromRenderManager();
    float f1 = 1.6f;
    float f2 = 0.01666667f * f1;
    glPushMatrix();
    glTranslatef((float)d + 0.0f, (float)d1 + 2.3f, (float)d2);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glRotatef(-renderManager->playerViewY, 0.0f, 1.0f, 0.0f);
    glRotatef(renderManager->playerViewX, 1.0f, 0.0f, 0.0f);
    glScalef(-f2, -f2, f2);
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Tessellator* tessellator = &Tessellator::instance;
    int byte0 = 0;
    if (s == "deadmau5") {
        byte0 = -10;
    }
    glDisable(GL_TEXTURE_2D);
    tessellator->startDrawingQuads();
    int j = fontrenderer->getStringWidth(s) / 2;
    tessellator->setColorRGBA_F(0.0f, 0.0f, 0.0f, 0.25f);
    tessellator->addVertex(-j - 1, -1 + byte0, 0.0);
    tessellator->addVertex(-j - 1, 8 + byte0, 0.0);
    tessellator->addVertex(j + 1, 8 + byte0, 0.0);
    tessellator->addVertex(j + 1, -1 + byte0, 0.0);
    tessellator->draw();
    glEnable(GL_TEXTURE_2D);
    fontrenderer->drawString(s, -fontrenderer->getStringWidth(s) / 2, byte0, 0x20ffffff);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    fontrenderer->drawString(s, -fontrenderer->getStringWidth(s) / 2, byte0, -1);
    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glPopMatrix();
}

void RenderLiving::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    doRenderLiving((EntityLiving*)entity, d, d1, d2, f, f1);
}
