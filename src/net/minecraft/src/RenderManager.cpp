#include "RenderManager.h"
#include "Render.h"
#include "World.h"
#include "Block.h"
#include "FontRenderer.h"
#include "RenderEngine.h"
#include "ItemRenderer.h"
#include "GameSettings.h"
#include "EntityLiving.h"
#include "Entity.h"
#include "MathHelper.h"

#include "EntitySpider.h"
#include "RenderSpider.h"
#include "EntityPig.h"
#include "RenderPig.h"
#include "ModelPig.h"
#include "EntitySheep.h"
#include "RenderSheep.h"
#include "ModelSheep2.h"
#include "ModelSheep1.h"
#include "EntityCow.h"
#include "RenderCow.h"
#include "ModelCow.h"
#include "EntityWolf.h"
#include "RenderWolf.h"
#include "ModelWolf.h"
#include "EntityChicken.h"
#include "RenderChicken.h"
#include "ModelChicken.h"
#include "EntityCreeper.h"
#include "RenderCreeper.h"
#include "EntitySkeleton.h"
#include "RenderBiped.h"
#include "ModelSkeleton.h"
#include "EntityZombie.h"
#include "ModelZombie.h"
#include "EntitySlime.h"
#include "RenderSlime.h"
#include "ModelSlime.h"
#include "EntityPlayer.h"
#include "EntityPlayerSP.h"
#include "EntityClientPlayerMP.h"
#include "EntityOtherPlayerMP.h"
#include "RenderPlayer.h"
#include "EntityGiantZombie.h"
#include "RenderGiantZombie.h"
#include "EntityGhast.h"
#include "RenderGhast.h"
#include "EntitySquid.h"
#include "RenderSquid.h"
#include "ModelSquid.h"
#include "ModelBiped.h"
#include "EntityPainting.h"
#include "RenderPainting.h"
#include "EntityArrow.h"
#include "RenderArrow.h"
#include "EntitySnowball.h"
#include "RenderSnowball.h"
#include "EntityEgg.h"
#include "Item.h"
#include "EntityFireball.h"
#include "RenderFireball.h"
#include "EntityItem.h"
#include "RenderItem.h"
#include "EntityTNTPrimed.h"
#include "RenderTNTPrimed.h"
#include "EntityFallingSand.h"
#include "RenderFallingSand.h"
#include "EntityMinecart.h"
#include "RenderMinecart.h"
#include "EntityBoat.h"
#include "RenderBoat.h"
#include "EntityFish.h"
#include "RenderFish.h"
#include "EntityLightningBolt.h"
#include "RenderLightningBolt.h"
#include "RenderEntity.h"

RenderManager* RenderManager::instance = nullptr;
double RenderManager::renderPosX = 0.0;
double RenderManager::renderPosY = 0.0;
double RenderManager::renderPosZ = 0.0;

RenderManager::RenderManager() {
    instance      = this;
    renderEngine  = nullptr;
    fontRenderer  = nullptr;
    worldObj      = nullptr;
    livingPlayer  = nullptr;
    itemRenderer  = nullptr;
    options       = nullptr;
    playerViewY   = 0.0f;
    playerViewX   = 0.0f;
    field_1222_l  = 0.0;
    field_1221_m  = 0.0;
    field_1220_n  = 0.0;
    registerRenderers();
}

void RenderManager::registerRenderers() {
    entityRenderMap[std::type_index(typeid(EntitySpider))] = new RenderSpider();
    entityRenderMap[std::type_index(typeid(EntityPig))] = new RenderPig(new ModelPig(), new ModelPig(0.5f), 0.7f);
    entityRenderMap[std::type_index(typeid(EntitySheep))] = new RenderSheep(new ModelSheep2(), new ModelSheep1(), 0.7f);
    entityRenderMap[std::type_index(typeid(EntityCow))] = new RenderCow(new ModelCow(), 0.7f);
    entityRenderMap[std::type_index(typeid(EntityWolf))] = new RenderWolf(new ModelWolf(), 0.5f);
    entityRenderMap[std::type_index(typeid(EntityChicken))] = new RenderChicken(new ModelChicken(), 0.3f);
    entityRenderMap[std::type_index(typeid(EntityCreeper))] = new RenderCreeper();
    entityRenderMap[std::type_index(typeid(EntitySkeleton))] = new RenderBiped(new ModelSkeleton(), 0.5f);
    entityRenderMap[std::type_index(typeid(EntityZombie))] = new RenderBiped(new ModelZombie(), 0.5f);
    entityRenderMap[std::type_index(typeid(EntitySlime))] = new RenderSlime(new ModelSlime(16), new ModelSlime(0), 0.25f);
    RenderPlayer *renderPlayer = new RenderPlayer();
    entityRenderMap[std::type_index(typeid(EntityPlayer))]         = renderPlayer;
    entityRenderMap[std::type_index(typeid(EntityPlayerSP))]       = renderPlayer;
    entityRenderMap[std::type_index(typeid(EntityClientPlayerMP))] = renderPlayer;
    entityRenderMap[std::type_index(typeid(EntityOtherPlayerMP))]  = renderPlayer;
    entityRenderMap[std::type_index(typeid(EntityGiantZombie))] = new RenderGiantZombie(new ModelZombie(), 0.5f, 6.0f);
    entityRenderMap[std::type_index(typeid(EntityGhast))] = new RenderGhast();
    entityRenderMap[std::type_index(typeid(EntitySquid))] = new RenderSquid(new ModelSquid(), 0.7f);
    entityRenderMap[std::type_index(typeid(EntityLiving))] = new RenderLiving(new ModelBiped(), 0.5f);
    entityRenderMap[std::type_index(typeid(Entity))] = new RenderEntity();
    entityRenderMap[std::type_index(typeid(EntityPainting))] = new RenderPainting();
    entityRenderMap[std::type_index(typeid(EntityArrow))] = new RenderArrow();
    entityRenderMap[std::type_index(typeid(EntitySnowball))] = new RenderSnowball(Item::snowball->getIconFromDamage(0));
    entityRenderMap[std::type_index(typeid(EntityEgg))] = new RenderSnowball(Item::egg->getIconFromDamage(0));
    entityRenderMap[std::type_index(typeid(EntityFireball))] = new RenderFireball();
    entityRenderMap[std::type_index(typeid(EntityItem))] = new RenderItem();
    entityRenderMap[std::type_index(typeid(EntityTNTPrimed))] = new RenderTNTPrimed();
    entityRenderMap[std::type_index(typeid(EntityFallingSand))] = new RenderFallingSand();
    entityRenderMap[std::type_index(typeid(EntityMinecart))] = new RenderMinecart();
    entityRenderMap[std::type_index(typeid(EntityBoat))] = new RenderBoat();
    entityRenderMap[std::type_index(typeid(EntityFish))] = new RenderFish();
    entityRenderMap[std::type_index(typeid(EntityLightningBolt))] = new RenderLightningBolt();

    for (auto& pair : entityRenderMap) {
        pair.second->setRenderManager(this);
    }
}

Render* RenderManager::getEntityClassRenderObject(std::type_index classType) {
    auto it = entityRenderMap.find(classType);
    if (it != entityRenderMap.end()) {
        return it->second;
    }
    // Try parent classes
    if (classType == std::type_index(typeid(Entity))) {
        return nullptr;
    }
    // For simplicity, return default entity renderer
    return entityRenderMap[std::type_index(typeid(Entity))];
}

Render* RenderManager::getEntityRenderObject(Entity* entity) {
    return getEntityClassRenderObject(std::type_index(typeid(*entity)));
}

void RenderManager::cacheActiveRenderInfo(World* world, RenderEngine* renderengine, FontRenderer* fontrenderer, EntityLiving* entityliving, GameSettings* gamesettings, float f) {
    worldObj = world;
    renderEngine = renderengine;
    options = gamesettings;
    livingPlayer = entityliving;
    fontRenderer = fontrenderer;
    if (entityliving->isPlayerSleeping()) {
        int i = world->getBlockId(MathHelper::floor_double(entityliving->posX), MathHelper::floor_double(entityliving->posY), MathHelper::floor_double(entityliving->posZ));
        if (i == Block::blockBed->blockID) {
            int j = world->getBlockMetadata(MathHelper::floor_double(entityliving->posX), MathHelper::floor_double(entityliving->posY), MathHelper::floor_double(entityliving->posZ));
            int k = j & 3;
            playerViewY = k * 90 + 180;
            playerViewX = 0.0f;
        }
    } else {
        playerViewY = entityliving->prevRotationYaw + (entityliving->rotationYaw - entityliving->prevRotationYaw) * f;
        playerViewX = entityliving->prevRotationPitch + (entityliving->rotationPitch - entityliving->prevRotationPitch) * f;
    }
    field_1222_l = entityliving->lastTickPosX + (entityliving->posX - entityliving->lastTickPosX) * (double)f;
    field_1221_m = entityliving->lastTickPosY + (entityliving->posY - entityliving->lastTickPosY) * (double)f;
    field_1220_n = entityliving->lastTickPosZ + (entityliving->posZ - entityliving->lastTickPosZ) * (double)f;
}

void RenderManager::renderEntity(Entity* entity, float f) {
    double d = entity->lastTickPosX + (entity->posX - entity->lastTickPosX) * (double)f;
    double d1 = entity->lastTickPosY + (entity->posY - entity->lastTickPosY) * (double)f;
    double d2 = entity->lastTickPosZ + (entity->posZ - entity->lastTickPosZ) * (double)f;
    float f1 = entity->prevRotationYaw + (entity->rotationYaw - entity->prevRotationYaw) * f;
    float f2 = entity->getEntityBrightness(f);
    glColor3f(f2, f2, f2);
    renderEntityWithPosYaw(entity, d - renderPosX, d1 - renderPosY, d2 - renderPosZ, f1, f);
}

void RenderManager::renderEntityWithPosYaw(Entity* entity, double d, double d1, double d2, float f, float f1) {
    Render* render = getEntityRenderObject(entity);
    if (render != nullptr) {
        render->doRender(entity, d, d1, d2, f, f1);
        render->doRenderShadowAndFire(entity, d, d1, d2, f, f1);
    }
}

void RenderManager::setWorld(World* world) {
    worldObj = world;
}

double RenderManager::getDistanceToCamera(double d, double d1, double d2) {
    double d3 = d - field_1222_l;
    double d4 = d1 - field_1221_m;
    double d5 = d2 - field_1220_n;
    return d3 * d3 + d4 * d4 + d5 * d5;
}

FontRenderer* RenderManager::getFontRenderer() {
    return fontRenderer;
}
