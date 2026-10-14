// ============================================================================
// FILE: Render.cpp
// ============================================================================
#include "Render.h"
#include "Block.h"
#include "GLAllocation.h"

Render::Render() {
    modelBase = new ModelBiped();
    renderBlocks = new RenderBlocks();
    shadowSize = 0.0f;
    shadowOpaque = 1.0f;
    renderManager = nullptr;
}

Render::~Render() {
    delete modelBase;
    delete renderBlocks;
}

void Render::loadTexture(const std::string& path) {
    RenderEngine* renderengine = renderManager->renderEngine;
    renderengine->bindTexture(renderengine->getTexture(path));
}

bool Render::loadDownloadableImageTexture(const std::string& url, const std::string& fallback) {
    if (renderManager == nullptr || renderManager->renderEngine == nullptr)
        return false;
    RenderEngine* renderengine = renderManager->renderEngine;
    int i = renderengine->getTextureForDownloadableImage(url, fallback);
    if (i >= 0) {
        renderengine->bindTexture(i);
        return true;
    }
    return false;
}

void Render::renderEntityOnFire(Entity* entity, double x, double y, double z, float partialTicks) {
    glDisable(GL_LIGHTING);
    int i = Block::fire->blockIndexInTexture;
    int j = (i & 0xf) << 4;
    int k = i & 0xf0;
    float f1 = (float)j / 256.0f;
    float f3 = ((float)j + 15.99f) / 256.0f;
    float f5 = (float)k / 256.0f;
    float f7 = ((float)k + 15.99f) / 256.0f;
    glPushMatrix();
    glTranslatef((float)x, (float)y, (float)z);
    float f9 = entity->width * 1.4f;
    glScalef(f9, f9, f9);
    loadTexture("/terrain.png");
    Tessellator* tessellator = &Tessellator::instance;
    float f10 = 0.5f;
    float f11 = 0.0f;
    float f12 = entity->height / f9;
    float f13 = (float)(entity->posY - entity->boundingBox->minY);
    glRotatef(-renderManager->playerViewY, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.0f, 0.0f, -0.3f + (float)(int)f12 * 0.02f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    float f14 = 0.0f;
    int l = 0;
    tessellator->startDrawingQuads();
    while (f12 > 0.0f) {
        float f2, f4, f6, f8;
        if (l % 2 == 0) {
            f2 = (float)j / 256.0f;
            f4 = ((float)j + 15.99f) / 256.0f;
            f6 = (float)k / 256.0f;
            f8 = ((float)k + 15.99f) / 256.0f;
        } else {
            f2 = (float)j / 256.0f;
            f4 = ((float)j + 15.99f) / 256.0f;
            f6 = (float)(k + 16) / 256.0f;
            f8 = ((float)(k + 16) + 15.99f) / 256.0f;
        }
        if ((l / 2) % 2 == 0) {
            float f15 = f4;
            f4 = f2;
            f2 = f15;
        }
        tessellator->addVertexWithUV(f10 - f11, 0.0f - f13, f14, f4, f8);
        tessellator->addVertexWithUV(-f10 - f11, 0.0f - f13, f14, f2, f8);
        tessellator->addVertexWithUV(-f10 - f11, 1.4f - f13, f14, f2, f6);
        tessellator->addVertexWithUV(f10 - f11, 1.4f - f13, f14, f4, f6);
        f12 -= 0.45f;
        f13 -= 0.45f;
        f10 *= 0.9f;
        f14 += 0.03f;
        l++;
    }
    tessellator->draw();
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void Render::renderShadow(Entity* entity, double x, double y, double z, float opacity, float partialTicks) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    RenderEngine* renderengine = renderManager->renderEngine;
    renderengine->bindTexture(renderengine->getTexture("%clamp%/misc/shadow.png"));
    World* world = getWorldFromRenderManager();
    glDepthMask(GL_FALSE);
    float f2 = shadowSize;
    double d3 = entity->lastTickPosX + (entity->posX - entity->lastTickPosX) * (double)partialTicks;
    double d4 = entity->lastTickPosY + (entity->posY - entity->lastTickPosY) * (double)partialTicks + (double)entity->getShadowSize();
    double d5 = entity->lastTickPosZ + (entity->posZ - entity->lastTickPosZ) * (double)partialTicks;
    int i = MathHelper::floor_double(d3 - (double)f2);
    int j = MathHelper::floor_double(d3 + (double)f2);
    int k = MathHelper::floor_double(d4 - (double)f2);
    int l = MathHelper::floor_double(d4);
    int i1 = MathHelper::floor_double(d5 - (double)f2);
    int j1 = MathHelper::floor_double(d5 + (double)f2);
    double d6 = x - d3;
    double d7 = y - d4;
    double d8 = z - d5;
    Tessellator* tessellator = &Tessellator::instance;
    tessellator->startDrawingQuads();
    for (int k1 = i; k1 <= j; k1++) {
        for (int l1 = k; l1 <= l; l1++) {
            for (int i2 = i1; i2 <= j1; i2++) {
                int j2 = world->getBlockId(k1, l1 - 1, i2);
                if (j2 > 0 && world->getBlockLightValue(k1, l1, i2) > 3) {
                    renderShadowOnBlock(Block::blocksList[j2], x, y + (double)entity->getShadowSize(), z, k1, l1, i2, opacity, f2, d6, d7 + (double)entity->getShadowSize(), d8);
                }
            }
        }
    }
    tessellator->draw();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

World* Render::getWorldFromRenderManager() {
    return renderManager->worldObj;
}

void Render::renderShadowOnBlock(Block* block, double x, double y, double z, int blockX, int blockY, int blockZ, float opacity, float shadowSize, double offsetX, double offsetY, double offsetZ) {
    Tessellator* tessellator = &Tessellator::instance;
    if (!block->renderAsNormalBlock()) {
        return;
    }
    double d6 = ((double)opacity - (y - ((double)blockY + offsetY)) / 2.0) * 0.5 * (double)getWorldFromRenderManager()->getLightBrightness(blockX, blockY, blockZ);
    if (d6 < 0.0) {
        return;
    }
    if (d6 > 1.0) {
        d6 = 1.0;
    }
    tessellator->setColorRGBA_F(1.0f, 1.0f, 1.0f, (float)d6);
    double d7 = (double)blockX + block->minX + offsetX;
    double d8 = (double)blockX + block->maxX + offsetX;
    double d9 = (double)blockY + block->minY + offsetY + 0.015625;
    double d10 = (double)blockZ + block->minZ + offsetZ;
    double d11 = (double)blockZ + block->maxZ + offsetZ;
    float f2 = (float)((x - d7) / 2.0 / (double)shadowSize + 0.5);
    float f3 = (float)((x - d8) / 2.0 / (double)shadowSize + 0.5);
    float f4 = (float)((z - d10) / 2.0 / (double)shadowSize + 0.5);
    float f5 = (float)((z - d11) / 2.0 / (double)shadowSize + 0.5);
    tessellator->addVertexWithUV(d7, d9, d10, f2, f4);
    tessellator->addVertexWithUV(d7, d9, d11, f2, f5);
    tessellator->addVertexWithUV(d8, d9, d11, f3, f5);
    tessellator->addVertexWithUV(d8, d9, d10, f3, f4);
}

void Render::renderOffsetAABB(AxisAlignedBB* aabb, double x, double y, double z) {
    glDisable(GL_TEXTURE_2D);
    Tessellator* tessellator = &Tessellator::instance;
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    tessellator->startDrawingQuads();
    tessellator->setTranslationD(x, y, z);
    tessellator->setNormal(0.0f, 0.0f, -1.0f);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->minZ);
    tessellator->setNormal(0.0f, 0.0f, 1.0f);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->maxZ);
    tessellator->setNormal(0.0f, -1.0f, 0.0f);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->maxZ);
    tessellator->setNormal(0.0f, 1.0f, 0.0f);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->minZ);
    tessellator->setNormal(-1.0f, 0.0f, 0.0f);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->minZ);
    tessellator->setNormal(1.0f, 0.0f, 0.0f);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->maxZ);
    tessellator->setTranslationD(0.0, 0.0, 0.0);
    tessellator->draw();
    glEnable(GL_TEXTURE_2D);
}

void Render::renderAABB(AxisAlignedBB* aabb) {
    Tessellator* tessellator = &Tessellator::instance;
    tessellator->startDrawingQuads();
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->minX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->minX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->minZ);
    tessellator->addVertex(aabb->maxX, aabb->maxY, aabb->maxZ);
    tessellator->addVertex(aabb->maxX, aabb->minY, aabb->maxZ);
    tessellator->draw();
}

void Render::doRenderShadowAndFire(Entity* entity, double x, double y, double z, float yaw, float partialTicks) {
    if (renderManager->options->fancyGraphics && shadowSize > 0.0f) {
        double d = renderManager->getDistanceToCamera(entity->posX, entity->posY, entity->posZ);
        float f = (float)((1.0 - d / 256.0) * (double)shadowOpaque);
        if (f > 0.0f) {
            renderShadow(entity, x, y, z, f, partialTicks);
        }
    }
    if (entity->isBurning()) {
        renderEntityOnFire(entity, x, y, z, partialTicks);
    }
}

FontRenderer* Render::getFontRendererFromRenderManager() {
    return renderManager->getFontRenderer();
}
