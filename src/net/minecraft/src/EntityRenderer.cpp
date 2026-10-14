#include "EntityRenderer.h"
#include <atomic>

#ifdef MC_IOS
extern "C" int gles11MatrixCurDepth(int idx);
#else
static int gles11MatrixCurDepth(int) { return 0; }
#endif
#include "Minecraft.h"
#include "ItemRenderer.h"
#include "EntityLiving.h"
#include "MouseFilter.h"
#include "RenderGlobal.h"
#include "EffectRenderer.h"
#include "World.h"
#include "GameSettings.h"
#include "Config.h"
#include "PlayerController.h"
#include "MovingObjectPosition.h"
#include "Vec3D.h"
#include "PlayerControllerTest.h"
#include "AxisAlignedBB.h"
#include "Entity.h"
#include "Material.h"
#include "EntityPlayer.h"
#include "Block.h"
#include "EntityPlayerSP.h"
#include "MouseHelper.h"
#include "ScaledResolution.h"
#include "GuiIngame.h"
#include "GuiScreen.h"
#include "GuiParticle.h"
#include "ChunkProviderLoadOrGenerate.h"
#include "ClippingHelperImpl.h"
#include "Frustrum.h"
#include "ICamera.h"
#include "RenderEngine.h"
#include "RenderHelper.h"
#include "pc/lwjgl/Display.h"
#include "InventoryPlayer.h"
#include "WorldChunkManager.h"
#include "BiomeGenBase.h"
#include "EntitySmokeFX.h"
#include "EntityRainFX.h"
#include "Tessellator.h"
#include "WorldProvider.h"
#include "IChunkProvider.h"
#include "MathHelper.h"
#include "GLAllocation.h"

// GLU replacement para gluPerspective
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Static members
bool EntityRenderer::anaglyphEnabled = false;
int EntityRenderer::anaglyphField = 0;

// Helper para gluPerspective (LWJGL usa GLU, en C++ puro usamos nuestra propia implementacion)
static void perspectiveGL(double fovY, double aspect, double zNear, double zFar)
{
    double fH = tan(fovY * M_PI / 360.0) * zNear;  // half-height at near plane
    double fW = fH * aspect;                          // half-width = height * aspect
    glFrustum(-fW, fW, -fH, fH, zNear, zFar);
}

EntityRenderer::EntityRenderer(Minecraft* minecraft)
{
    farPlaneDistance = 0.0f;
    pointedEntity = nullptr;
    
    mouseFilterXAxis = new MouseFilter();
    mouseFilterYAxis = new MouseFilter();
    mouseFilterDummy1 = new MouseFilter();
    mouseFilterDummy2 = new MouseFilter();
    mouseFilterDummy3 = new MouseFilter();
    mouseFilterDummy4 = new MouseFilter();
    
    cameraDistance = 4.0f;       // field_22228_r
    prevCameraDistance = 4.0f;   // field_22227_s
    cameraYaw = 0.0f;            // field_22226_t
    prevCameraYaw = 0.0f;        // field_22225_u
    cameraPitch = 0.0f;          // field_22224_v
    prevCameraPitch = 0.0f;      // field_22223_w
    cameraRoll = 0.0f;           // field_22222_x
    prevCameraRoll = 0.0f;       // field_22221_y
    cameraRollTarget = 0.0f;     // field_22220_z
    prevCameraRollTarget = 0.0f; // field_22230_A
    
    cloudFog = false;
    cameraZoom = 1.0;
    cameraYawOffset = 0.0;
    cameraPitchOffset = 0.0;
    
    prevFrameTime = std::chrono::steady_clock::now();
    field_28133_I = 0;
    
    random.seed(static_cast<unsigned long>(std::chrono::steady_clock::now().time_since_epoch().count()));
    rainSoundCounter = 0;
    
    field_1394_b = 0;
    field_1393_c = 0;
    
    fogColorRed = 0.0f;
    fogColorGreen = 0.0f;
    fogColorBlue = 0.0f;
    fogColor2 = 0.0f;
    fogColor1 = 0.0f;
    
    rendererUpdateCount = 0;
    
    mc = minecraft;
    itemRenderer = new ItemRenderer(minecraft);
    updatedWorldProvider = nullptr;
}

// OptiFine: regenera worldProvider->lightBrightnessTable usando ofBrightness.
// brightness=0 reproduce vanilla (k=3, minLevel=0.05); brightness=1 aclara las
// zonas oscuras (k=0 -> rampa lineal hasta 1.0).
void EntityRenderer::updateWorldLightLevels()
{
    if (mc == nullptr)
        return;
    if (mc->theWorld == nullptr)
        return;
    if (mc->theWorld->worldProvider == nullptr)
        return;

    float brightness = mc->gameSettings->ofBrightness;
    float* lightLevels = mc->theWorld->worldProvider->lightBrightnessTable;
    float minLevel = 0.05f;
    if (mc->theWorld->worldProvider->hasNoSky)
        minLevel = 0.1f + brightness * 0.15f;
    float k = 3.0f * (1.0f - brightness);
    for (int i = 0; i <= 15; i++)
    {
        float f1 = 1.0f - (float)i / 15.0f;
        lightLevels[i] = (1.0f - f1) / (f1 * k + 1.0f) * (1.0f - minLevel) + minLevel;
    }
    Config::setLightLevels(lightLevels);
}

EntityRenderer::~EntityRenderer()
{
    delete mouseFilterXAxis;
    delete mouseFilterYAxis;
    delete mouseFilterDummy1;
    delete mouseFilterDummy2;
    delete mouseFilterDummy3;
    delete mouseFilterDummy4;
    delete itemRenderer;
}

void EntityRenderer::updateRenderer()
{
    // Interpolacion de niebla
    fogColor2 = fogColor1;
    prevCameraDistance = cameraDistance;
    prevCameraYaw = cameraYaw;
    prevCameraPitch = cameraPitch;
    prevCameraRoll = cameraRoll;
    prevCameraRollTarget = cameraRollTarget;
    
    if (mc->renderViewEntity == nullptr)
    {
        mc->renderViewEntity = mc->thePlayer;
    }
    
    // Calcular brillo de luz en la posicion de la entidad vista
    float lightBrightness = mc->theWorld->getLightBrightness(
        MathHelper::floor_double(mc->renderViewEntity->posX),
        MathHelper::floor_double(mc->renderViewEntity->posY),
        MathHelper::floor_double(mc->renderViewEntity->posZ)
    );
    
    float renderDistFactor = (float)(3 - mc->gameSettings->renderDistance) / 3.0f;
    float targetFog = lightBrightness * (1.0f - renderDistFactor) + renderDistFactor;
    
    fogColor1 += (targetFog - fogColor1) * 0.1f;
    rendererUpdateCount++;
    
    itemRenderer->updateEquippedItem();
    addRainParticles();
}

void EntityRenderer::getMouseOver(float partialTicks)
{
    if (mc->thePlayer == nullptr || mc->theWorld == nullptr)
    {
        return;
    }

    // The vanilla client always uses the local player as its camera entity.  A dimension
    // respawn replaces that C++ object and destroys the previous World explicitly (Java
    // merely let both be garbage-collected).  Re-establish both non-owning links at the
    // render boundary so rayTrace() can never follow the previous player's/world's pointer.
    mc->renderViewEntity = mc->thePlayer;
    mc->renderViewEntity->setWorld(mc->theWorld);
    
    double reachDistance = mc->playerController->getBlockReachDistance();
    delete mc->objectMouseOver;
    mc->objectMouseOver = mc->renderViewEntity->rayTrace(reachDistance, partialTicks);
    
    double entityReach = reachDistance;
    Vec3D* viewPos = mc->renderViewEntity->getPosition(partialTicks);
    
    if (mc->objectMouseOver != nullptr)
    {
        entityReach = mc->objectMouseOver->hitVec->distanceTo(viewPos);
    }
    
    // PlayerControllerTest tiene alcance extendido
    bool isTestController = dynamic_cast<PlayerControllerTest*>(mc->playerController) != nullptr;
    
    double blockReach;
    if (isTestController)
    {
        entityReach = reachDistance = 32.0;
        blockReach = reachDistance;
    }
    else
    {
        if (entityReach > 3.0)
        {
            entityReach = 3.0;
        }
        blockReach = entityReach;
    }
    
    Vec3D* lookVec = mc->renderViewEntity->getLook(partialTicks);
    Vec3D* lookEnd = viewPos->addVector(lookVec->xCoord * blockReach,
                                         lookVec->yCoord * blockReach,
                                         lookVec->zCoord * blockReach);
    
    pointedEntity = nullptr;
    float expandSize = 1.0f;
    
    std::vector<Entity*> entities = mc->theWorld->getEntitiesWithinAABBExcludingEntity(
        mc->renderViewEntity,
        mc->renderViewEntity->boundingBox->addCoord(lookVec->xCoord * blockReach,
                                                    lookVec->yCoord * blockReach,
                                                    lookVec->zCoord * blockReach)
            ->expand(expandSize, expandSize, expandSize)
    );
    
    double closestDistance = 0.0;
    
    for (size_t i = 0; i < entities.size(); i++)
    {
        Entity* entity = entities[i];
        
        if (!entity->canBeCollidedWith())
        {
            continue;
        }
        
        float collisionBorder = entity->getCollisionBorderSize();
        AxisAlignedBB* expandedBB = entity->boundingBox->expand(collisionBorder, collisionBorder, collisionBorder);

        MovingObjectPosition* mop = expandedBB->calculateIntercept(viewPos, lookEnd);

        if (expandedBB->isVecInside(viewPos))
        {
            if (0.0 < closestDistance || closestDistance == 0.0)
            {
                pointedEntity = entity;
                closestDistance = 0.0;
            }
            delete mop;
            continue;
        }
        
        if (mop == nullptr)
        {
            continue;
        }
        
        double hitDistance = viewPos->distanceTo(mop->hitVec);
        
        if (hitDistance < closestDistance || closestDistance == 0.0)
        {
            pointedEntity = entity;
            closestDistance = hitDistance;
        }
        delete mop;
    }
    
    if (pointedEntity != nullptr && !isTestController)
    {
        delete mc->objectMouseOver;
        mc->objectMouseOver = new MovingObjectPosition(pointedEntity);
    }
}

float EntityRenderer::getFOVModifier(float partialTicks)
{
    EntityLiving* entityliving = mc->renderViewEntity;
    float fov = (mc->gameSettings != nullptr) ? mc->gameSettings->ofFov : 70.0f;
    
    if (entityliving->isInsideOfMaterial(Material::water))
    {
        fov = 60.0f;
    }
    
    if (entityliving->health <= 0)
    {
        float deathTime = (float)entityliving->deathTime + partialTicks;
        fov /= (1.0f - 500.0f / (deathTime + 500.0f)) * 2.0f + 1.0f;
    }
    
    return fov + prevCameraRoll + (cameraRoll - prevCameraRoll) * partialTicks;
}

void EntityRenderer::hurtCameraEffect(float partialTicks)
{
    EntityLiving* entityliving = mc->renderViewEntity;
    float hurtTime = (float)entityliving->hurtTime - partialTicks;
    
    if (entityliving->health <= 0)
    {
        float deathTime = (float)entityliving->deathTime + partialTicks;
        glRotatef(40.0f - 8000.0f / (deathTime + 200.0f), 0.0f, 0.0f, 1.0f);
    }
    
    if (hurtTime < 0.0f)
    {
        return;
    }
    
    hurtTime /= entityliving->maxHurtTime;
    hurtTime = MathHelper::sin(hurtTime * hurtTime * hurtTime * hurtTime * 3.141593f);
    
    float attackedYaw = entityliving->attackedAtYaw;
    
    glRotatef(-attackedYaw, 0.0f, 1.0f, 0.0f);
    glRotatef(-hurtTime * 14.0f, 0.0f, 0.0f, 1.0f);
    glRotatef(attackedYaw, 0.0f, 1.0f, 0.0f);
}

void EntityRenderer::setupViewBobbing(float partialTicks)
{
    EntityPlayer* entityplayer = dynamic_cast<EntityPlayer*>(mc->renderViewEntity);
    
    if (entityplayer == nullptr)
    {
        return;
    }
    
    float walkDelta = entityplayer->distanceWalkedModified - entityplayer->prevDistanceWalkedModified;
    float walkProgress = -(entityplayer->distanceWalkedModified + walkDelta * partialTicks);
    float bobAmount = entityplayer->field_775_e + (entityplayer->field_774_f - entityplayer->field_775_e) * partialTicks;
    float cameraPitchBob = entityplayer->cameraPitch + (entityplayer->field_9328_R - entityplayer->cameraPitch) * partialTicks;
    
    glTranslatef(MathHelper::sin(walkProgress * 3.141593f) * bobAmount * 0.5f,
                 -std::abs(MathHelper::cos(walkProgress * 3.141593f) * bobAmount),
                 0.0f);
    
    glRotatef(MathHelper::sin(walkProgress * 3.141593f) * bobAmount * 3.0f,
              0.0f, 0.0f, 1.0f);
    
    glRotatef(std::abs(MathHelper::cos(walkProgress * 3.141593f - 0.2f) * bobAmount) * 5.0f,
              1.0f, 0.0f, 0.0f);
    
    glRotatef(cameraPitchBob, 1.0f, 0.0f, 0.0f);
}

void EntityRenderer::orientCamera(float partialTicks)
{
    EntityLiving* entityliving = mc->renderViewEntity;
    float eyeHeight = entityliving->yOffset - 1.62f;
    
    double posX = entityliving->prevPosX + (entityliving->posX - entityliving->prevPosX) * (double)partialTicks;
    double posY = (entityliving->prevPosY + (entityliving->posY - entityliving->prevPosY) * (double)partialTicks) - (double)eyeHeight;
    double posZ = entityliving->prevPosZ + (entityliving->posZ - entityliving->prevPosZ) * (double)partialTicks;
    
    glRotatef(prevCameraRollTarget + (cameraRollTarget - prevCameraRollTarget) * partialTicks,
              0.0f, 0.0f, 1.0f);
    
    if (entityliving->isPlayerSleeping())
    {
        eyeHeight = (float)((double)eyeHeight + 1.0);
        glTranslatef(0.0f, 0.3f, 0.0f);
        
        if (!mc->gameSettings->field_22273_E)
        {
            int bedX = MathHelper::floor_double(entityliving->posX);
            int bedY = MathHelper::floor_double(entityliving->posY);
            int bedZ = MathHelper::floor_double(entityliving->posZ);
            
            int blockId = mc->theWorld->getBlockId(bedX, bedY, bedZ);
            
            if (blockId == Block::blockBed->blockID)
            {
                int meta = mc->theWorld->getBlockMetadata(bedX, bedY, bedZ);
                int direction = meta & 3;
                glRotatef((float)(direction * 90), 0.0f, 1.0f, 0.0f);
            }
            
            glRotatef(entityliving->prevRotationYaw + 
                      (entityliving->rotationYaw - entityliving->prevRotationYaw) * partialTicks + 180.0f,
                      0.0f, -1.0f, 0.0f);
            
            glRotatef(entityliving->prevRotationPitch + 
                      (entityliving->rotationPitch - entityliving->prevRotationPitch) * partialTicks,
                      -1.0f, 0.0f, 0.0f);
        }
    }
    else if (mc->gameSettings->thirdPersonView)
    {
        double camDist = prevCameraDistance + (cameraDistance - prevCameraDistance) * partialTicks;
        
        if (mc->gameSettings->field_22273_E)
        {
            float camYaw = prevCameraYaw + (cameraYaw - prevCameraYaw) * partialTicks;
            float camPitch = prevCameraPitch + (cameraPitch - prevCameraPitch) * partialTicks;
            
            glTranslatef(0.0f, 0.0f, (float)(-camDist));
            glRotatef(camPitch, 1.0f, 0.0f, 0.0f);
            glRotatef(camYaw, 0.0f, 1.0f, 0.0f);
        }
        else
        {
            float entityYaw = entityliving->rotationYaw;
            float entityPitch = entityliving->rotationPitch;
            
            double offsetX = (double)(-MathHelper::sin((entityYaw / 180.0f) * 3.141593f) * 
                                       MathHelper::cos((entityPitch / 180.0f) * 3.141593f)) * camDist;
            double offsetZ = (double)(MathHelper::cos((entityYaw / 180.0f) * 3.141593f) * 
                                       MathHelper::cos((entityPitch / 180.0f) * 3.141593f)) * camDist;
            double offsetY = (double)(-MathHelper::sin((entityPitch / 180.0f) * 3.141593f)) * camDist;
            
            // Raycast para evitar paredes en tercera persona
            for (int i = 0; i < 8; i++)
            {
                float offsetXSign = (float)((i & 1) * 2 - 1) * 0.1f;
                float offsetYSign = (float)((i >> 1 & 1) * 2 - 1) * 0.1f;
                float offsetZSign = (float)((i >> 2 & 1) * 2 - 1) * 0.1f;
                
                Vec3D* rayStart = Vec3D::createVector(
                    posX + (double)offsetXSign,
                    posY + (double)offsetYSign,
                    posZ + (double)offsetZSign
                );

                Vec3D* rayEnd = Vec3D::createVector(
                    (posX - offsetX) + (double)offsetXSign + (double)offsetZSign,
                    (posY - offsetY) + (double)offsetYSign,
                    (posZ - offsetZ) + (double)offsetZSign  // Nota: Java tiene bug aqui con +f8 dos veces
                );
                
                MovingObjectPosition* mop = mc->theWorld->rayTraceBlocks(rayStart, rayEnd);
                
                if (mop == nullptr)
                {
                    continue;
                }
                
                double hitDist = mop->hitVec->distanceTo(Vec3D::createVector(posX, posY, posZ));
                
                if (hitDist < camDist)
                {
                    camDist = hitDist;
                }
            }
            
            glRotatef(entityliving->rotationPitch - entityPitch, 1.0f, 0.0f, 0.0f);
            glRotatef(entityliving->rotationYaw - entityYaw, 0.0f, 1.0f, 0.0f);
            glTranslatef(0.0f, 0.0f, (float)(-camDist));
            glRotatef(entityYaw - entityliving->rotationYaw, 0.0f, 1.0f, 0.0f);
            glRotatef(entityPitch - entityliving->rotationPitch, 1.0f, 0.0f, 0.0f);
        }
    }
    else
    {
        // Primera persona normal
        glTranslatef(0.0f, 0.0f, -0.1f);
    }
    
    if (!mc->gameSettings->field_22273_E)
    {
        glRotatef(entityliving->prevRotationPitch + 
                  (entityliving->rotationPitch - entityliving->prevRotationPitch) * partialTicks,
                  1.0f, 0.0f, 0.0f);
        
        glRotatef(entityliving->prevRotationYaw + 
                  (entityliving->rotationYaw - entityliving->prevRotationYaw) * partialTicks + 180.0f,
                  0.0f, 1.0f, 0.0f);
    }
    
    glTranslatef(0.0f, eyeHeight, 0.0f);
    
    // Recalcular posicion para cloudFog
    posX = entityliving->prevPosX + (entityliving->posX - entityliving->prevPosX) * (double)partialTicks;
    posY = (entityliving->prevPosY + (entityliving->posY - entityliving->prevPosY) * (double)partialTicks) - (double)eyeHeight;
    posZ = entityliving->prevPosZ + (entityliving->posZ - entityliving->prevPosZ) * (double)partialTicks;
    
    cloudFog = mc->renderGlobal->isCloudFog(posX, posY, posZ, partialTicks);
}

void EntityRenderer::setupCameraTransform(float partialTicks, int anaglyphPass)
{
    farPlaneDistance = 256 >> mc->gameSettings->renderDistance;
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    float anaglyphOffset = 0.07f;
    
    if (mc->gameSettings->anaglyph)
    {
        glTranslatef((float)(-(anaglyphPass * 2 - 1)) * anaglyphOffset, 0.0f, 0.0f);
    }
    
    if (cameraZoom != 1.0)
    {
        glTranslatef((float)cameraYawOffset, (float)(-cameraPitchOffset), 0.0f);
        glScaled(cameraZoom, cameraZoom, 1.0);
        
        perspectiveGL(getFOVModifier(partialTicks),
                      (float)mc->displayWidth / (float)mc->displayHeight,
                      0.05f, farPlaneDistance * 2.0f);
    }
    else
    {
        perspectiveGL(getFOVModifier(partialTicks),
                      (float)mc->displayWidth / (float)mc->displayHeight,
                      0.05f, farPlaneDistance * 2.0f);
    }
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    if (mc->gameSettings->anaglyph)
    {
        glTranslatef((float)(anaglyphPass * 2 - 1) * 0.1f, 0.0f, 0.0f);
    }
    
    hurtCameraEffect(partialTicks);
    
    if (mc->gameSettings->viewBobbing)
    {
        setupViewBobbing(partialTicks);
    }
    
    // Efecto de portal (Nether)
    float portalTime = mc->thePlayer->prevTimeInPortal + 
                       (mc->thePlayer->timeInPortal - mc->thePlayer->prevTimeInPortal) * partialTicks;
    
    if (portalTime > 0.0f)
    {
        float portalScale = 5.0f / (portalTime * portalTime + 5.0f) - portalTime * 0.04f;
        portalScale *= portalScale;
        
        glRotatef(((float)rendererUpdateCount + partialTicks) * 20.0f, 0.0f, 1.0f, 1.0f);
        glScalef(1.0f / portalScale, 1.0f, 1.0f);
        glRotatef(-((float)rendererUpdateCount + partialTicks) * 20.0f, 0.0f, 1.0f, 1.0f);
    }
    
    orientCamera(partialTicks);
}

void EntityRenderer::renderHand(float partialTicks, int anaglyphPass)
{
    glLoadIdentity();
    
    if (mc->gameSettings->anaglyph)
    {
        glTranslatef((float)(anaglyphPass * 2 - 1) * 0.1f, 0.0f, 0.0f);
    }
    
    glPushMatrix();
    
    hurtCameraEffect(partialTicks);
    
    if (mc->gameSettings->viewBobbing)
    {
        setupViewBobbing(partialTicks);
    }
    
    // Renderizar item en mano (primera persona)
    if (!mc->gameSettings->thirdPersonView && 
        !mc->renderViewEntity->isPlayerSleeping() && 
        !mc->gameSettings->hideGUI)
    {
        itemRenderer->renderItemInFirstPerson(partialTicks);
    }
    
    glPopMatrix();
    
    // Overlays (agua, fuego, etc.)
    if (!mc->gameSettings->thirdPersonView && !mc->renderViewEntity->isPlayerSleeping())
    {
        itemRenderer->renderOverlays(partialTicks);
        hurtCameraEffect(partialTicks);
    }
    
    if (mc->gameSettings->viewBobbing)
    {
        setupViewBobbing(partialTicks);
    }
}

void EntityRenderer::updateCameraAndRender(float partialTicks)
{
    // OptiFine: si cambio el worldProvider (p.ej. al entrar a un mundo o cambiar de
    // dimension) reaplica el brillo a la nueva tabla lightBrightnessTable.
    World* world = mc->theWorld;
    if (world != nullptr && world->worldProvider != nullptr)
    {
        if (updatedWorldProvider != world->worldProvider)
        {
            updateWorldLightLevels();
            updatedWorldProvider = world->worldProvider;
        }
    }

    bool isActive = lwjgl::Display::isActive();
    
    if (!isActive)
    {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - prevFrameTime).count();
        
        if (elapsed > 500)
        {
            mc->displayInGameMenu();
        }
    }
    else
    {
        prevFrameTime = std::chrono::steady_clock::now();
    }
    
    // Manejo de mouse
    if (mc->inGameHasFocus)
    {
        mc->mouseHelper->mouseXYChange();
        
        float sensitivity = mc->gameSettings->mouseSensitivity * 0.6f + 0.2f;
        float sensitivityCubed = sensitivity * sensitivity * sensitivity * 8.0f;
        
        float deltaX = (float)mc->mouseHelper->deltaX * sensitivityCubed;
        float deltaY = (float)mc->mouseHelper->deltaY * sensitivityCubed;
        
        int invertMultiplier = mc->gameSettings->invertMouse ? -1 : 1;
        
        if (mc->gameSettings->smoothCamera)
        {
            deltaX = mouseFilterXAxis->smooth(deltaX, 0.05f * sensitivityCubed);
            deltaY = mouseFilterYAxis->smooth(deltaY, 0.05f * sensitivityCubed);
        }
        
        mc->thePlayer->turnEntity(deltaX, deltaY * (float)invertMultiplier);
    }
    
    {
        static std::atomic<int> dbgRW{0};
        int rn = dbgRW.fetch_add(1);
        if (rn < 20)
            fprintf(stderr, "Beta++: renderWorld[%d] mvDepth=%d\n", rn, gles11MatrixCurDepth(0));
    }
    if (mc->skipRenderWorld)
    {
        return;
    }
    
    anaglyphEnabled = mc->gameSettings->anaglyph;
    
    ScaledResolution scaledResolution(mc->gameSettings, mc->displayWidth, mc->displayHeight);
    int scaledWidth = scaledResolution.getScaledWidth();
    int scaledHeight = scaledResolution.getScaledHeight();
    
    // Posicion del mouse escalada
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    int scaledMouseX = (mouseX * scaledWidth) / mc->displayWidth;
    int scaledMouseY = (mouseY * scaledHeight) / mc->displayHeight;
    
    // Caracteres para limitFramerate: 0='\0', 1='x' (120), 2='(' (40)
    char fpsLimitChar = '\0';
    if (mc->gameSettings->limitFramerate == 1)
    {
        fpsLimitChar = 'x';  // ~120 fps
    }
    else if (mc->gameSettings->limitFramerate == 2)
    {
        fpsLimitChar = '(';  // ~40 fps
    }
    
    if (mc->theWorld != nullptr)
    {
        if (mc->gameSettings->limitFramerate == 0)
        {
            renderWorld(partialTicks, 0);
        }
        else
        {
            // 0x3b9aca00 = 1000000000 nanosegundos
            int64_t targetTime = field_28133_I + (int64_t)(1000000000LL / (long)fpsLimitChar);
            renderWorld(partialTicks, targetTime);
        }
        
        if (mc->gameSettings->limitFramerate == 2)
        {
            int64_t sleepTime = (field_28133_I + (int64_t)(1000000000LL / (long)fpsLimitChar) - 
                                std::chrono::duration_cast<std::chrono::nanoseconds>(
                                    std::chrono::steady_clock::now().time_since_epoch()).count()) / 1000000LL;
            
            if (sleepTime > 0 && sleepTime < 500)
            {
                SDL_Delay((Uint32)sleepTime);
            }
        }
        
        field_28133_I = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        
        // Renderizar GUI
        if (!mc->gameSettings->hideGUI || mc->currentScreen != nullptr)
        {
            mc->ingameGUI->renderGameOverlay(partialTicks, mc->currentScreen != nullptr, 
                                             scaledMouseX, scaledMouseY);
        }
    }
    else
    {
        // Menu principal / sin mundo
        glViewport(0, 0, mc->displayWidth, mc->displayHeight);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        setupOverlayRendering();
        
        if (mc->gameSettings->limitFramerate == 2)
        {
            int64_t sleepTime = (field_28133_I + (int64_t)(1000000000LL / (long)fpsLimitChar) - 
                                std::chrono::duration_cast<std::chrono::nanoseconds>(
                                    std::chrono::steady_clock::now().time_since_epoch()).count()) / 1000000LL;
            
            if (sleepTime < 0)
            {
                sleepTime += 10;
            }
            
            if (sleepTime > 0 && sleepTime < 500)
            {
                SDL_Delay((Uint32)sleepTime);
            }
        }
        
        field_28133_I = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }
    
    // Renderizar pantalla actual (GUI)
    if (mc->currentScreen != nullptr)
    {
        glClear(GL_DEPTH_BUFFER_BIT);  // 256 = GL_DEPTH_BUFFER_BIT
        
        mc->currentScreen->drawScreen(scaledMouseX, scaledMouseY, partialTicks);
        
        if (mc->currentScreen != nullptr && mc->currentScreen->guiParticles != nullptr)
        {
            mc->currentScreen->guiParticles->updateAndRender(partialTicks);
        }
    }
}

void EntityRenderer::renderWorld(float partialTicks, int64_t renderTimeLimitNano)
{
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    
    if (mc->renderViewEntity == nullptr)
    {
        mc->renderViewEntity = mc->thePlayer;
    }
    
    getMouseOver(partialTicks);
    
    EntityLiving* entityliving = mc->renderViewEntity;
    RenderGlobal* renderglobal = mc->renderGlobal;
    EffectRenderer* effectrenderer = mc->effectRenderer;
    
    double renderPosX = entityliving->lastTickPosX + (entityliving->posX - entityliving->lastTickPosX) * (double)partialTicks;
    double renderPosY = entityliving->lastTickPosY + (entityliving->posY - entityliving->lastTickPosY) * (double)partialTicks;
    double renderPosZ = entityliving->lastTickPosZ + (entityliving->posZ - entityliving->lastTickPosZ) * (double)partialTicks;
    
    IChunkProvider* chunkProvider = mc->theWorld->getIChunkProvider();
    
    // Verificar si es ChunkProviderLoadOrGenerate para setCurrentChunkOver
    ChunkProviderLoadOrGenerate* chunkLoadOrGen = dynamic_cast<ChunkProviderLoadOrGenerate*>(chunkProvider);
    if (chunkLoadOrGen != nullptr)
    {
        int chunkX = MathHelper::floor_float((int)renderPosX) >> 4;
        int chunkZ = MathHelper::floor_float((int)renderPosZ) >> 4;
        chunkLoadOrGen->setCurrentChunkOver(chunkX, chunkZ);
    }
    
    // Loop para anaglifo (0 = ojo izquierdo, 1 = ojo derecho)
    for (int eye = 0; eye < 2; eye++)
    {
        if (mc->gameSettings->anaglyph)
        {
            anaglyphField = eye;
            
            if (anaglyphField == 0)
            {
                glColorMask(false, true, true, false);
            }
            else
            {
                glColorMask(true, false, false, false);
            }
        }
        
        glViewport(0, 0, mc->displayWidth, mc->displayHeight);
        
        updateFogColor(partialTicks);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // 16640
        
        glEnable(GL_CULL_FACE);
        setupCameraTransform(partialTicks, eye);
        
        ClippingHelperImpl::getInstance();
        
        // Renderizar cielo (solo en distancias cortas)
        if (mc->gameSettings->renderDistance < 2)
        {
            setupFog(-1, partialTicks);
            renderglobal->renderSky(partialTicks);
        }
        
        glEnable(GL_FOG);
        setupFog(1, partialTicks);
        
        if (mc->gameSettings->ambientOcclusion)
        {
            glShadeModel(GL_SMOOTH);
        }
        
        // Frustum culling
        Frustrum frustrum;
        frustrum.setPosition(renderPosX, renderPosY, renderPosZ);
        
        mc->renderGlobal->clipRenderersByFrustrum(&frustrum, partialTicks);
        
        // Actualizar renderers (construccion de chunks)
        if (eye == 0)
        {
            int64_t timeLeft;
            do
            {
                if (mc->renderGlobal->updateRenderers(entityliving, false) || renderTimeLimitNano == 0)
                {
                    break;
                }
                
                timeLeft = renderTimeLimitNano - std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count();
            }
            while (timeLeft >= 0 && timeLeft <= 1000000000LL);  // 0x3b9aca00
        }
        
        // Renderizar solidos (paso 0)
        setupFog(0, partialTicks);
        glEnable(GL_FOG);
        glBindTexture(GL_TEXTURE_2D, mc->renderEngine->getTexture("/terrain.png"));
        
        RenderHelper::disableStandardItemLighting();
        renderglobal->sortAndRender(entityliving, 0, partialTicks);
        
        glShadeModel(GL_FLAT);
        RenderHelper::enableStandardItemLighting();
        
        // Entidades
        renderglobal->renderEntities(entityliving->getPosition(partialTicks), &frustrum, partialTicks);
        if (mc->gameSettings->ofShowChunkBorders)
            renderglobal->drawChunkBorders(partialTicks);
        
        // Efectos (particulas de destruccion de bloques, etc.)
        effectrenderer->renderLitParticles(entityliving, partialTicks);
        
        RenderHelper::disableStandardItemLighting();
        
        setupFog(0, partialTicks);
        
        // Particulas
        effectrenderer->renderParticles(entityliving, partialTicks);
        
        // Seleccion de bloque bajo agua
        if (mc->objectMouseOver != nullptr && 
            entityliving->isInsideOfMaterial(Material::water) && 
            dynamic_cast<EntityPlayer*>(entityliving) != nullptr)
        {
            EntityPlayer* entityplayer = (EntityPlayer*)entityliving;
            
            glDisable(GL_ALPHA_TEST);
            renderglobal->drawBlockBreaking(entityplayer, mc->objectMouseOver, 0, 
                                           entityplayer->inventory->getCurrentItem(), partialTicks);
            renderglobal->drawSelectionBox(entityplayer, mc->objectMouseOver, 0,
                                          entityplayer->inventory->getCurrentItem(), partialTicks);
            glEnable(GL_ALPHA_TEST);
        }
        
        // Renderizar transparentes (paso 1)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        setupFog(0, partialTicks);
        glEnable(GL_BLEND);
        glDisable(GL_CULL_FACE);
        glBindTexture(GL_TEXTURE_2D, mc->renderEngine->getTexture("/terrain.png"));
        
        if (mc->gameSettings->fancyGraphics)
        {
            if (mc->gameSettings->ambientOcclusion)
            {
                glShadeModel(GL_SMOOTH);
            }
            
            // Renderizar a color de ataque para transparencias (depth peeling simple)
            glColorMask(false, false, false, false);
            int transparentCount = renderglobal->sortAndRender(entityliving, 1, partialTicks);
            
            if (mc->gameSettings->anaglyph)
            {
                if (anaglyphField == 0)
                {
                    glColorMask(false, true, true, true);
                }
                else
                {
                    glColorMask(true, false, false, true);
                }
            }
            else
            {
                glColorMask(true, true, true, true);
            }
            
            if (transparentCount > 0)
            {
                renderglobal->renderAllRenderLists(1, partialTicks);
            }
            
            glShadeModel(GL_FLAT);
        }
        else
        {
            renderglobal->sortAndRender(entityliving, 1, partialTicks);
        }
        
        glDepthMask(true);
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        
        // Seleccion de bloque (fuera de agua)
        if (cameraZoom == 1.0 && 
            dynamic_cast<EntityPlayer*>(entityliving) != nullptr && 
            mc->objectMouseOver != nullptr && 
            !entityliving->isInsideOfMaterial(Material::water))
        {
            EntityPlayer* entityplayer1 = (EntityPlayer*)entityliving;
            
            glDisable(GL_ALPHA_TEST);
            renderglobal->drawBlockBreaking(entityplayer1, mc->objectMouseOver, 0,
                                           entityplayer1->inventory->getCurrentItem(), partialTicks);
            renderglobal->drawSelectionBox(entityplayer1, mc->objectMouseOver, 0,
                                          entityplayer1->inventory->getCurrentItem(), partialTicks);
            glEnable(GL_ALPHA_TEST);
        }
        
        // Lluvia y nieve
        renderRainSnow(partialTicks);
        
        // Niebla para nubes
        glDisable(GL_FOG);
        setupFog(0, partialTicks);
        glEnable(GL_FOG);
        renderglobal->renderClouds(partialTicks);
        
        glDisable(GL_FOG);
        setupFog(1, partialTicks);
        
        // Renderizar mano y overlays
        if (cameraZoom == 1.0)
        {
            glClear(GL_DEPTH_BUFFER_BIT);  // 256
            renderHand(partialTicks, eye);
        }
        
        if (!mc->gameSettings->anaglyph)
        {
            return;
        }
    }
    
    // Restaurar color mask
    glColorMask(true, true, true, false);
}

void EntityRenderer::addRainParticles()
{
    float rainStrength = mc->theWorld->getRainStrength(1.0f);
    
    if (!mc->gameSettings->fancyGraphics)
    {
        rainStrength /= 2.0f;
    }
    
    if (rainStrength == 0.0f)
    {
        return;
    }
    
    random.seed((unsigned long)rendererUpdateCount * 0x12a7ce5fL);
    
    EntityLiving* entityliving = mc->renderViewEntity;
    World* world = mc->theWorld;
    
    int centerX = MathHelper::floor_double(entityliving->posX);
    int centerY = MathHelper::floor_double(entityliving->posY);
    int centerZ = MathHelper::floor_double(entityliving->posZ);
    
    const int range = 10;
    
    double soundX = 0.0;
    double soundY = 0.0;
    double soundZ = 0.0;
    int rainCount = 0;
    
    int particleCount = (int)(100.0f * rainStrength * rainStrength);
    
    for (int i = 0; i < particleCount; i++)
    {
        int x = (centerX + (int)(random() % range)) - (int)(random() % range);
        int z = (centerZ + (int)(random() % range)) - (int)(random() % range);
        
        int topY = world->findTopSolidBlock(x, z);
        int blockIdBelow = world->getBlockId(x, topY - 1, z);
        
        if (topY > centerY + range || topY < centerY - range)
        {
            continue;
        }
        
        if (!world->getWorldChunkManager()->getBiomeGenAt(x, z)->canSpawnLightningBolt())
        {
            continue;
        }
        
        float offsetX = (float)(random() % 1000) / 1000.0f;
        float offsetZ = (float)(random() % 1000) / 1000.0f;
        
        if (blockIdBelow <= 0)
        {
            continue;
        }
        
        Block* blockBelow = Block::blocksList[blockIdBelow];
        
        if (blockBelow->blockMaterial == Material::lava)
        {
            mc->effectRenderer->addEffect(new EntitySmokeFX(
                world,
                (float)x + offsetX,
                (double)((float)topY + 0.1f) - blockBelow->minY,
                (float)z + offsetZ,
                0.0, 0.0, 0.0
            ));
            continue;
        }
        
        if ((int)(random() % (++rainCount)) == 0)
        {
            soundX = (float)x + offsetX;
            soundY = (double)((float)topY + 0.1f) - blockBelow->minY;
            soundZ = (float)z + offsetZ;
        }
        
        mc->effectRenderer->addEffect(new EntityRainFX(
            world,
            (float)x + offsetX,
            (double)((float)topY + 0.1f) - blockBelow->minY,
            (float)z + offsetZ
        ));
    }
    
    if (rainCount > 0 && (int)(random() % 3) < rainSoundCounter++)
    {
        rainSoundCounter = 0;
        
        bool isUnderCover = world->findTopSolidBlock(
            MathHelper::floor_double(entityliving->posX),
            MathHelper::floor_double(entityliving->posZ)
        ) > MathHelper::floor_double(entityliving->posY);
        
        float volume = isUnderCover ? 0.1f : 0.2f;
        float pitch = isUnderCover ? 0.5f : 1.0f;
        
        mc->theWorld->playSoundEffect(soundX, soundY, soundZ, String::fromUTF8("ambient.weather.rain"), volume, pitch);
    }
}

void EntityRenderer::renderRainSnow(float partialTicks)
{
    float rainStrength = mc->theWorld->getRainStrength(partialTicks);
    
    if (rainStrength <= 0.0f)
    {
        return;
    }
    
    EntityLiving* entityliving = mc->renderViewEntity;
    World* world = mc->theWorld;
    
    int centerX = MathHelper::floor_double(entityliving->posX);
    int centerY = MathHelper::floor_double(entityliving->posY);
    int centerZ = MathHelper::floor_double(entityliving->posZ);
    
    Tessellator* tessellator = &Tessellator::instance;
    
    glDisable(GL_CULL_FACE);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glAlphaFunc(GL_GREATER, 0.01f);
    
    glBindTexture(GL_TEXTURE_2D, mc->renderEngine->getTexture("/environment/snow.png"));
    
    double renderPosX = entityliving->lastTickPosX + (entityliving->posX - entityliving->lastTickPosX) * (double)partialTicks;
    double renderPosY = entityliving->lastTickPosY + (entityliving->posY - entityliving->lastTickPosY) * (double)partialTicks;
    double renderPosZ = entityliving->lastTickPosZ + (entityliving->posZ - entityliving->lastTickPosZ) * (double)partialTicks;
    
    int eyeHeight = MathHelper::floor_double(renderPosY);
    int range = 5;
    
    if (mc->gameSettings->fancyGraphics)
    {
        range = 10;
    }
    
    std::vector<BiomeGenBase*> biomes = world->getWorldChunkManager()->getBiomeBlock(
        centerX - range, centerZ - range, range * 2 + 1, range * 2 + 1
    );
    
    int biomeIndex = 0;
    
    // Renderizar NIEVE
    for (int x = centerX - range; x <= centerX + range; x++)
    {
        for (int z = centerZ - range; z <= centerZ + range; z++)
        {
            BiomeGenBase* biome = biomes[biomeIndex++];
            
            if (!biome->getEnableSnow())
            {
                continue;
            }
            
            int topY = world->findTopSolidBlock(x, z);
            if (topY < 0)
            {
                topY = 0;
            }
            
            int renderY = topY;
            if (renderY < eyeHeight)
            {
                renderY = eyeHeight;
            }
            
            int minY = centerY - range;
            int maxY = centerY + range;
            
            if (minY < topY)
            {
                minY = topY;
            }
            if (maxY < topY)
            {
                maxY = topY;
            }
            
            float scale = 1.0f;
            
            if (minY != maxY)
            {
                unsigned int seed = (unsigned int)(x * x * 3121 + x * 0x2b24abb + z * z * 0x66397 + z * 13761);
                random.seed(seed);
                
                float time = (float)rendererUpdateCount + partialTicks;
                float texV = ((float)(rendererUpdateCount & 0x1ff) + partialTicks) / 512.0f;
                std::normal_distribution<float> gaussDist(0.0f, 1.0f);
                float texU1 = (float)(random() % 1000) / 1000.0f + time * 0.01f * gaussDist(random);
                float texU2 = (float)(random() % 1000) / 1000.0f + time * gaussDist(random) * 0.001f;
                
                double distX = (double)((float)x + 0.5f) - entityliving->posX;
                double distZ = (double)((float)z + 0.5f) - entityliving->posZ;
                float distance = MathHelper::sqrt_double(distX * distX + distZ * distZ) / (float)range;
                
                tessellator->startDrawingQuads();
                
                float brightness = world->getLightBrightness(x, renderY, z);
                glColor4f(brightness, brightness, brightness, 
                         ((1.0f - distance * distance) * 0.3f + 0.5f) * rainStrength);
                
                tessellator->setTranslationD(-renderPosX, -renderPosY, -renderPosZ);
                
                // Cara Z
                tessellator->addVertexWithUV(x + 0, minY, (double)z + 0.5, 0.0f * scale + texU1, 
                    ((float)minY * scale) / 4.0f + texV * scale + texU2);
                tessellator->addVertexWithUV(x + 1, minY, (double)z + 0.5, 1.0f * scale + texU1,
                    ((float)minY * scale) / 4.0f + texV * scale + texU2);
                tessellator->addVertexWithUV(x + 1, maxY, (double)z + 0.5, 1.0f * scale + texU1,
                    ((float)maxY * scale) / 4.0f + texV * scale + texU2);
                tessellator->addVertexWithUV(x + 0, maxY, (double)z + 0.5, 0.0f * scale + texU1,
                    ((float)maxY * scale) / 4.0f + texV * scale + texU2);
                
                // Cara X
                tessellator->addVertexWithUV((double)x + 0.5, minY, z + 0, 0.0f * scale + texU1,
                    ((float)minY * scale) / 4.0f + texV * scale + texU2);
                tessellator->addVertexWithUV((double)x + 0.5, minY, z + 1, 1.0f * scale + texU1,
                    ((float)minY * scale) / 4.0f + texV * scale + texU2);
                tessellator->addVertexWithUV((double)x + 0.5, maxY, z + 1, 1.0f * scale + texU1,
                    ((float)maxY * scale) / 4.0f + texV * scale + texU2);
                tessellator->addVertexWithUV((double)x + 0.5, maxY, z + 0, 0.0f * scale + texU1,
                    ((float)maxY * scale) / 4.0f + texV * scale + texU2);
                
                tessellator->setTranslationD(0.0, 0.0, 0.0);
                tessellator->draw();
            }
        }
    }
    
    // Renderizar LLUVIA
    glBindTexture(GL_TEXTURE_2D, mc->renderEngine->getTexture("/environment/rain.png"));
    
    if (mc->gameSettings->fancyGraphics)
    {
        range = 10;
    }
    
    biomeIndex = 0;
    
    for (int x = centerX - range; x <= centerX + range; x++)
    {
        for (int z = centerZ - range; z <= centerZ + range; z++)
        {
            BiomeGenBase* biome = biomes[biomeIndex++];
            
            if (!biome->canSpawnLightningBolt())
            {
                continue;
            }
            
            int topY = world->findTopSolidBlock(x, z);
            
            int minY = centerY - range;
            int maxY = centerY + range;
            
            if (minY < topY)
            {
                minY = topY;
            }
            if (maxY < topY)
            {
                maxY = topY;
            }
            
            float scale = 1.0f;
            
            if (minY != maxY)
            {
                unsigned int seed = (unsigned int)(x * x * 3121 + x * 0x2b24abb + z * z * 0x66397 + z * 13761);
                random.seed(seed);
                
                int timeBits = rendererUpdateCount + x * x * 3121 + x * 0x2b24abb + z * z * 0x66397 + z * 13761;
                float texV = (((float)(timeBits & 0x1f) + partialTicks) / 32.0f) * (3.0f + (float)(random() % 1000) / 1000.0f);
                
                double distX = (double)((float)x + 0.5f) - entityliving->posX;
                double distZ = (double)((float)z + 0.5f) - entityliving->posZ;
                float distance = MathHelper::sqrt_double(distX * distX + distZ * distZ) / (float)range;
                
                tessellator->startDrawingQuads();
                
                float brightness = world->getLightBrightness(x, 128, z) * 0.85f + 0.15f;
                glColor4f(brightness, brightness, brightness,
                         ((1.0f - distance * distance) * 0.5f + 0.5f) * rainStrength);
                
                tessellator->setTranslationD(-renderPosX, -renderPosY, -renderPosZ);
                
                // Cara Z
                tessellator->addVertexWithUV(x + 0, minY, (double)z + 0.5, 0.0f * scale + texV,
                    ((float)minY * scale) / 4.0f + texV * scale);
                tessellator->addVertexWithUV(x + 1, minY, (double)z + 0.5, 1.0f * scale + texV,
                    ((float)minY * scale) / 4.0f + texV * scale);
                tessellator->addVertexWithUV(x + 1, maxY, (double)z + 0.5, 1.0f * scale + texV,
                    ((float)maxY * scale) / 4.0f + texV * scale);
                tessellator->addVertexWithUV(x + 0, maxY, (double)z + 0.5, 0.0f * scale + texV,
                    ((float)maxY * scale) / 4.0f + texV * scale);
                
                // Cara X
                tessellator->addVertexWithUV((double)x + 0.5, minY, z + 0, 0.0f * scale + texV,
                    ((float)minY * scale) / 4.0f + texV * scale);
                tessellator->addVertexWithUV((double)x + 0.5, minY, z + 1, 1.0f * scale + texV,
                    ((float)minY * scale) / 4.0f + texV * scale);
                tessellator->addVertexWithUV((double)x + 0.5, maxY, z + 1, 1.0f * scale + texV,
                    ((float)maxY * scale) / 4.0f + texV * scale);
                tessellator->addVertexWithUV((double)x + 0.5, maxY, z + 0, 0.0f * scale + texV,
                    ((float)maxY * scale) / 4.0f + texV * scale);
                
                tessellator->setTranslationD(0.0, 0.0, 0.0);
                tessellator->draw();
            }
        }
    }
    
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glAlphaFunc(GL_GREATER, 0.1f);
}

void EntityRenderer::setupOverlayRendering()
{
    ScaledResolution scaledResolution(mc->gameSettings, mc->displayWidth, mc->displayHeight);
    
    glClear(GL_DEPTH_BUFFER_BIT);  // 256
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, scaledResolution.field_25121_a, scaledResolution.field_25120_b, 0.0, 1000.0, 3000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -2000.0f);
}

void EntityRenderer::updateFogColor(float partialTicks)
{
    World* world = mc->theWorld;
    EntityLiving* entityliving = mc->renderViewEntity;
    
    float fogDistanceFactor = 1.0f / (float)(4 - mc->gameSettings->renderDistance);
    fogDistanceFactor = 1.0f - (float)pow(fogDistanceFactor, 0.25);
    
    Vec3D* skyColor = world->getSkyColor(mc->renderViewEntity, partialTicks);
    float skyR = (float)skyColor->xCoord;
    float skyG = (float)skyColor->yCoord;
    float skyB = (float)skyColor->zCoord;
    
    Vec3D* fogColor = world->getFogColor(partialTicks);
    fogColorRed = (float)fogColor->xCoord;
    fogColorGreen = (float)fogColor->yCoord;
    fogColorBlue = (float)fogColor->zCoord;
    
    fogColorRed += (skyR - fogColorRed) * fogDistanceFactor;
    fogColorGreen += (skyG - fogColorGreen) * fogDistanceFactor;
    fogColorBlue += (skyB - fogColorBlue) * fogDistanceFactor;
    
    // Aplicar efecto de lluvia
    float rainStrength = world->getRainStrength(partialTicks);
    if (rainStrength > 0.0f)
    {
        float rainDarkness = 1.0f - rainStrength * 0.5f;
        float rainBlueDarkness = 1.0f - rainStrength * 0.4f;
        fogColorRed *= rainDarkness;
        fogColorGreen *= rainDarkness;
        fogColorBlue *= rainBlueDarkness;
    }
    
    // Aplicar efecto de tormenta
    float thunderStrength = world->getThunderStrength(partialTicks);
    if (thunderStrength > 0.0f)
    {
        float thunderDarkness = 1.0f - thunderStrength * 0.5f;
        fogColorRed *= thunderDarkness;
        fogColorGreen *= thunderDarkness;
        fogColorBlue *= thunderDarkness;
    }
    
    // Niebla de nubes
    if (cloudFog)
    {
        Vec3D* cloudColor = world->getCloudFogColor(partialTicks);
        fogColorRed = (float)cloudColor->xCoord;
        fogColorGreen = (float)cloudColor->yCoord;
        fogColorBlue = (float)cloudColor->zCoord;
    }
    else if (entityliving->isInsideOfMaterial(Material::water))
    {
        fogColorRed = 0.02f;
        fogColorGreen = 0.02f;
        fogColorBlue = 0.2f;
    }
    else if (entityliving->isInsideOfMaterial(Material::lava))
    {
        fogColorRed = 0.6f;
        fogColorGreen = 0.1f;
        fogColorBlue = 0.0f;
    }
    
    // Interpolacion de niebla
    float fogBrightness = fogColor2 + (fogColor1 - fogColor2) * partialTicks;
    fogColorRed *= fogBrightness;
    fogColorGreen *= fogBrightness;
    fogColorBlue *= fogBrightness;
    
    // Anaglifo
    if (mc->gameSettings->anaglyph)
    {
        float grayR = (fogColorRed * 30.0f + fogColorGreen * 59.0f + fogColorBlue * 11.0f) / 100.0f;
        float grayG = (fogColorRed * 30.0f + fogColorGreen * 70.0f) / 100.0f;
        float grayB = (fogColorRed * 30.0f + fogColorBlue * 70.0f) / 100.0f;
        
        fogColorRed = grayR;
        fogColorGreen = grayG;
        fogColorBlue = grayB;
    }
    
    glClearColor(fogColorRed, fogColorGreen, fogColorBlue, 0.0f);
}

void EntityRenderer::setupFog(int fogMode, float partialTicks)
{
    EntityLiving* entityliving = mc->renderViewEntity;
    
    setupFogColorBuffer(fogColorRed, fogColorGreen, fogColorBlue, 1.0f);
    glFogfv(GL_FOG_COLOR, fogColorBuffer);
    
    glNormal3f(0.0f, -1.0f, 0.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    
    if (cloudFog)
    {
        // Niebla de nubes - EXP
        glFogi(GL_FOG_MODE, GL_EXP);
        glFogf(GL_FOG_DENSITY, 0.1f);
        
        float f1 = 1.0f, f4 = 1.0f, f7 = 1.0f;
        
        if (mc->gameSettings->anaglyph)
        {
            float f10 = (f1 * 30.0f + f4 * 59.0f + f7 * 11.0f) / 100.0f;
            float f13 = (f1 * 30.0f + f4 * 70.0f) / 100.0f;
            float f16 = (f1 * 30.0f + f7 * 70.0f) / 100.0f;
            f1 = f10; f4 = f13; f7 = f16;
        }
    }
    else if (entityliving->isInsideOfMaterial(Material::water))
    {
        // Bajo agua - EXP
        glFogi(GL_FOG_MODE, GL_EXP);
        glFogf(GL_FOG_DENSITY, 0.1f);
        
        float f2 = 0.4f, f5 = 0.4f, f8 = 0.9f;
        
        if (mc->gameSettings->anaglyph)
        {
            float f11 = (f2 * 30.0f + f5 * 59.0f + f8 * 11.0f) / 100.0f;
            float f14 = (f2 * 30.0f + f5 * 70.0f) / 100.0f;
            float f17 = (f2 * 30.0f + f8 * 70.0f) / 100.0f;
            f2 = f11; f5 = f14; f8 = f17;
        }
    }
    else if (entityliving->isInsideOfMaterial(Material::lava))
    {
        // En lava - EXP denso
        glFogi(GL_FOG_MODE, GL_EXP);
        glFogf(GL_FOG_DENSITY, 2.0f);
        
        float f3 = 0.4f, f6 = 0.3f, f9 = 0.3f;
        
        if (mc->gameSettings->anaglyph)
        {
            float f12 = (f3 * 30.0f + f6 * 59.0f + f9 * 11.0f) / 100.0f;
            float f15 = (f3 * 30.0f + f6 * 70.0f) / 100.0f;
            float f18 = (f3 * 30.0f + f9 * 70.0f) / 100.0f;
            f3 = f12; f6 = f15; f9 = f18;
        }
    }
    else
    {
        // Niebla normal - LINEAR
        glFogi(GL_FOG_MODE, GL_LINEAR);

        // OptiFine: Fancy vs Fast. La diferencia real es la CALIDAD de la niebla:
        //  - Fancy: niebla por pixel (GL_NICEST) y, si el driver lo soporta, radial
        //           (redonda) via GL_NV_fog_distance.
        //  - Fast : niebla por vertice (GL_FASTEST): mas barata, con bandas.
        // glHint la respetan casi todos los drivers (a diferencia de NV_fog_distance,
        // que en hardware no-NVIDIA se ignora y por eso "no se notaba diferencia").
        if (Config::isFogFancy())
        {
            glHint(GL_FOG_HINT, GL_NICEST);
#ifndef GL_FOG_DISTANCE_MODE_NV
#define GL_FOG_DISTANCE_MODE_NV 0x855A
#define GL_EYE_RADIAL_NV 0x855B
#endif
            glFogi(GL_FOG_DISTANCE_MODE_NV, GL_EYE_RADIAL_NV);
        }
        else
        {
            glHint(GL_FOG_HINT, GL_FASTEST);
        }

        // OptiFine: Fog Start configurable (vanilla usaba 0.25f fijo)
        float fogStart = farPlaneDistance * Config::getFogStart();
        float fogEnd   = farPlaneDistance;

        if (fogMode < 0)
        {
            // Cielo
            fogStart = 0.0f;
            fogEnd   = farPlaneDistance * 0.8f;
        }

        // Nether tiene niebla diferente
        if (mc->theWorld->worldProvider->isNether)
            fogStart = 0.0f;

        // Extension del port: Fog OFF -> empuja la niebla del mundo (fogMode >= 0) mas
        // alla del plano lejano para que no haya "muro" de niebla. No toca el cielo
        // (fogMode < 0), el Nether, ni la niebla de agua/lava/nubes (otras ramas).
        if (Config::isFogOff() && fogMode >= 0 && !mc->theWorld->worldProvider->isNether)
        {
            fogStart = farPlaneDistance * 4.0f;
            fogEnd   = farPlaneDistance * 8.0f;
        }

        glFogf(GL_FOG_START, fogStart);
        glFogf(GL_FOG_END,   fogEnd);
    }
    
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT);
}

void EntityRenderer::setupFogColorBuffer(float r, float g, float b, float a)
{
    fogColorBuffer[0] = r;
    fogColorBuffer[1] = g;
    fogColorBuffer[2] = b;
    fogColorBuffer[3] = a;
}
