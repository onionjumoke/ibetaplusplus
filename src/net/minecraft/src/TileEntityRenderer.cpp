#include "TileEntityRenderer.h"

#include "pc/OpenGL.h"
#include "TileEntity.h"
#include "TileEntitySpecialRenderer.h"
#include "TileEntitySign.h"
#include "TileEntitySignRenderer.h"
#include "TileEntityMobSpawner.h"
#include "TileEntityMobSpawnerRenderer.h"
#include "TileEntityPiston.h"
#include "TileEntityRendererPiston.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "World.h"
#include "EntityLiving.h"
#include <typeindex>

TileEntityRenderer TileEntityRenderer::instance;
double TileEntityRenderer::staticPlayerX = 0.0;
double TileEntityRenderer::staticPlayerY = 0.0;
double TileEntityRenderer::staticPlayerZ = 0.0;

TileEntityRenderer::TileEntityRenderer()
    : fontRenderer(nullptr)
    , renderEngine(nullptr)
    , worldObj(nullptr)
    , entityLivingPlayer(nullptr)
    , playerYaw(0.0f)
    , playerPitch(0.0f)
    , playerX(0.0)
    , playerY(0.0)
    , playerZ(0.0)
{
    specialRendererMap[typeid(TileEntitySign).name()]        = new TileEntitySignRenderer();
    specialRendererMap[typeid(TileEntityMobSpawner).name()]  = new TileEntityMobSpawnerRenderer();
    specialRendererMap[typeid(TileEntityPiston).name()]      = new TileEntityRendererPiston();

    for (auto &pair : specialRendererMap)
        if (pair.second)
            pair.second->setTileEntityRenderer(this);
}

void TileEntityRenderer::setWorld(World *world)
{
    worldObj = world;
    for (auto &pair : specialRendererMap)
        if (pair.second)
            pair.second->func_31069_a(world);
}

bool TileEntityRenderer::hasSpecialRenderer(TileEntity *tileentity)
{
    if (tileentity == nullptr) return false;
    return specialRendererMap.count(typeid(*tileentity).name()) > 0;
}

void TileEntityRenderer::renderTileEntity(TileEntity *tileentity, float f)
{
    if (tileentity->getDistanceFrom(playerX, playerY, playerZ) < 4096.0)
    {
        float f1 = worldObj->getLightBrightness(tileentity->xCoord, tileentity->yCoord, tileentity->zCoord);
        glColor3f(f1, f1, f1);
        renderTileEntityAt(tileentity,
            (double)tileentity->xCoord - staticPlayerX,
            (double)tileentity->yCoord - staticPlayerY,
            (double)tileentity->zCoord - staticPlayerZ, f);
    }
}

void TileEntityRenderer::renderTileEntityAt(TileEntity *tileentity, double d, double d1, double d2, float f)
{
    auto it = specialRendererMap.find(typeid(*tileentity).name());
    if (it != specialRendererMap.end() && it->second != nullptr)
        it->second->renderTileEntityAt(tileentity, d, d1, d2, f);
}

void TileEntityRenderer::cacheActiveRenderInfo(World *world, RenderEngine *renderengine, FontRenderer *fontrenderer, EntityLiving *entityliving, float f)
{
    if (worldObj != world)
        setWorld(world);
    renderEngine      = renderengine;
    entityLivingPlayer = entityliving;
    fontRenderer      = fontrenderer;
    playerYaw   = entityliving->prevRotationYaw   + (entityliving->rotationYaw   - entityliving->prevRotationYaw)   * f;
    playerPitch = entityliving->prevRotationPitch + (entityliving->rotationPitch - entityliving->prevRotationPitch) * f;
    playerX = entityliving->lastTickPosX + (entityliving->posX - entityliving->lastTickPosX) * (double)f;
    playerY = entityliving->lastTickPosY + (entityliving->posY - entityliving->lastTickPosY) * (double)f;
    playerZ = entityliving->lastTickPosZ + (entityliving->posZ - entityliving->lastTickPosZ) * (double)f;
}

void TileEntityRenderer::getPistonExtensionRender(World *world)
{
    worldObj = world;
}

void TileEntityRenderer::getPistonExtensionFacing()
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
}

void TileEntityRenderer::getPistonExtensionBounds(FontRenderer *fontrenderer)
{
    fontRenderer = fontrenderer;
}

void TileEntityRenderer::getPistonExtensionTexture(RenderEngine *renderengine)
{
    renderEngine = renderengine;
}
