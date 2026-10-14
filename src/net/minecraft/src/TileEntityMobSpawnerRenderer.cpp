#include "TileEntityMobSpawnerRenderer.h"

#include "TileEntityMobSpawner.h"
#include "EntityList.h"
#include "World.h"
#include "RenderManager.h"
#include "Entity.h"

#include "pc/OpenGL.h"

TileEntityMobSpawnerRenderer::TileEntityMobSpawnerRenderer()
{
}

void TileEntityMobSpawnerRenderer::renderTileEntityMobSpawner(TileEntityMobSpawner *tileentitymobspawner, double d, double d1, double d2, float f)
{
    glPushMatrix();
    glTranslatef((float)d + 0.5f, (float)d1, (float)d2 + 0.5f);
    Entity *entity = nullptr;
    auto it = entityHashMap.find(tileentitymobspawner->getMobID());
    if (it == entityHashMap.end())
    {
        entity = EntityList::createEntityInWorld(tileentitymobspawner->getMobID(), nullptr);
        entityHashMap[tileentitymobspawner->getMobID()] = entity;
    }
    else
    {
        entity = it->second;
    }
    if (entity != nullptr)
    {
        entity->setWorld(tileentitymobspawner->worldObj);
        float f1 = 0.4375f;
        glTranslatef(0.0f, 0.4f, 0.0f);
        glRotatef((float)(tileentitymobspawner->yaw2 + (tileentitymobspawner->yaw - tileentitymobspawner->yaw2) * (double)f) * 10.0f, 0.0f, 1.0f, 0.0f);
        glRotatef(-30.0f, 1.0f, 0.0f, 0.0f);
        glTranslatef(0.0f, -0.4f, 0.0f);
        glScalef(f1, f1, f1);
        entity->setLocationAndAngles(d, d1, d2, 0.0f, 0.0f);
        RenderManager::instance->renderEntityWithPosYaw(entity, 0.0, 0.0, 0.0, 0.0f, f);
    }
    glPopMatrix();
}

void TileEntityMobSpawnerRenderer::renderTileEntityAt(TileEntity* tileentity, double d, double d1, double d2, float f)
{
    renderTileEntityMobSpawner((TileEntityMobSpawner *)tileentity, d, d1, d2, f);
}
