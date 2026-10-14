#include "NetClientHandler.h"
#include <iostream>
#include "java/String.h"
#include "NetHandler.h"
#include "Minecraft.h"
#include "MapStorage.h"
#include "NetworkManager.h"
#include "WorldClient.h"
#include "PlayerControllerMP.h"
#include "StatList.h"
#include "StatFileWriter.h"
#include "GuiDownloadTerrain.h"
#include "EntityItem.h"
#include "ItemStack.h"
#include "EntityMinecart.h"
#include "EntityFish.h"
#include "EntityArrow.h"
#include "EntitySnowball.h"
#include "EntityFireball.h"
#include "EntityEgg.h"
#include "EntityBoat.h"
#include "EntityTNTPrimed.h"
#include "EntityFallingSand.h"
#include "Block.h"
#include "Entity.h"
#include "EntityLiving.h"
#include "EntityLightningBolt.h"
#include "EntityPainting.h"
#include "EntityOtherPlayerMP.h"
#include "InventoryPlayer.h"
#include "EntityPlayer.h"
#include "EntityPlayerSP.h"
#include "EntityClientPlayerMP.h"
#include "Chunk.h"
#include "GuiConnectFailed.h"
#include "EntityPickupFX.h"
#include "EffectRenderer.h"
#include "GuiIngame.h"
#include "Session.h"
#include "EntityList.h"
#include "ChunkCoordinates.h"
#include "WorldInfo.h"
#include "Explosion.h"
#include "InventoryBasic.h"
#include "Container.h"
#include "TileEntityFurnace.h"
#include "TileEntityDispenser.h"
#include "MathHelper.h"
#include "Slot.h"
#include "TileEntitySign.h"
#include "TileEntity.h"
#include "ItemMap.h"
#include "Item.h"
#include "MapData.h"
#include "StatList.h"
#include "DataWatcher.h"
#include "ChunkPosition.h"
#include "AxisAlignedBB.h"

// Packets
#include "Packet.h"
#include "Packet1Login.h"
#include "Packet21PickupSpawn.h"
#include "Packet23VehicleSpawn.h"
#include "Packet71Weather.h"
#include "Packet25EntityPainting.h"
#include "Packet28EntityVelocity.h"
#include "Packet40EntityMetadata.h"
#include "Packet20NamedEntitySpawn.h"
#include "Packet34EntityTeleport.h"
#include "Packet30Entity.h"
#include "Packet29DestroyEntity.h"
#include "Packet10Flying.h"
#include "Packet13PlayerLookMove.h"
#include "Packet50PreChunk.h"
#include "Packet52MultiBlockChange.h"
#include "Packet51MapChunk.h"
#include "Packet53BlockChange.h"
#include "Packet255KickDisconnect.h"
#include "Packet22Collect.h"
#include "Packet3Chat.h"
#include "Packet18Animation.h"
#include "Packet17Sleep.h"
#include "Packet0KeepAlive.h"
#include "Packet2Handshake.h"
#include "Packet24MobSpawn.h"
#include "Packet4UpdateTime.h"
#include "Packet6SpawnPosition.h"
#include "Packet39AttachEntity.h"
#include "Packet38EntityStatus.h"
#include "Packet8UpdateHealth.h"
#include "Packet9Respawn.h"
#include "Packet60Explosion.h"
#include "Packet100OpenWindow.h"
#include "Packet103SetSlot.h"
#include "Packet106Transaction.h"
#include "Packet104WindowItems.h"
#include "Packet130UpdateSign.h"
#include "Packet105UpdateProgressbar.h"
#include "Packet5PlayerInventory.h"
#include "Packet101CloseWindow.h"
#include "Packet54PlayNoteBlock.h"
#include "Packet70Bed.h"
#include "Packet131MapData.h"
#include "Packet61DoorChange.h"
#include "Packet200Statistic.h"

// Networking (SDL_net not available — TCP handled via platform sockets or stub)

NetClientHandler::NetClientHandler(Minecraft* minecraft, const std::string& host, int port)
{
    disconnected = false;
    terrainDownloaded = false;
    mapStorage = new MapStorage(nullptr);
    
    std::random_device rd;
    rand.seed(rd());
    
    mc = minecraft;
    
    netManager = new NetworkManager(host, port, "Client", this);
    serverHostname = host;
}

NetClientHandler::~NetClientHandler()
{
    delete mapStorage;
    delete netManager;
}

void NetClientHandler::processReadPackets()
{
    if (!disconnected)
    {
        netManager->processReadPackets();
    }
    netManager->wakeThreads();
}

void NetClientHandler::handleLogin(Packet1Login* packet)
{
    mc->playerController = new PlayerControllerMP(mc, this);
    mc->statFileWriter->readStat(StatList::joinMultiplayerStat, 1);
    
    worldClient = new WorldClient(this, packet->mapSeed, packet->dimension);
    worldClient->multiplayerWorld = true;
    
    mc->changeWorld1(worldClient);
    mc->thePlayer->dimension = packet->dimension;
    mc->displayGuiScreen(new GuiDownloadTerrain(this));
    mc->thePlayer->entityId = packet->protocolVersion;
}

void NetClientHandler::handlePickupSpawn(Packet21PickupSpawn* packet)
{
    double posX = (double)packet->xPosition / 32.0;
    double posY = (double)packet->yPosition / 32.0;
    double posZ = (double)packet->zPosition / 32.0;
    
    EntityItem* entityItem = new EntityItem(
        worldClient, posX, posY, posZ,
        new ItemStack(packet->itemID, packet->count, packet->itemDamage)
    );
    
    entityItem->motionX = (double)packet->rotation / 128.0;
    entityItem->motionY = (double)packet->pitch / 128.0;
    entityItem->motionZ = (double)packet->roll / 128.0;
    
    entityItem->serverPosX = packet->xPosition;
    entityItem->serverPosY = packet->yPosition;
    entityItem->serverPosZ = packet->zPosition;
    
    worldClient->addEntityToWorld(packet->entityId, entityItem);
}

void NetClientHandler::handleVehicleSpawn(Packet23VehicleSpawn* packet)
{
    double posX = (double)packet->xPosition / 32.0;
    double posY = (double)packet->yPosition / 32.0;
    double posZ = (double)packet->zPosition / 32.0;
    
    Entity* entity = nullptr;
    
    if (packet->type == 10)
    {
        entity = new EntityMinecart(worldClient, posX, posY, posZ, 0);
    }
    else if (packet->type == 11)
    {
        entity = new EntityMinecart(worldClient, posX, posY, posZ, 1);
    }
    else if (packet->type == 12)
    {
        entity = new EntityMinecart(worldClient, posX, posY, posZ, 2);
    }
    else if (packet->type == 90)
    {
        entity = new EntityFish(worldClient);
    }
    else if (packet->type == 60)
    {
        entity = new EntityArrow(worldClient, posX, posY, posZ);
    }
    else if (packet->type == 61)
    {
        entity = new EntitySnowball(worldClient, posX, posY, posZ);
    }
    else if (packet->type == 63)
    {
        entity = new EntityFireball(
            worldClient, posX, posY, posZ,
            (double)packet->speedX / 8000.0,
            (double)packet->speedY / 8000.0,
            (double)packet->speedZ / 8000.0
        );
        packet->throwerEntityId = 0;
    }
    else if (packet->type == 62)
    {
        entity = new EntityEgg(worldClient, posX, posY, posZ);
    }
    else if (packet->type == 1)
    {
        entity = new EntityBoat(worldClient, posX, posY, posZ);
    }
    else if (packet->type == 50)
    {
        entity = new EntityTNTPrimed(worldClient, posX, posY, posZ);
    }
    else if (packet->type == 70)
    {
        entity = new EntityFallingSand(worldClient, posX, posY, posZ, Block::sand->blockID);
    }
    else if (packet->type == 71)
    {
        entity = new EntityFallingSand(worldClient, posX, posY, posZ, Block::gravel->blockID);
    }
    
    if (entity != nullptr)
    {
        entity->serverPosX = packet->xPosition;
        entity->serverPosY = packet->yPosition;
        entity->serverPosZ = packet->zPosition;
        entity->rotationYaw = 0.0f;
        entity->rotationPitch = 0.0f;
        entity->entityId = packet->entityId;

        worldClient->addEntityToWorld(packet->entityId, entity);

        if (packet->throwerEntityId > 0)
        {
            if (packet->type == 60)
            {
                Entity* thrower = getEntityByID(packet->throwerEntityId);
                EntityLiving* throwerLiving = dynamic_cast<EntityLiving*>(thrower);
                if (throwerLiving != nullptr)
                {
                    EntityArrow* arrow = dynamic_cast<EntityArrow*>(entity);
                    if (arrow != nullptr)
                    {
                        arrow->owner = throwerLiving;
                    }
                }
            }

            entity->setVelocity(
                (double)packet->speedX / 8000.0,
                (double)packet->speedY / 8000.0,
                (double)packet->speedZ / 8000.0
            );
        }
    }
}

void NetClientHandler::handleWeather(Packet71Weather* packet)
{
    double posX = (double)packet->posX / 32.0;
    double posY = (double)packet->posY / 32.0;
    double posZ = (double)packet->posZ / 32.0;
    
    EntityLightningBolt* lightningBolt = nullptr;
    
    if (packet->weatherType == 1)
    {
        lightningBolt = new EntityLightningBolt(worldClient, posX, posY, posZ);
    }
    
    if (lightningBolt != nullptr)
    {
        lightningBolt->serverPosX = packet->posX;
        lightningBolt->serverPosY = packet->posY;
        lightningBolt->serverPosZ = packet->posZ;
        lightningBolt->rotationYaw = 0.0f;
        lightningBolt->rotationPitch = 0.0f;
        lightningBolt->entityId = packet->entityId;
        
        worldClient->addWeatherEffect(lightningBolt);
    }
}

void NetClientHandler::handlePaintingSpawn(Packet25EntityPainting* packet)
{
    EntityPainting* painting = new EntityPainting(
        worldClient,
        packet->xPosition,
        packet->yPosition,
        packet->zPosition,
        packet->direction,
        packet->title
    );
    
    worldClient->addEntityToWorld(packet->entityId, painting);
}

void NetClientHandler::handleEntityVelocity(Packet28EntityVelocity* packet)
{
    Entity* entity = getEntityByID(packet->entityId);
    
    if (entity == nullptr)
    {
        return;
    }
    
    entity->setVelocity(
        (double)packet->motionX / 8000.0,
        (double)packet->motionY / 8000.0,
        (double)packet->motionZ / 8000.0
    );
}

void NetClientHandler::handleEntityMetadata(Packet40EntityMetadata* packet)
{
    Entity* entity = getEntityByID(packet->entityId);
    
    if (entity != nullptr)
    {
        auto metaList = packet->getMetadata();
        if (!metaList.empty())
            entity->getDataWatcher()->updateWatchedObjectsFromList(metaList);
    }
}

void NetClientHandler::handleNamedEntitySpawn(Packet20NamedEntitySpawn* packet)
{
    double posX = (double)packet->xPosition / 32.0;
    double posY = (double)packet->yPosition / 32.0;
    double posZ = (double)packet->zPosition / 32.0;
    
    float yaw = (float)(packet->rotation * 360) / 256.0f;
    float pitch = (float)(packet->pitch * 360) / 256.0f;
    
    EntityOtherPlayerMP* otherPlayer = new EntityOtherPlayerMP(mc->theWorld, packet->name);
    
    otherPlayer->prevPosX = otherPlayer->lastTickPosX = otherPlayer->serverPosX = packet->xPosition;
    otherPlayer->prevPosY = otherPlayer->lastTickPosY = otherPlayer->serverPosY = packet->yPosition;
    otherPlayer->prevPosZ = otherPlayer->lastTickPosZ = otherPlayer->serverPosZ = packet->zPosition;
    
    int currentItem = packet->currentItem;
    
    if (currentItem == 0)
    {
        otherPlayer->inventory->mainInventory[otherPlayer->inventory->currentItem] = nullptr;
    }
    else
    {
        otherPlayer->inventory->mainInventory[otherPlayer->inventory->currentItem] = new ItemStack(currentItem, 1, 0);
    }
    
    otherPlayer->setPositionAndRotation(posX, posY, posZ, yaw, pitch);
    worldClient->addEntityToWorld(packet->entityId, otherPlayer);
}

void NetClientHandler::handleEntityTeleport(Packet34EntityTeleport* packet)
{
    Entity* entity = getEntityByID(packet->entityId);
    
    if (entity == nullptr)
    {
        return;
    }
    
    entity->serverPosX = packet->xPosition;
    entity->serverPosY = packet->yPosition;
    entity->serverPosZ = packet->zPosition;
    
    double posX = (double)entity->serverPosX / 32.0;
    double posY = (double)entity->serverPosY / 32.0 + 0.015625;
    double posZ = (double)entity->serverPosZ / 32.0;
    
    float yaw = (float)(packet->yaw * 360) / 256.0f;
    float pitch = (float)(packet->pitch * 360) / 256.0f;
    
    entity->setPositionAndRotation2(posX, posY, posZ, yaw, pitch, 3);
}

void NetClientHandler::handleEntityMovement(Packet30Entity* packet)
{
    Entity* entity = getEntityByID(packet->entityId);
    
    if (entity == nullptr)
    {
        return;
    }
    
    entity->serverPosX += packet->xPosition;
    entity->serverPosY += packet->yPosition;
    entity->serverPosZ += packet->zPosition;
    
    double posX = (double)entity->serverPosX / 32.0;
    double posY = (double)entity->serverPosY / 32.0;
    double posZ = (double)entity->serverPosZ / 32.0;
    
    float yaw = packet->rotating ? (float)(packet->yaw * 360) / 256.0f : entity->rotationYaw;
    float pitch = packet->rotating ? (float)(packet->pitch * 360) / 256.0f : entity->rotationPitch;
    
    entity->setPositionAndRotation2(posX, posY, posZ, yaw, pitch, 3);
}

void NetClientHandler::handleDestroyEntity(Packet29DestroyEntity* packet)
{
    worldClient->removeEntityFromWorld(packet->entityId);
}

void NetClientHandler::handleFlying(Packet10Flying* packet)
{
    {
        static std::atomic<int> fin{0};
        static std::atomic<int> flyTotal{0};
        flyTotal.fetch_add(1);
        long_t nowMs = System::currentTimeMillis();
        static long_t lastCorr = 0;
        if (nowMs - lastCorr > 4000)
        {
            lastCorr = nowMs;
            fprintf(stderr, "Beta++: CORR total=%d moving=%d rotating=%d pos=(%.2f,%.2f,%.2f) onGround=%d cur=(%.2f,%.2f,%.2f)\n",
                    (int)flyTotal.load(), packet->moving ? 1 : 0, packet->rotating ? 1 : 0,
                    packet->xPosition, packet->yPosition, packet->zPosition,
                    packet->onGround ? 1 : 0,
                    mc->thePlayer != nullptr ? mc->thePlayer->posX : 0.0,
                    mc->thePlayer != nullptr ? mc->thePlayer->posY : 0.0,
                    mc->thePlayer != nullptr ? mc->thePlayer->posZ : 0.0);
        }
        if (fin.fetch_add(1) < 4)
            fprintf(stderr, "Beta++: FLY moving=%d rotating=%d pos=(%.2f,%.2f,%.2f) yaw=%.1f pitch=%.1f onGround=%d cur=(%.2f,%.2f,%.2f)\n",
                    packet->moving ? 1 : 0, packet->rotating ? 1 : 0,
                    packet->xPosition, packet->yPosition, packet->zPosition,
                    packet->yaw, packet->pitch, packet->onGround ? 1 : 0,
                    mc->thePlayer != nullptr ? mc->thePlayer->posX : 0.0,
                    mc->thePlayer != nullptr ? mc->thePlayer->posY : 0.0,
                    mc->thePlayer != nullptr ? mc->thePlayer->posZ : 0.0);
    }
    EntityPlayerSP* playerSP = mc->thePlayer;
    EntityPlayer* player = playerSP;
    
    double posX = player->posX;
    double posY = player->posY;
    double posZ = player->posZ;
    float yaw = player->rotationYaw;
    float pitch = player->rotationPitch;
    
    if (packet->moving)
    {
        posX = packet->xPosition;
        posY = packet->yPosition;
        posZ = packet->zPosition;
    }
    
    if (packet->rotating)
    {
        yaw = packet->yaw;
        pitch = packet->pitch;
    }
    
    playerSP->ySize = 0.0f;
    playerSP->motionX = playerSP->motionY = playerSP->motionZ = 0.0;
    playerSP->setPositionAndRotation(posX, posY, posZ, yaw, pitch);

    // Java re-sends the received packet object back to the server. In C++ that
    // packet is owned by processReadPackets' unique_ptr (which deletes it after
    // this call), and addToSendQueue takes ownership too -> double free / crash
    // when deleting packet id 13. Send a fresh copy instead and let the received
    // packet be freed normally.
    netManager->addToSendQueue(new Packet13PlayerLookMove(
        player->posX,
        player->boundingBox->minY,
        player->posY,            // stance
        player->posZ,
        yaw,
        pitch,
        packet->onGround));
    
    if (!terrainDownloaded)
    {
        mc->thePlayer->prevPosX = mc->thePlayer->posX;
        mc->thePlayer->prevPosY = mc->thePlayer->posY;
        mc->thePlayer->prevPosZ = mc->thePlayer->posZ;
        terrainDownloaded = true;
        mc->displayGuiScreen(nullptr);
    }
}

void NetClientHandler::handleKeepAlive(Packet0KeepAlive &packet)
{
    // Beta servers kick clients that fail to echo keepalives (~30s).
    addToSendQueue(new Packet0KeepAlive(packet.keepAliveId));
}

void NetClientHandler::handlePreChunk(Packet50PreChunk* packet)
{
    {
        static std::atomic<int> pc{0};
        static std::atomic<int> pcfar{0};
        int pcn = pc.fetch_add(1);
        int ax = packet->xPosition < 0 ? -packet->xPosition : packet->xPosition;
        int az = packet->yPosition < 0 ? -packet->yPosition : packet->yPosition;
        if (pcn < 8 || ((ax > 32 || az > 32) && pcfar.fetch_add(1) < 24))
            fprintf(stderr, "Beta++: PRECHUNK x=%d z=%d load=%d (n=%d)\n",
                    (int)packet->xPosition, (int)packet->yPosition, packet->mode ? 1 : 0, pcn);
    }
    worldClient->doPreChunk(packet->xPosition, packet->yPosition, packet->mode);
}

void NetClientHandler::handleMultiBlockChange(Packet52MultiBlockChange* packet)
{
    Chunk* chunk = worldClient->getChunkFromChunkCoords(packet->xPosition, packet->zPosition);
    
    int baseX = packet->xPosition * 16;
    int baseZ = packet->zPosition * 16;
    
    for (int i = 0; i < packet->size; i++)
    {
        short coord = packet->coordinateArray[i];
        int blockId = packet->typeArray[i] & 0xff;
        char metadata = packet->metadataArray[i];
        
        int x = (coord >> 12) & 0xf;
        int z = (coord >> 8) & 0xf;
        int y = coord & 0xff;
        
        chunk->setBlockIDWithMetadata(x, y, z, blockId, metadata);
        
        worldClient->invalidateBlockReceiveRegion(x + baseX, y, z + baseZ, x + baseX, y, z + baseZ);
        worldClient->markBlocksDirty(x + baseX, y, z + baseZ, x + baseX, y, z + baseZ);
    }
}

void NetClientHandler::handleMapChunk(Packet51MapChunk* packet)
{
    // Guarantee a real chunk exists for every chunk touched by this region,
    // even if its PreChunk was missed: setChunkData on a provider that returns
    // the shared blank chunk would write the terrain into the void.
    {
        int cx0 = packet->xPosition >> 4;
        int cz0 = packet->zPosition >> 4;
        int cx1 = (packet->xPosition + packet->xSize - 1) >> 4;
        int cz1 = (packet->zPosition + packet->zSize - 1) >> 4;
        for (int cx = cx0; cx <= cx1; cx++)
            for (int cz = cz0; cz <= cz1; cz++)
                worldClient->doPreChunk(cx, cz, true);
    }

    worldClient->invalidateBlockReceiveRegion(
        packet->xPosition,
        packet->yPosition,
        packet->zPosition,
        (packet->xPosition + packet->xSize) - 1,
        (packet->yPosition + packet->ySize) - 1,
        (packet->zPosition + packet->zSize) - 1
    );
    
    worldClient->setChunkData(
        packet->xPosition,
        packet->yPosition,
        packet->zPosition,
        packet->xSize,
        packet->ySize,
        packet->zSize,
        packet->chunk
    );
}

void NetClientHandler::handleBlockChange(Packet53BlockChange* packet)
{
    worldClient->setBlockAndMetadataAndInvalidate(
        packet->xPosition,
        packet->yPosition,
        packet->zPosition,
        packet->type,
        packet->metadata
    );
}

void NetClientHandler::handleKickDisconnect(Packet255KickDisconnect* packet)
{
    netManager->networkShutdown("disconnect.kicked", {});
    disconnected = true;
    
    mc->changeWorld1(nullptr);
    mc->displayGuiScreen(new GuiConnectFailed(
        "disconnect.disconnected",
        "disconnect.genericReason",
        String::toUTF8(packet->reason)
    ));
}

void NetClientHandler::handleErrorMessage(const std::string& message, const std::vector<std::string>& args)
{
    if (disconnected)
    {
        return;
    }
    
    disconnected = true;
    mc->changeWorld1(nullptr);
    mc->displayGuiScreen(new GuiConnectFailed("disconnect.lost", message, args.empty() ? std::string() : args[0]));
}

void NetClientHandler::sendPacketAndFlush(Packet* packet)
{
    if (disconnected)
    {
        return;
    }
    
    static std::atomic<uint32_t> sn{0};
    if (sn.fetch_add(1) < 40)
        fprintf(stderr, "Beta++: SEND pkt id=%d\n", (int)packet->getPacketId());
    netManager->addToSendQueue(packet);
    netManager->flushQueue();  // func_28142_c
}

void NetClientHandler::addToSendQueue(Packet* packet)
{
    if (disconnected)
    {
        return;
    }
    
    static std::atomic<uint32_t> sn{0};
    if (sn.fetch_add(1) < 40)
        fprintf(stderr, "Beta++: SEND pkt id=%d\n", (int)packet->getPacketId());
    netManager->addToSendQueue(packet);
}

void NetClientHandler::handleCollect(Packet22Collect* packet)
{
    Entity* collected = getEntityByID(packet->collectedEntityId);
    EntityLiving* collector = dynamic_cast<EntityLiving*>(getEntityByID(packet->collectorEntityId));
    
    if (collector == nullptr)
    {
        collector = mc->thePlayer;
    }
    
    if (collected != nullptr)
    {
        worldClient->playSoundAtEntity(collected, "random.pop", 0.2f,
            ((rand() % 1000 / 1000.0f - rand() % 1000 / 1000.0f) * 0.7f + 1.0f) * 2.0f);
        
        mc->effectRenderer->addEffect(new EntityPickupFX(
            mc->theWorld, collected, collector, -0.5f
        ));
        
        worldClient->removeEntityFromWorld(packet->collectedEntityId);
    }
}

void NetClientHandler::handleChat(Packet3Chat* packet)
{
    mc->ingameGUI->addChatMessage(packet->message);
}

void NetClientHandler::handleArmAnimation(Packet18Animation* packet)
{
    Entity* entity = getEntityByID(packet->entityId);
    
    if (entity == nullptr)
    {
        return;
    }
    
    if (packet->animate == 1)
    {
        EntityPlayer* player = dynamic_cast<EntityPlayer*>(entity);
        if (player != nullptr)
        {
            player->swingItem();
        }
    }
    else if (packet->animate == 2)
    {
        entity->performHurtAnimation();
    }
    else if (packet->animate == 3)
    {
        EntityPlayer* player = dynamic_cast<EntityPlayer*>(entity);
        if (player != nullptr)
        {
            player->wakeUpPlayer(false, false, false);
        }
    }
    else if (packet->animate == 4)
    {
        EntityPlayer* player = dynamic_cast<EntityPlayer*>(entity);
        if (player != nullptr)
        {
            player->func_6420_o();  // Accion de comer/drink
        }
    }
}

void NetClientHandler::handleSleep(Packet17Sleep* packet)
{
    Entity* entity = getEntityByID(packet->entityId);
    
    if (entity == nullptr)
    {
        return;
    }
    
    if (packet->sleepState == 0)
    {
        EntityPlayer* player = dynamic_cast<EntityPlayer*>(entity);
        if (player != nullptr)
        {
            player->sleepInBedAt(packet->bedX, packet->bedY, packet->bedZ);
        }
    }
}

void NetClientHandler::handleHandshake(Packet2Handshake* packet)
{
    if (packet->username == "-")
    {
        addToSendQueue(new Packet1Login(mc->session->username, 14));
    }
    else
    {
        try
        {
            // HTTP request a minecraft.net para autenticacion
            // En SDL2 necesitamos usar una libreria HTTP o implementar manualmente
            // Esto es un placeholder para la logica de autenticacion
            
            std::string url = "http://www.minecraft.net/game/joinserver.jsp?user=" 
                            + mc->session->username 
                            + "&sessionId=" + mc->session->sessionId 
                            + "&serverId=" + packet->username;
            
            // TODO: Implementar HTTP request con SDL_net o curl
            // BufferedReader equivalente
            std::string response = "ok";  // Placeholder
            
            if (response == "ok" || response == "OK")
            {
                addToSendQueue(new Packet1Login(mc->session->username, 14));
            }
            else
            {
                netManager->networkShutdown("disconnect.loginFailedInfo", { response });
            }
        }
        catch (const std::exception& e)
        {
            netManager->networkShutdown("disconnect.genericReason", {
                "Internal client error: " + std::string(e.what())
            });
        }
    }
}

void NetClientHandler::disconnect()
{
    disconnected = true;
    netManager->wakeThreads();
    netManager->networkShutdown("disconnect.closed", {});
}

void NetClientHandler::handleMobSpawn(Packet24MobSpawn* packet)
{
    double posX = (double)packet->xPosition / 32.0;
    double posY = (double)packet->yPosition / 32.0;
    double posZ = (double)packet->zPosition / 32.0;
    
    float yaw = (float)(packet->yaw * 360) / 256.0f;
    float pitch = (float)(packet->pitch * 360) / 256.0f;
    
    EntityLiving* entityLiving = dynamic_cast<EntityLiving*>(
        EntityList::createEntity(packet->type, mc->theWorld)
    );
    
    entityLiving->serverPosX = packet->xPosition;
    entityLiving->serverPosY = packet->yPosition;
    entityLiving->serverPosZ = packet->zPosition;
    entityLiving->entityId = packet->entityId;
    entityLiving->setPositionAndRotation(posX, posY, posZ, yaw, pitch);
    entityLiving->isMultiplayerEntity = true;
    
    worldClient->addEntityToWorld(packet->entityId, entityLiving);
    
    auto metadata = packet->getMetadata();
    if (!metadata.empty())
    {
        entityLiving->getDataWatcher()->updateWatchedObjectsFromList(metadata);
    }
}

void NetClientHandler::handleUpdateTime(Packet4UpdateTime* packet)
{
    mc->theWorld->setWorldTime(packet->time);
}

void NetClientHandler::handleSpawnPosition(Packet6SpawnPosition* packet)
{
    mc->thePlayer->setPlayerSpawnCoordinate(
        new ChunkCoordinates(packet->xPosition, packet->yPosition, packet->zPosition)
    );
    
    mc->theWorld->getWorldInfo()->setSpawn(
        packet->xPosition,
        packet->yPosition,
        packet->zPosition
    );
}

void NetClientHandler::handleAttachEntity(Packet39AttachEntity* packet)
{
    Entity* entity = getEntityByID(packet->entityId);
    Entity* vehicle = getEntityByID(packet->vehicleEntityId);
    
    if (packet->entityId == mc->thePlayer->entityId)
    {
        entity = mc->thePlayer;
    }
    
    if (entity == nullptr)
    {
        return;
    }
    
    entity->mountEntity(vehicle);
}

void NetClientHandler::handleEntityStatus(Packet38EntityStatus* packet)
{
    Entity* entity = getEntityByID(packet->entityId);
    
    if (entity != nullptr)
    {
        entity->handleHealthUpdate(packet->entityStatus);
    }
}

Entity* NetClientHandler::getEntityByID(int entityId)
{
    if (entityId == mc->thePlayer->entityId)
    {
        return mc->thePlayer;
    }
    
    return worldClient->getEntityByID(entityId);
}

void NetClientHandler::handleHealth(Packet8UpdateHealth* packet)
{
    mc->thePlayer->setHealth(packet->health);
}

void NetClientHandler::handleRespawn(Packet9Respawn* packet)
{
    if (packet->dimension != mc->thePlayer->dimension)
    {
        terrainDownloaded = false;
        
        worldClient = new WorldClient(
            this,
            worldClient->getWorldInfo()->getRandomSeed(),
            packet->dimension
        );
        
        worldClient->multiplayerWorld = true;
        mc->changeWorld1(worldClient);
        mc->thePlayer->dimension = packet->dimension;
        mc->displayGuiScreen(new GuiDownloadTerrain(this));
    }
    
    mc->respawn(true, packet->dimension);
}

void NetClientHandler::handleExplosion(Packet60Explosion* packet)
{
    Explosion* explosion = new Explosion(
        mc->theWorld,
        nullptr,
        packet->explosionX,
        packet->explosionY,
        packet->explosionZ,
        packet->explosionSize
    );
    
    for (ChunkPosition *pos : packet->destroyedBlockPositions)
        explosion->destroyedBlockPositions.insert({pos->x, pos->y, pos->z});
    explosion->doExplosionB(true);
}

void NetClientHandler::handleOpenWindow(Packet100OpenWindow* packet)
{
    if (packet->inventoryType == 0)
    {
        // Chest
        InventoryBasic* inventory = new InventoryBasic(packet->windowTitle, packet->slotsCount);
        mc->thePlayer->displayGUIChest(inventory);
        mc->thePlayer->craftingInventory->windowId = packet->windowId;
    }
    else if (packet->inventoryType == 2)
    {
        // Furnace
        TileEntityFurnace* furnace = new TileEntityFurnace();
        mc->thePlayer->displayGUIFurnace(furnace);
        mc->thePlayer->craftingInventory->windowId = packet->windowId;
    }
    else if (packet->inventoryType == 3)
    {
        // Dispenser
        TileEntityDispenser* dispenser = new TileEntityDispenser();
        mc->thePlayer->displayGUIDispenser(dispenser);
        mc->thePlayer->craftingInventory->windowId = packet->windowId;
    }
    else if (packet->inventoryType == 1)
    {
        // Crafting table
        EntityPlayerSP* playerSP = mc->thePlayer;
        EntityPlayer* player = playerSP;
        
        mc->thePlayer->displayWorkbenchGUI(
            MathHelper::floor_double(player->posX),
            MathHelper::floor_double(player->posY),
            MathHelper::floor_double(player->posZ)
        );
        
        mc->thePlayer->craftingInventory->windowId = packet->windowId;
    }
}

void NetClientHandler::handleSetSlot(Packet103SetSlot* packet)
{
    if (packet->windowId == -1)
    {
        // Cursor item
        mc->thePlayer->inventory->setItemStack(packet->myItemStack);
    }
    else if (packet->windowId == 0 && packet->itemSlot >= 36 && packet->itemSlot < 45)
    {
        // Hotbar/armor
        ItemStack* currentStack = mc->thePlayer->inventorySlots->getSlot(packet->itemSlot)->getStack();
        
        if (packet->myItemStack != nullptr && 
            (currentStack == nullptr || currentStack->stackSize < packet->myItemStack->stackSize))
        {
            packet->myItemStack->animationsToGo = 5;
        }
        
        mc->thePlayer->inventorySlots->putStackInSlot(packet->itemSlot, packet->myItemStack);
    }
    else if (packet->windowId == mc->thePlayer->craftingInventory->windowId)
    {
        // Open container
        mc->thePlayer->craftingInventory->putStackInSlot(packet->itemSlot, packet->myItemStack);
    }
}

void NetClientHandler::handleTransaction(Packet106Transaction* packet)
{
    Container* container = nullptr;
    
    if (packet->windowId == 0)
    {
        container = mc->thePlayer->inventorySlots;
    }
    else if (packet->windowId == mc->thePlayer->craftingInventory->windowId)
    {
        container = mc->thePlayer->craftingInventory;
    }
    
    if (container != nullptr)
    {
        if (packet->accepted)
        {
            container->updateTransaction(packet->action);
        }
        else
        {
            container->onTransactionResponse(packet->action);
            addToSendQueue(new Packet106Transaction(packet->windowId, packet->action, true));
        }
    }
}

void NetClientHandler::handleWindowItems(Packet104WindowItems* packet)
{
    if (packet->windowId == 0)
    {
        mc->thePlayer->inventorySlots->putStacksInSlots(packet->itemStack.data(), (int_t)packet->itemStack.size());
    }
    else if (packet->windowId == mc->thePlayer->craftingInventory->windowId)
    {
        mc->thePlayer->craftingInventory->putStacksInSlots(packet->itemStack.data(), (int_t)packet->itemStack.size());
    }
}

void NetClientHandler::handleSignUpdate(Packet130UpdateSign* packet)
{
    if (mc->theWorld->blockExists(packet->xPosition, packet->yPosition, packet->zPosition))
    {
        TileEntity* tileEntity = mc->theWorld->getBlockTileEntity(
            packet->xPosition,
            packet->yPosition,
            packet->zPosition
        );
        
        TileEntitySign* sign = dynamic_cast<TileEntitySign*>(tileEntity);
        
        if (sign != nullptr)
        {
            for (int i = 0; i < 4; i++)
            {
                sign->signText[i] = packet->signLines[i];
            }
            
            sign->onInventoryChanged();
        }
    }
}

void NetClientHandler::handleUpdateProgressbar(Packet105UpdateProgressbar* packet)
{
    if (mc->thePlayer->craftingInventory != nullptr &&
        mc->thePlayer->craftingInventory->windowId == packet->windowId)
    {
        mc->thePlayer->craftingInventory->updateProgressBar(
            packet->progressBar,
            packet->progressBarValue
        );
    }
}

void NetClientHandler::handlePlayerInventory(Packet5PlayerInventory* packet)
{
    Entity* entity = getEntityByID(packet->entityID);
    
    if (entity != nullptr)
    {
        entity->outfitWithItem(packet->slot, packet->itemID, packet->itemDamage);
    }
}

void NetClientHandler::handleCloseWindow(Packet101CloseWindow* packet)
{
    mc->thePlayer->closeScreen();
}

void NetClientHandler::handleNotePlay(Packet54PlayNoteBlock* packet)
{
    mc->theWorld->playNoteAt(
        packet->xLocation,
        packet->yLocation,
        packet->zLocation,
        packet->instrumentType,
        packet->pitch
    );
}

void NetClientHandler::handleBedEvent(Packet70Bed* packet)
{
    int eventType = packet->bedState;

    if (eventType >= 0 && eventType < Packet70Bed::bedMessageCount &&
        !Packet70Bed::bedMessages[eventType].empty())
    {
        mc->thePlayer->addChatMessage(Packet70Bed::bedMessages[eventType]);
    }

    if (eventType == 1)
    {
        worldClient->getWorldInfo()->setRaining(true);
        worldClient->setRainStrength(1.0f);
    }
    else if (eventType == 2)
    {
        worldClient->getWorldInfo()->setRaining(false);
        worldClient->setRainStrength(0.0f);
    }
}

void NetClientHandler::handleMapData(Packet131MapData* packet)
{
    if (packet->itemId == Item::mapItem->shiftedIndex)
    {
        MapData *mapdata = ItemMap::getMapData(packet->mapId, mc->theWorld);
        if (mapdata != nullptr)
            mapdata->handleMapPacket(packet->data);
    }
    else
    {
        std::cout << "Unknown itemid: " << packet->mapId << std::endl;
    }
}

void NetClientHandler::handleDoorChange(Packet61DoorChange* packet)
{
    // func_28106_e(type, x, y, z, data): doorType = soundType, doorState = soundData
    mc->theWorld->func_28106_e(
        packet->soundType,
        packet->xPosition,
        packet->yPosition,
        packet->zPosition,
        packet->soundData
    );
}

void NetClientHandler::handleStatistic(Packet200Statistic* packet)
{
    EntityClientPlayerMP* clientPlayer = dynamic_cast<EntityClientPlayerMP*>(mc->thePlayer);
    
    if (clientPlayer != nullptr)
    {
        clientPlayer->func_27027_b(
            StatList::getStatById(packet->statId),
            packet->amount
        );
    }
}

bool NetClientHandler::isServerHandler()
{
    return false;
}
