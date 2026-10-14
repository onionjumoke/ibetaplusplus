#include "client/Minecraft.h"
#include <cstdio>
#include "Profiling.h"

#include <iostream>
#include <atomic>
#include <cstdlib>
#include <execinfo.h>
#include <unistd.h>
#include <cstring>
#include <filesystem>
#include <thread>
#include <chrono>

#include "pc/OpenGL.h"
#include "pc/CrashHandler.h"
#include "pc/lwjgl/Display.h"
#include "pc/lwjgl/Keyboard.h"
#include "pc/lwjgl/Mouse.h"

#include "java/File.h"
#include "java/String.h"
#include "java/System.h"

#include "net/minecraft/src/AchievementList.h"
#include "net/minecraft/src/AxisAlignedBB.h"
#include "net/minecraft/src/BiomeGenBase.h"
#include "net/minecraft/src/Block.h"
#include "net/minecraft/src/ChunkBlockMap.h"
#include "net/minecraft/src/EnumOptionsMappingHelper.h"
#include "net/minecraft/src/Item.h"
#include "net/minecraft/src/Material.h"
#include "net/minecraft/src/BlockGrass.h"
#include "net/minecraft/src/ChunkCoordinates.h"
#include "net/minecraft/src/ChunkProviderLoadOrGenerate.h"
#include "net/minecraft/src/ColorizerFoliage.h"
#include "net/minecraft/src/ColorizerGrass.h"
#include "net/minecraft/src/ColorizerWater.h"
#include "net/minecraft/src/EffectRenderer.h"
#include "net/minecraft/src/EntityClientPlayerMP.h"
#include "net/minecraft/src/EntityPlayer.h"
#include "net/minecraft/src/EntityPlayerSP.h"
#include "net/minecraft/src/EntityRenderer.h"
#include "net/minecraft/src/EnumMovingObjectType.h"
#include "net/minecraft/src/EnumOptions.h"
#include "net/minecraft/src/FontRenderer.h"
#include "net/minecraft/src/GameResources.h"
#ifdef MC_IOS
#include "pc/ios/IosBridge.h"
#include "IosInput.h"
#else
#include "pc/stub/IosBridge_stub.h"
#endif
#include "net/minecraft/src/GameSettings.h"
#include "net/minecraft/src/Config.h"
#include "net/minecraft/src/GuiParticle.h"
#include "net/minecraft/src/InventoryPlayer.h"
#include "net/minecraft/src/KeyBinding.h"
#include "net/minecraft/src/WorldInfo.h"
#include "net/minecraft/src/GuiInventory.h"
#include "net/minecraft/src/GuiCraftingMenu.h"
#include "net/minecraft/src/GuiIngameMenu.h"
#include "net/minecraft/src/GuiChat.h"
#include "net/minecraft/src/GuiContainer.h"
#include "net/minecraft/src/GuiGameOver.h"
#include "net/minecraft/src/GLAllocation.h"
#include "net/minecraft/src/GuiAchievement.h"
#include "net/minecraft/src/GuiChat.h"
#include "net/minecraft/src/GuiOptions.h"
#include "net/minecraft/src/GuiTextField.h"
#include "net/minecraft/src/GuiConflictWarning.h"
#include "net/minecraft/src/MinecraftException.h"
#include "net/minecraft/src/GuiConnecting.h"
#include "net/minecraft/src/GuiErrorScreen.h"
#include "net/minecraft/src/GuiGameOver.h"
#include "net/minecraft/src/GuiIngame.h"
#include "net/minecraft/src/GuiIngameMenu.h"
#include "net/minecraft/src/GuiInventory.h"
#include "net/minecraft/src/GuiMainMenu.h"
#include "net/minecraft/src/GuiSleepMP.h"
#include "net/minecraft/src/GuiUnused.h"
#include "net/minecraft/src/IChunkProvider.h"
#include "net/minecraft/src/ISaveFormat.h"
#include "net/minecraft/src/ISaveHandler.h"
#include "net/minecraft/src/ItemRenderer.h"
#include "net/minecraft/src/ItemStack.h"
#include "net/minecraft/src/LoadingScreenRenderer.h"
#include "net/minecraft/src/MathHelper.h"
#include "net/minecraft/src/ModelBiped.h"
#include "net/minecraft/src/MouseHelper.h"
#include "net/minecraft/src/MovementInputFromOptions.h"
#include "net/minecraft/src/MovingObjectPosition.h"
#include "net/minecraft/src/NetClientHandler.h"
#include "net/minecraft/src/OpenGlCapsChecker.h"
#include "net/minecraft/src/PlayerController.h"
#include "net/minecraft/src/PlayerControllerTest.h"
#include "net/minecraft/src/PlayerControllerSP.h"
#include "net/minecraft/src/KeyBinding.h"
#include "net/minecraft/src/RenderBlocks.h"
#include "net/minecraft/src/RenderEngine.h"
#include "net/minecraft/src/RenderGlobal.h"
#include "net/minecraft/src/RenderManager.h"
#include "net/minecraft/src/SaveConverterMcRegion.h"
#include "net/minecraft/src/ScaledResolution.h"
#include "net/minecraft/src/ScreenShotHelper.h"
#include "net/minecraft/src/Session.h"
#include "net/minecraft/src/SoundManager.h"
#include "net/minecraft/src/StatFileWriter.h"
#include "net/minecraft/src/StatList.h"
#include "net/minecraft/src/StatStringFormatKeyInv.h"
#include "net/minecraft/src/Teleporter.h"
#include "net/minecraft/src/Tessellator.h"
#include "net/minecraft/src/TextureCompassFX.h"
#include "net/minecraft/src/TextureFlamesFX.h"
#include "net/minecraft/src/TextureLavaFX.h"
#include "net/minecraft/src/TextureLavaFlowFX.h"
#include "net/minecraft/src/TexturePackList.h"
#include "net/minecraft/src/TexturePortalFX.h"
#include "net/minecraft/src/TextureWatchFX.h"
#include "net/minecraft/src/TextureWaterFX.h"
#include "net/minecraft/src/TextureWaterFlowFX.h"
#include "net/minecraft/src/ThreadCheckHasPaid.h"
#include "net/minecraft/src/ThreadDownloadResources.h"
#include "net/minecraft/src/ThreadSleepForever.h"
#include "net/minecraft/src/Timer.h"
#include "net/minecraft/src/UnexpectedThrowable.h"
#include "net/minecraft/src/Vec3D.h"
#include "net/minecraft/src/World.h"
#include "net/minecraft/src/WorldProvider.h"
#include "net/minecraft/src/WorldRenderer.h"

// ─── Static members ───────────────────────────────────────────────────────────

byte_t  Minecraft::field_28006_b[0xa00000] = {};
long_t  Minecraft::frameTimes[512]          = {};
long_t  Minecraft::tickTimes[512]           = {};
int_t   Minecraft::numRecordedFrameTimes    = 0;
long_t  Minecraft::hasPaidCheckTime         = 0L;
Minecraft *Minecraft::theMinecraft          = nullptr;
File      *Minecraft::minecraftDir          = nullptr;

// ─── Constructor / Destructor ─────────────────────────────────────────────────

Minecraft::Minecraft(int_t width, int_t height, bool flag) :
    playerController(nullptr),
    displayWidth(width),
    displayHeight(height),
    theWorld(nullptr),
    renderGlobal(nullptr),
    thePlayer(nullptr),
    renderViewEntity(nullptr),
    effectRenderer(nullptr),
    session(nullptr),
    minecraftUri(),
    hideQuitButton(true),
    isGamePaused(false),
    renderEngine(nullptr),
    fontRenderer(nullptr),
    currentScreen(nullptr),
    loadingScreen(nullptr),
    entityRenderer(nullptr),
    ticksRan(0),
    guiAchievement(nullptr),
    ingameGUI(nullptr),
    skipRenderWorld(false),
    field_9242_w(nullptr),
    objectMouseOver(nullptr),
    gameSettings(nullptr),
    sndManager(nullptr),
    mouseHelper(nullptr),
    texturePackList(nullptr),
    running(true),
    debug(),
    inGameHasFocus(false),
    isRaining(false),
    fullscreen(flag),
    hasCrashed(false),
    glCapabilities(nullptr),
    timer(nullptr),
    downloadResourcesThread(nullptr),
    leftClickCounter(0),
    tempDisplayWidth(width),
    tempDisplayHeight(height),
    mcDataDir(nullptr),
    saveLoader(nullptr),
    statFileWriter(nullptr),
    serverName(),
    serverPort(0),
    textureWaterFX(nullptr),
    textureLavaFX(nullptr),
    isTakingScreenshot(false),
    prevFrameTime(-1L),
    mouseTicksRan(0),
    systemTime(System::currentTimeMillis()),
    joinPlayerCounter(0)
{
    Material::initialize();
    Block::initialize();
    ChunkBlockMap::initialize();
    Item::initItems();
    BiomeGenBase::initialize();
    AchievementList::initialize();
    EnumOptionsMappingHelper::initialize();
    Session::initialize();

    timer        = new Timer(20.0f);
    loadingScreen = new LoadingScreenRenderer(this);
    guiAchievement = new GuiAchievement(this);
    field_9242_w   = new ModelBiped(0.0f);
    sndManager     = new SoundManager();
    textureWaterFX = new TextureWaterFX();
    textureLavaFX  = new TextureLavaFX();

    StatList::initStats();

    theMinecraft = this;

    new ThreadSleepForever(this, "Timer hack thread");
}

Minecraft::~Minecraft()
{
    delete timer;
    delete loadingScreen;
    delete guiAchievement;
    delete field_9242_w;
    delete sndManager;
    delete textureWaterFX;
    delete textureLavaFX;
}

// ─── Static start ─────────────────────────────────────────────────────────────

void Minecraft::start(const jstring *username, const jstring *sessionId)
{
    // Use whatever surface the platform Display is bound to (window size on
    // desktop, EAGL backing size on iOS) instead of hardcoded PC defaults —
    // setupScreen() derives glViewport and scaled GUI metrics from these.
    int_t w = lwjgl::Display::getDisplayMode().getWidth();
    int_t h = lwjgl::Display::getDisplayMode().getHeight();
    if (w <= 0) w = 854;
    if (h <= 0) h = 480;
    Minecraft *mc = new Minecraft(w, h, false);
    mc->session = new Session(
        String::toUTF8(*username),
        String::toUTF8(*sessionId)
    );
    mc->run();
    delete mc;
}

Minecraft *Minecraft::getMinecraft()
{
    return theMinecraft;
}

// ─── Crash handler ───────────────────────────────────────────────────────────

void Minecraft::onMinecraftCrash(UnexpectedThrowable *unexpectedthrowable)
{
    hasCrashed = true;
    displayUnexpectedThrowable(unexpectedthrowable);
}

void Minecraft::displayUnexpectedThrowable(UnexpectedThrowable *unexpectedthrowable)
{
    CrashHandler::Crash(unexpectedthrowable ? String::toUTF8(unexpectedthrowable->description) : "Unknown crash");
}

// ─── Server ──────────────────────────────────────────────────────────────────

void Minecraft::setServer(const std::string &s, int_t i)
{
    serverName = s;
    serverPort = i;
}

// ─── Directory helpers ────────────────────────────────────────────────────────

File *Minecraft::getMinecraftDir()
{
    if (minecraftDir == nullptr)
    {
        std::filesystem::path localDir =
            std::filesystem::path(GameResources::getExeDir()) / ".minecraft";
        minecraftDir = File::open(String::fromUTF8(localDir.string()));
        minecraftDir->mkdirs();
    }
    return minecraftDir;
}

File *Minecraft::getAppDir(const std::string & /*s*/)
{
    return getMinecraftDir();
}

// ─── startGame ───────────────────────────────────────────────────────────────

void Minecraft::startGame()
{
    if (fullscreen)
    {
        lwjgl::Display::setFullscreen(true);
        displayWidth  = lwjgl::Display::getDisplayMode().getWidth();
        displayHeight = lwjgl::Display::getDisplayMode().getHeight();
        if (displayWidth  <= 0) displayWidth  = 1;
        if (displayHeight <= 0) displayHeight = 1;
    }
    else
    {
        lwjgl::Display::setDisplayMode(lwjgl::DisplayMode(displayWidth, displayHeight));
    }

    lwjgl::Display::setTitle(u"Minecraft Beta 1.7.3");
    lwjgl::Display::create();
    fprintf(stderr, "Beta++: startGame Display.create done\n");

    mcDataDir  = getMinecraftDir();
    std::unique_ptr<File> savesDirectory(File::open(*mcDataDir, u"saves"));
    saveLoader = new SaveConverterMcRegion(String::toUTF8(savesDirectory->toString()));
    gameSettings    = new GameSettings(this, String::toUTF8(mcDataDir->toString()));
    texturePackList = new TexturePackList(this, String::toUTF8(mcDataDir->toString()));
    renderEngine    = new RenderEngine(texturePackList, gameSettings);
    fontRenderer    = new FontRenderer(gameSettings, "/font/default.png", renderEngine);
    fprintf(stderr, "Beta++: startGame render engine + fonts ready\n");

    ColorizerWater::func_28182_a(renderEngine->func_28149_a("/misc/watercolor.png"));
    ColorizerGrass::func_28181_a(renderEngine->func_28149_a("/misc/grasscolor.png"));
    ColorizerFoliage::func_28152_a(renderEngine->func_28149_a("/misc/foliagecolor.png"));

    new RenderManager();  // constructor sets RenderManager::instance = this
    entityRenderer = new EntityRenderer(this);
    RenderManager::instance->itemRenderer = new ItemRenderer(this);
    statFileWriter = new StatFileWriter(session, String::toUTF8(mcDataDir->toString()));
    AchievementList::openInventory->setStatStringFormatter(new StatStringFormatKeyInv(this));

    loadScreen();
    fprintf(stderr, "Beta++: startGame loadScreen done\n");

    mouseHelper = new MouseHelper(nullptr);

    checkGLError("Pre startup");

    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);
    glCullFace(GL_BACK);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    checkGLError("Startup");

    glCapabilities = new OpenGlCapsChecker();
    sndManager->loadSoundSettings(gameSettings);

    // OptiFine: registrar las animaciones de textura segun los toggles (sin variantes HD).
    // Si no se registran, la textura estatica de terrain.png permanece (= animacion OFF).
    if (Config::isAnimatedLava())
        renderEngine->registerTextureFX(textureLavaFX);
    if (Config::isAnimatedWater())
        renderEngine->registerTextureFX(textureWaterFX);
    if (Config::isAnimatedPortal())
        renderEngine->registerTextureFX(new TexturePortalFX());
    renderEngine->registerTextureFX(new TextureCompassFX(this));
    renderEngine->registerTextureFX(new TextureWatchFX(this));
    if (Config::isAnimatedWater())
        renderEngine->registerTextureFX(new TextureWaterFlowFX());
    if (Config::isAnimatedLava())
        renderEngine->registerTextureFX(new TextureLavaFlowFX());
    if (Config::isAnimatedFire())
    {
        renderEngine->registerTextureFX(new TextureFlamesFX(0));
        renderEngine->registerTextureFX(new TextureFlamesFX(1));
    }

    renderGlobal = new RenderGlobal(this, renderEngine);
    glViewport(0, 0, displayWidth, displayHeight);
    effectRenderer = new EffectRenderer(theWorld, renderEngine);

    try
    {
        downloadResourcesThread = new ThreadDownloadResources(
            String::toUTF8(mcDataDir->toString()), this);
        downloadResourcesThread->start();
    }
    catch (...) {}

    checkGLError("Post startup");

    ingameGUI = new GuiIngame(this);

    if (!serverName.empty())
        displayGuiScreen(new GuiConnecting(this, serverName, serverPort));
    else
        displayGuiScreen(new GuiMainMenu());
}

// ─── loadScreen ──────────────────────────────────────────────────────────────

void Minecraft::loadScreen()
{
    ScaledResolution scaledresolution(gameSettings, displayWidth, displayHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, scaledresolution.field_25121_a, scaledresolution.field_25120_b, 0.0, 1000.0, 3000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -2000.0f);
    glViewport(0, 0, displayWidth, displayHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    Tessellator *tessellator = &Tessellator::instance;
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_FOG);
    glBindTexture(GL_TEXTURE_2D, renderEngine->getTexture("/title/mojang.png"));
    tessellator->startDrawingQuads();
    tessellator->setColorOpaque_I(0xffffff);
    tessellator->addVertexWithUV(0.0,          displayHeight, 0.0, 0.0, 0.0);
    tessellator->addVertexWithUV(displayWidth,  displayHeight, 0.0, 0.0, 0.0);
    tessellator->addVertexWithUV(displayWidth,  0.0,           0.0, 0.0, 0.0);
    tessellator->addVertexWithUV(0.0,           0.0,           0.0, 0.0, 0.0);
    tessellator->draw();

    int_t c  = 0x100;
    int_t c1 = 0x100;
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    tessellator->setColorOpaque_I(0xffffff);
    func_6274_a((scaledresolution.getScaledWidth() - c) / 2,
                (scaledresolution.getScaledHeight() - c1) / 2,
                0, 0, c, c1);

    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);
    lwjgl::Display::swapBuffers();
}

void Minecraft::func_6274_a(int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1)
{
    float f  = 0.00390625f;
    float f1 = 0.00390625f;
    Tessellator *tessellator = &Tessellator::instance;
    tessellator->startDrawingQuads();
    tessellator->addVertexWithUV(i + 0,  j + j1, 0.0, (float)(k + 0)  * f,  (float)(l + j1) * f1);
    tessellator->addVertexWithUV(i + i1, j + j1, 0.0, (float)(k + i1) * f,  (float)(l + j1) * f1);
    tessellator->addVertexWithUV(i + i1, j + 0,  0.0, (float)(k + i1) * f,  (float)(l + 0)  * f1);
    tessellator->addVertexWithUV(i + 0,  j + 0,  0.0, (float)(k + 0)  * f,  (float)(l + 0)  * f1);
    tessellator->draw();
}

// ─── GL error check ──────────────────────────────────────────────────────────

void Minecraft::checkGLError(const std::string &s)
{
    int err = glGetError();
    if (err != 0)
    {
        std::cout << "########## GL ERROR ##########" << std::endl;
        std::cout << "@ " << s << std::endl;
        std::cout << err << ": GL error" << std::endl;
    }
}

// ─── Shutdown ─────────────────────────────────────────────────────────────────

void Minecraft::shutdownMinecraftApplet()
{
    try
    {
        statFileWriter->func_27175_b();
        statFileWriter->syncStats();

        try
        {
            if (downloadResourcesThread != nullptr)
                downloadResourcesThread->closeMinecraft();
        }
        catch (...) {}

        std::cout << "Stopping!" << std::endl;

        try { changeWorld1(nullptr); } catch (...) {}
        // No habra mas runTick que vacie la cola diferida: borrar el mundo ahora
        // (en shutdown no hay ningun World::tick() en la pila).
        try
        {
            for (World *w : worldsToDelete)
                delete w;
            worldsToDelete.clear();
        }
        catch (...) {}
        try { GLAllocation::deleteTexturesAndDisplayLists(); } catch (...) {}

        sndManager->closeMinecraft();
    }
    catch (...)
    {
        lwjgl::Display::swapBuffers();
        if (!hasCrashed)
            exit(0);
    }
    lwjgl::Display::swapBuffers();
    if (!hasCrashed)
        exit(0);
}

// ─── Main game loop ──────────────────────────────────────────────────────────

void Minecraft::run()
{
    running = true;

    try
    {
        startGame();
        fprintf(stderr, "Beta++: startGame() completed, entering main loop\n");
    }
    catch (...)
    {
        onMinecraftCrash(new UnexpectedThrowable("Failed to start game"));
        return;
    }

    try
    {
        long_t l = System::currentTimeMillis();
        int_t i  = 0;

        // Frame-stall watchdog: the previous frame's phase timings are copied
        // here so that a >300 ms frame can be attributed to tick/render/swap.
        static long_t prevFrameStartNs = 0;
        static double lastTickMs = 0.0, lastRenderMs = 0.0, lastSwapMs = 0.0, lastPresentMs = 0.0;

        while (running)
        {
            {
                long_t frameStartNs = System::nanoTime();
                if (prevFrameStartNs != 0)
                {
                    double gapMs = (double)(frameStartNs - prevFrameStartNs) / 1e6;
                    if (gapMs > 300.0)
                        fprintf(stderr,
                                "Beta++: STALL frame=%.0fms tick=%.1f render=%.1f swap=%.1f present=%.1f world=%p screen=%p\n",
                                gapMs, lastTickMs, lastRenderMs, lastSwapMs, lastPresentMs,
                                (void *)theWorld, (void *)currentScreen);
                }
                prevFrameStartNs = frameStartNs;
            }
            try
            {
                AxisAlignedBB::clearBoundingBoxPool();
                Vec3D::initialize();

                if (lwjgl::Display::isCloseRequested())
                    shutdown();

                if (isGamePaused && theWorld != nullptr)
                {
                    float f = timer->renderPartialTicks;
                    timer->updateTimer();
                    timer->renderPartialTicks = f;
                }
                else
                {
                    timer->updateTimer();
                }

                // TEMP diagnostics: auto-walk a superflat world & watch for stuck.
                if (getenv("IBPP_MOVETEST") != nullptr)
                {
                    static bool s_started = false;
                    static bool s_forwardDown = false;
                    if (!s_started && theWorld == nullptr)
                    {
                        s_started = true;
                        fprintf(stderr, "Beta++: MOVETEST creating superflat world\n");
                        playerController = new PlayerControllerSP(this);
                        World::setNextWorldType(getenv("IBPP_MOVETEST_FLAT") ? 1 : 0);
                        startWorld(getenv("IBPP_MOVETEST_FLAT") ? "ibpp_move_test" : "ibpp_move_test2",
                                   "Move Test", 4242);
                        displayGuiScreen(nullptr);
                    }
                    if (theWorld != nullptr && thePlayer != nullptr && !s_forwardDown)
                    {
                        s_forwardDown = true;
                        thePlayer->handleKeyPress(gameSettings->keyBindForward->keyCode, true);
                        fprintf(stderr, "Beta++: MOVETEST forward held\n");
                    }
                    if (theWorld != nullptr && thePlayer != nullptr && !isGamePaused)
                    {
                        // Auto-jump over 1-block steps so "stuck" means a real bug.
                        bool j = (thePlayer->isCollidedHorizontally && thePlayer->onGround);
                        thePlayer->handleKeyPress(gameSettings->keyBindJump->keyCode, j);
                    }
                }

                long_t l1 = System::nanoTime();
                for (int_t j = 0; j < timer->elapsedTicks; j++)
                {
                    ticksRan++;
                    try
                    {
                        runTick();
                    }
                    catch (const MinecraftException &)
                    {
                        // Java only catches MinecraftException here (the session-lock conflict).
                        // Any other exception must propagate so the real error surfaces instead of
                        // being misreported as a "Level save conflict".
                        World *leakedWorld = theWorld;
                        theWorld = nullptr;
                        changeWorld1(nullptr);
                        // Java relied on GC here; with theWorld nulled, changeWorld1 sees no
                        // oldWorld and skips the save (correct for a lock conflict). Free it ourselves.
                        delete leakedWorld;
                        displayGuiScreen(new GuiConflictWarning());
                    }
                }

                long_t l2 = System::nanoTime() - l1;
                long_t g1 = System::nanoTime();
                checkGLError("Pre render");
                RenderBlocks::fancyGrass = gameSettings->fancyGraphics;
                sndManager->setListenerPosition(thePlayer, timer->renderPartialTicks);
                glEnable(GL_TEXTURE_2D);

                if (theWorld != nullptr)
                    theWorld->updatingLighting();

                long_t swapNs = 0;
                {
                    long_t sw1 = System::nanoTime();
                    if (!lwjgl::Keyboard::isKeyDown(0x41))
                        lwjgl::Display::update();
                    swapNs += System::nanoTime() - sw1;
                }

                if (thePlayer != nullptr && thePlayer->isEntityInsideOpaqueBlock())
                    gameSettings->thirdPersonView = false;

                if (!skipRenderWorld)
                {
                    if (playerController != nullptr)
                        playerController->setPartialTime(timer->renderPartialTicks);
#ifdef MC_IOS
                    // Overlay visibility + pending username (iOS bridge).
                    ::iosapp::setOverlayMode(
                        theWorld == nullptr ? 0 : (currentScreen != nullptr ? 2 : 1));
                    {
                        // Compact HUD: coords + fps, republished ~4x/second.
                        static long_t hudLastMs = 0;
                        static int hudFrames = 0;
                        hudFrames++;
                        long_t hudNow = ::System::currentTimeMillis();
                        if (hudNow - hudLastMs >= 250)
                        {
                            float hudFps = hudLastMs == 0
                                ? 0.0f
                                : (float)hudFrames * 1000.0f / (float)(hudNow - hudLastMs);
                            hudLastMs = hudNow;
                            hudFrames = 0;
                            char hudBuf[192];
                            if (thePlayer != nullptr)
                            {
                                snprintf(hudBuf, sizeof(hudBuf),
                                         "XYZ %.1f %.1f %.1f   %.0f fps",
                                         thePlayer->posX, thePlayer->posY,
                                         thePlayer->posZ, hudFps);
                            }
                            else
                            {
                                snprintf(hudBuf, sizeof(hudBuf), "%.0f fps", hudFps);
                            }
                            ::iosapp::publishHud(hudBuf);
                        }
                    }
                    {
                        GuiChat *chatScreen = dynamic_cast<GuiChat *>(currentScreen);
                        GuiTextField *focusedField =
                            (currentScreen != nullptr) ? currentScreen->getFocusedTextField() : nullptr;

                        if (focusedField != nullptr)
                            ::iosapp::publishTextInput(focusedField->getText());
                        else if (chatScreen != nullptr)
                            ::iosapp::publishTextInput(chatScreen->getChatMessage());
                        else
                            ::iosapp::publishTextInput(std::string());

                        ::iosapp::publishInputTarget(focusedField != nullptr || chatScreen != nullptr);

                        // Tapping a text field pops the keyboard once, prefilled
                        // with the field's current text so it can be edited
                        // (replaced) rather than only appended to.
                        static GuiTextField *s_lastFocusedField = nullptr;
                        if (focusedField != nullptr && focusedField != s_lastFocusedField &&
                            !::iosapp::isTextInputActive())
                            ::iosapp::showFieldInput(focusedField->getText().c_str());
                        s_lastFocusedField = focusedField;

                        // The edit popup closed: drop field focus so tapping the
                        // field again brings it back (focus is otherwise
                        // unchanged, which would suppress the re-open).
                        if (::iosapp::takePendingFieldBlur())
                        {
                            if (focusedField != nullptr)
                                focusedField->setFocused(false);
                            s_lastFocusedField = nullptr;
                        }

                        if (::iosapp::takePendingKeyboardShow())
                            ::iosapp::showTextInput(nullptr);
                    }
                    {
                        std::string newName;
                        if (::iosapp::consumePendingTextInput(newName) && !newName.empty())
                        {
                            session->username = newName;
                            if (thePlayer != nullptr)
                                thePlayer->username = String::fromUTF8(newName);
                            FILE *uf = fopen(
                                (std::filesystem::path(GameResources::getExeDir()) /
                                 "username.txt")
                                    .string()
                                    .c_str(),
                                "w");
                            if (uf != nullptr)
                            {
                                fputs(newName.c_str(), uf);
                                fclose(uf);
                            }
                            fprintf(stderr, "Beta++: username set to '%s'\n",
                                    newName.c_str());
                        }
                    }

                    {
                        static long_t lastProbe = 0;
                        long_t nowP = ::System::currentTimeMillis();
                        if (theWorld != nullptr && thePlayer != nullptr &&
                            nowP - lastProbe > 2000)
                        {
                            lastProbe = nowP;
                            int px = MathHelper::floor_double(thePlayer->posX);
                            int py = MathHelper::floor_double(thePlayer->posY);
                            int pz = MathHelper::floor_double(thePlayer->posZ);
                            fprintf(stderr,
                                    "Beta++: light @(%d,%d,%d) above=%d surf=%d "
                                    "-1=%d -3=%d b=%d,%d,%d,%d\n",
                                    px, py, pz,
                                    theWorld->getSavedLightValue(EnumSkyBlock::Sky, px, py + 2, pz),
                                    theWorld->getSavedLightValue(EnumSkyBlock::Sky, px, py, pz),
                                    theWorld->getSavedLightValue(EnumSkyBlock::Sky, px, py - 1, pz),
                                    theWorld->getSavedLightValue(EnumSkyBlock::Sky, px, py - 3, pz),
                                    theWorld->getBlockId(px, py - 1, pz),
                                    theWorld->getBlockId(px, py - 2, pz),
                                    theWorld->getBlockId(px, py - 3, pz),
                                    theWorld->getBlockId(px, py - 6, pz));
                        }
                    }

#endif // MC_IOS

                    entityRenderer->updateCameraAndRender(timer->renderPartialTicks);
                }

                if (!lwjgl::Display::isActive())
                {
                    if (fullscreen)
                        toggleFullscreen();
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }

                if (gameSettings->showDebugInfo)
                    displayDebugInfo(l2);
                else
                    prevFrameTime = System::nanoTime();

                guiAchievement->updateAchievementWindow();
                long_t g2 = System::nanoTime(); // after entityRenderer->updateCameraAndRender

                if (lwjgl::Keyboard::isKeyDown(0x41))
                {
                    long_t sw1 = System::nanoTime();
                    lwjgl::Display::update();
                    swapNs += System::nanoTime() - sw1;
                }
                long_t g3 = System::nanoTime();

                // Frame-phase profiling (stderr, cheap): catches which phase
                // eats the time on slow devices. Averages over 120 frames.
                {
                    static double accTicks = 0, accRender = 0, accPresent = 0;
                    static int accN = 0;
                    static long worstTicks = 0, worstRender = 0, worstPresent = 0;
                    accTicks += l2 / 1e6; accRender += (g2 - g1) / 1e6; accPresent += (g3 - g2) / 1e6;
                    if (l2 > worstTicks) worstTicks = l2;
                    if ((g2 - g1) > worstRender) worstRender = g2 - g1;
                    if ((g3 - g2) > worstPresent) worstPresent = g3 - g2;
                    if (++accN >= 120)
                    {
                        ::Profiling::advanceFrame();
                        int cnt = ::Profiling::currentCount();
                        fprintf(stderr,
                            "Beta++: phase ms/frame avg tick=%.1f render=%.1f present=%.1f "
                            "| worst tick=%ld render=%ld present=%ld\n",
                            accTicks / accN, accRender / accN, accPresent / accN,
                            worstTicks, worstRender, worstPresent);
                        fprintf(stderr,
                            "  sub ms: world=%.1f ents=%.1f ctrl=%.1f texfx=%.1f rdisp=%.1f fx=%.1f light=%.1f randb=%.1f spawn=%.1f stat=%.1f ing=%.1f mo=%.1f | clip=%.1f upd=%.1f draw=%.1f entsR=%.1f gui=%.1f\n",
                            ::Profiling::takeAvgMs(::Profiling::S_TICK_WORLD, &cnt),
                            ::Profiling::takeAvgMs(::Profiling::S_TICK_ENTITIES, &cnt),
                            ::Profiling::takeAvgMs(::Profiling::S_TICK_CONTROLLER, &cnt),
                            ::Profiling::takeAvgMs(::Profiling::S_TICK_TEXFX, &cnt),
                            ::Profiling::takeAvgMs(::Profiling::S_TICK_RANDOMDISP, &cnt),
                            ::Profiling::takeAvgMs(::Profiling::S_TICK_EFFECTS, &cnt),
                            ::Profiling::takeAvgMs(::Profiling::S_TICK_LIGHT, &cnt),
                            ::Profiling::takeAvgMs(::Profiling::S_TICK_RANDBLOCK, &cnt),
                            ::Profiling::takeAvgMs(::Profiling::S_TICK_SPAWNER, &cnt),
                            ::Profiling::takeAvgMs(::Profiling::S_TICK_STAT, &cnt),
                            ::Profiling::takeAvgMs(::Profiling::S_TICK_INGAME, &cnt),
                            ::Profiling::takeAvgMs(::Profiling::S_TICK_MOUSEOVER, &cnt),
                            ::Profiling::takeAvgMs(::Profiling::S_REND_CLIP, &cnt),
                            ::Profiling::takeAvgMs(::Profiling::S_REND_UPDATE, &cnt),
                            ::Profiling::takeAvgMs(::Profiling::S_REND_SORTDRAW, &cnt),
                            ::Profiling::takeAvgMs(::Profiling::S_REND_ENTITIES, &cnt),
                            ::Profiling::takeAvgMs(::Profiling::S_REND_GUI, &cnt));
                        fprintf(stderr,
                            "  sub2 ms: pre=%.1f mouse=%.1f world=%.1f\n",
                            ::Profiling::takeAvgMs(::Profiling::S_REND_PRE, &cnt),
                            ::Profiling::takeAvgMs(::Profiling::S_REND_MOUSE, &cnt),
                            ::Profiling::takeAvgMs(::Profiling::S_REND_WORLD, &cnt));
                        accTicks = accRender = accPresent = 0; accN = 0;
                        worstTicks = worstRender = worstPresent = 0;
                    }
                }

                screenshotListener();

                // Resize check (no AWT canvas; use Display size directly)
                {
                    int_t w = lwjgl::Display::getDisplayMode().getWidth();
                    int_t h = lwjgl::Display::getDisplayMode().getHeight();
                    if (!fullscreen && (w != displayWidth || h != displayHeight))
                    {
                        displayWidth  = (w  <= 0) ? 1 : w;
                        displayHeight = (h <= 0) ? 1 : h;
                        resize(displayWidth, displayHeight);
                    }
                }

                lastTickMs   = l2 / 1e6;
                lastRenderMs = (double)(g2 - g1) / 1e6;
                lastSwapMs   = swapNs / 1e6;
                lastPresentMs = (double)(g3 - g2) / 1e6;

                checkGLError("Post render");
                i++;
                isGamePaused = !isMultiplayerWorld() &&
                               currentScreen != nullptr &&
                               currentScreen->doesGuiPauseGame();

                while (System::currentTimeMillis() >= l + 1000L)
                {
                    debug = std::to_string(i) + " fps, " +
                            std::to_string(WorldRenderer::chunksUpdated) + " chunk updates";
                    WorldRenderer::chunksUpdated = 0;
                    l += 1000L;
                    i = 0;

                    if (theWorld != nullptr && thePlayer != nullptr)
                    {
                        static double spx = 1e9, spy = 1e9, spz = 1e9;
                        static int stall = 0;
                        bool same = std::fabs(thePlayer->posX - spx) < 0.01 &&
                                    std::fabs(thePlayer->posY - spy) < 0.01 &&
                                    std::fabs(thePlayer->posZ - spz) < 0.01;
                        stall = same ? stall + 1 : 0;
                        spx = thePlayer->posX; spy = thePlayer->posY; spz = thePlayer->posZ;
                        const char *tag = "-";
                        if (currentScreen != nullptr)
                        {
                            if (dynamic_cast<GuiInventory *>(currentScreen) != nullptr) tag = "INVENTORY";
                            else if (dynamic_cast<GuiCraftingMenu *>(currentScreen) != nullptr) tag = "CRAFTING";
                            else if (dynamic_cast<GuiContainer *>(currentScreen) != nullptr) tag = "CONTAINER";
                            else if (dynamic_cast<GuiIngameMenu *>(currentScreen) != nullptr) tag = "PAUSEMENU";
                            else if (dynamic_cast<GuiChat *>(currentScreen) != nullptr) tag = "CHAT";
                            else if (dynamic_cast<GuiGameOver *>(currentScreen) != nullptr) tag = "GAMEOVER";
                            else tag = "other";
                        }
                        fprintf(stderr,
                                "Beta++: STUCKDBG wt=%lld stall=%d paused=%d scr=%p pauseGui=%d tag=%s "
                                "pos=(%.2f,%.2f,%.2f) last=(%.2f,%.2f,%.2f) onGround=%d cHz=%d cV=%d fwd=%.2f\n",
                                (long long)theWorld->getWorldInfo()->getWorldTime(), stall,
                                (int)isGamePaused, (void *)currentScreen,
                                currentScreen ? (int)currentScreen->doesGuiPauseGame() : 0, tag,
                                thePlayer->posX, thePlayer->posY, thePlayer->posZ,
                                thePlayer->lastTickPosX, thePlayer->lastTickPosY, thePlayer->lastTickPosZ,
                                (int)thePlayer->onGround, (int)thePlayer->isCollidedHorizontally,
                                (int)thePlayer->isCollidedVertically,
                                thePlayer->movementInput ? thePlayer->movementInput->moveForward : 0.0f);
                    }
                }
            }
            catch (const MinecraftException &)
            {
                World *leakedWorld = theWorld;
                theWorld = nullptr;
                changeWorld1(nullptr);
                // See note above: free the orphaned world the Java GC would have collected.
                delete leakedWorld;
                displayGuiScreen(new GuiConflictWarning());
            }
            catch (const std::bad_alloc &)
            {
                // Java: catch(OutOfMemoryError)
                func_28002_e();
                displayGuiScreen(new GuiErrorScreen());
            }
        }
    }
    catch (const std::exception &throwable)
    {
        // Java: catch(Throwable) -> print stack trace + onMinecraftCrash.
        // Surface the real error instead of swallowing it.
        func_28002_e();
        std::cerr << "Unexpected error: " << throwable.what() << std::endl;
        onMinecraftCrash(new UnexpectedThrowable("Unexpected error"));
    }
    catch (...)
    {
        func_28002_e();
        std::cerr << "Unexpected error (non-std exception)" << std::endl;
        onMinecraftCrash(new UnexpectedThrowable("Unexpected error"));
    }

    shutdownMinecraftApplet();
}

// ─── func_28002_e ────────────────────────────────────────────────────────────

void Minecraft::func_28002_e()
{
    try { std::memset(field_28006_b, 0, sizeof(field_28006_b)); } catch (...) {}
    try
    {
        renderGlobal->clearWorldRenderers();
    }
    catch (...) {}
    try
    {
        AxisAlignedBB::func_28196_a();
        Vec3D::func_28215_a();
    }
    catch (...) {}
    try { changeWorld1(nullptr); } catch (...) {}
}

// ─── screenshotListener ──────────────────────────────────────────────────────

void Minecraft::screenshotListener()
{
    // F2 = 0x3C
    if (lwjgl::Keyboard::isKeyDown(0x3C))
    {
        if (!isTakingScreenshot)
        {
            isTakingScreenshot = true;
            ingameGUI->addChatMessage(
                ScreenShotHelper::saveScreenshot(
                    String::toUTF8(minecraftDir->toString()), displayWidth, displayHeight));
        }
    }
    else
    {
        isTakingScreenshot = false;
    }
}

// ─── displayDebugInfo ────────────────────────────────────────────────────────

void Minecraft::displayDebugInfo(long_t l)
{
    constexpr long_t l1 = 0xfe502aL;

    if (prevFrameTime == -1L)
        prevFrameTime = System::nanoTime();

    long_t l2 = System::nanoTime();
    tickTimes[numRecordedFrameTimes & (512 - 1)] = l;
    frameTimes[numRecordedFrameTimes++ & (512 - 1)] = l2 - prevFrameTime;
    prevFrameTime = l2;

    glClear(GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, displayWidth, displayHeight, 0.0, 1000.0, 3000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -2000.0f);
    glLineWidth(1.0f);
    glDisable(GL_TEXTURE_2D);

    Tessellator *tessellator = &Tessellator::instance;
    int_t i = (int_t)(l1 / 0x30d40L);

    tessellator->startDrawing(GL_QUADS);
    tessellator->setColorOpaque_I(0x20000000);
    tessellator->addVertex(0.0,               displayHeight - i, 0.0);
    tessellator->addVertex(0.0,               displayHeight,     0.0);
    tessellator->addVertex(512,               displayHeight,     0.0);
    tessellator->addVertex(512,               displayHeight - i, 0.0);
    tessellator->setColorOpaque_I(0x20200000);
    tessellator->addVertex(0.0,               displayHeight - i * 2, 0.0);
    tessellator->addVertex(0.0,               displayHeight - i,     0.0);
    tessellator->addVertex(512,               displayHeight - i,     0.0);
    tessellator->addVertex(512,               displayHeight - i * 2, 0.0);
    tessellator->draw();

    long_t l3 = 0L;
    for (int_t j = 0; j < 512; j++)
        l3 += frameTimes[j];

    int_t k = (int_t)(l3 / 0x30d40L / 512L);

    tessellator->startDrawing(GL_QUADS);
    tessellator->setColorOpaque_I(0x20400000);
    tessellator->addVertex(0.0,  displayHeight - k, 0.0);
    tessellator->addVertex(0.0,  displayHeight,     0.0);
    tessellator->addVertex(512,  displayHeight,     0.0);
    tessellator->addVertex(512,  displayHeight - k, 0.0);
    tessellator->draw();

    tessellator->startDrawing(GL_LINES);
    for (int_t i1 = 0; i1 < 512; i1++)
    {
        int_t j1 = ((i1 - numRecordedFrameTimes & 511) * 255) / 512;
        int_t k1 = (j1 * j1) / 255;
        k1 = (k1 * k1) / 255;
        int_t i2 = (k1 * k1) / 255;
        i2 = (i2 * i2) / 255;
        if (frameTimes[i1] > l1)
            tessellator->setColorOpaque_I(0xff000000 + k1 * 0x10000);
        else
            tessellator->setColorOpaque_I(0xff000000 + k1 * 256);

        long_t l4 = frameTimes[i1] / 0x30d40L;
        long_t l5 = tickTimes[i1]  / 0x30d40L;
        tessellator->addVertex((float)i1 + 0.5f, (float)(displayHeight - l4) + 0.5f, 0.0);
        tessellator->addVertex((float)i1 + 0.5f, (float)displayHeight        + 0.5f, 0.0);
        tessellator->setColorOpaque_I(0xff000000 + k1 * 0x10000 + k1 * 256 + k1);
        tessellator->addVertex((float)i1 + 0.5f, (float)(displayHeight - l4)       + 0.5f, 0.0);
        tessellator->addVertex((float)i1 + 0.5f, (float)(displayHeight - (l4 - l5)) + 0.5f, 0.0);
    }
    tessellator->draw();
    glEnable(GL_TEXTURE_2D);
}

// ─── shutdown / focus ────────────────────────────────────────────────────────

void Minecraft::shutdown()
{
    running = false;
}

void Minecraft::setIngameFocus()
{
    if (!lwjgl::Display::isActive())
        return;
    if (inGameHasFocus)
        return;
    inGameHasFocus = true;
    mouseHelper->grabMouseCursor();
    displayGuiScreen(nullptr);
    leftClickCounter  = 10000;
    mouseTicksRan     = ticksRan + 10000;
}

void Minecraft::setIngameNotInFocus()
{
    if (!inGameHasFocus)
        return;
    if (thePlayer != nullptr)
        thePlayer->resetPlayerKeyState();
    inGameHasFocus = false;
    mouseHelper->ungrabMouseCursor();
}

void Minecraft::displayInGameMenu()
{
    if (currentScreen != nullptr)
        return;
    displayGuiScreen(new GuiIngameMenu());
}

// ─── displayGuiScreen ────────────────────────────────────────────────────────

void Minecraft::displayGuiScreen(GuiScreen *guiscreen)
{
    if (dynamic_cast<GuiUnused *>(currentScreen) != nullptr)
        return;
    if (currentScreen != nullptr)
        currentScreen->onGuiClosed();
    if (dynamic_cast<GuiMainMenu *>(guiscreen) != nullptr)
        statFileWriter->func_27175_b();
    statFileWriter->syncStats();

    if (guiscreen == nullptr && theWorld == nullptr)
        guiscreen = new GuiMainMenu();
    else if (guiscreen == nullptr && thePlayer != nullptr && thePlayer->health <= 0)
        guiscreen = new GuiGameOver();

    if (dynamic_cast<GuiMainMenu *>(guiscreen) != nullptr)
        ingameGUI->clearChatMessages();

    currentScreen = guiscreen;
    if (guiscreen != nullptr)
    {
        // Track every screen we create. Java relied on GC; in C++ a screen and its
        // buttons leak unless we free them. We can't free on every transition because
        // child screens keep a parentScreen pointer (back-stack), so we defer to
        // purgeOwnedGuiScreens(), called once the whole stack is abandoned.
        // If this exact pointer was scrapped but not yet freed, revive it.
        for (size_t i = 0; i < guiScreensToDelete.size(); i++)
            if (guiScreensToDelete[i] == guiscreen)
            {
                guiScreensToDelete.erase(guiScreensToDelete.begin() + i);
                break;
            }
        bool tracked = false;
        for (GuiScreen *s : ownedGuiScreens)
            if (s == guiscreen) { tracked = true; break; }
        if (!tracked)
            ownedGuiScreens.push_back(guiscreen);

        setIngameNotInFocus();
        ScaledResolution scaledresolution(gameSettings, displayWidth, displayHeight);
        int_t i = scaledresolution.getScaledWidth();
        int_t j = scaledresolution.getScaledHeight();
        guiscreen->setWorldAndResolution(this, i, j);
        skipRenderWorld = false;
    }
    else
    {
        setIngameFocus();
        // Back to gameplay: the menu stack is fully abandoned, so no screen references
        // another anymore -> safe to destroy them all at once.
        purgeOwnedGuiScreens();
    }
}

void Minecraft::purgeOwnedGuiScreens()
{
    // DEFER the actual delete: this is reached from inside a screen's own input
    // handler (a screen closing itself via displayGuiScreen(nullptr)), so deleting
    // synchronously would free a screen whose method is still on the stack
    // (use-after-free, e.g. a GuiTextField touched after the screen dies). The
    // scrap list is flushed at the top of runTick where no screen code is running.
    for (GuiScreen *s : ownedGuiScreens)
        if (s != currentScreen)
            guiScreensToDelete.push_back(s);
    ownedGuiScreens.clear();
    if (currentScreen != nullptr)
        ownedGuiScreens.push_back(currentScreen);
}

// ─── Mouse click helpers ──────────────────────────────────────────────────────

void Minecraft::func_6254_a(int_t i, bool flag)
{
    if (playerController->field_1064_b)
        return;
    if (!flag)
        leftClickCounter = 0;
    if (i == 0 && leftClickCounter > 0)
        return;

    if (flag && objectMouseOver != nullptr &&
        objectMouseOver->typeOfHit == EnumMovingObjectType::TILE && i == 0)
    {
        int_t j = objectMouseOver->blockX;
        int_t k = objectMouseOver->blockY;
        int_t lv = objectMouseOver->blockZ;
        playerController->sendBlockRemoving(j, k, lv, objectMouseOver->sideHit);
        effectRenderer->addBlockHitEffects(j, k, lv, objectMouseOver->sideHit);
    }
    else
    {
        playerController->resetBlockRemoving();
    }
}

void Minecraft::clickMouse(int_t i)
{
    fprintf(stderr, "Beta++: clickMouse(i)\n");
    if (i == 0 && leftClickCounter > 0)
        return;
    if (i == 0)
        thePlayer->swingItem();

    bool flag = true;
    if (objectMouseOver == nullptr)
    {
        if (i == 0 && dynamic_cast<PlayerControllerTest *>(playerController) == nullptr)
            leftClickCounter = 10;
    }
    else if (objectMouseOver->typeOfHit == EnumMovingObjectType::ENTITY)
    {
        if (i == 0)
            playerController->attackEntity(thePlayer, objectMouseOver->entityHit);
        if (i == 1)
            playerController->interactWithEntity(thePlayer, objectMouseOver->entityHit);
    }
    else if (objectMouseOver->typeOfHit == EnumMovingObjectType::TILE)
    {
        int_t j  = objectMouseOver->blockX;
        int_t k  = objectMouseOver->blockY;
        int_t lv = objectMouseOver->blockZ;
        int_t i1 = objectMouseOver->sideHit;
        if (i == 0)
        {
            playerController->clickBlock(j, k, lv, objectMouseOver->sideHit);
        }
        else
        {
            ItemStack *itemstack1 = thePlayer->inventory->getCurrentItem();
            int_t j1 = (itemstack1 == nullptr) ? 0 : itemstack1->stackSize;
            if (playerController->sendPlaceBlock(thePlayer, theWorld, itemstack1, j, k, lv, i1))
            {
                flag = false;
                thePlayer->swingItem();
            }
            if (itemstack1 == nullptr)
                return;
            if (itemstack1->stackSize == 0)
                thePlayer->inventory->mainInventory[thePlayer->inventory->currentItem] = nullptr;
            else if (itemstack1->stackSize != j1)
                entityRenderer->getItemRenderer()->func_9449_b();
        }
    }

    if (flag && i == 1)
    {
        ItemStack *itemstack = thePlayer->inventory->getCurrentItem();
        if (itemstack != nullptr && playerController->sendUseItem(thePlayer, theWorld, itemstack))
            entityRenderer->getItemRenderer()->func_9450_c();
    }
}

void Minecraft::clickMiddleMouseButton()
{
    if (objectMouseOver == nullptr)
        return;
    int_t i = theWorld->getBlockId(objectMouseOver->blockX, objectMouseOver->blockY, objectMouseOver->blockZ);
    if (i == Block::grass->blockID)    i = Block::dirt->blockID;
    if (i == Block::stairDouble->blockID) i = Block::stairSingle->blockID;
    if (i == Block::bedrock->blockID)  i = Block::stone->blockID;
    thePlayer->inventory->setCurrentItem(i, dynamic_cast<PlayerControllerTest *>(playerController) != nullptr);
}

// ─── toggleFullscreen ────────────────────────────────────────────────────────

void Minecraft::toggleFullscreen()
{
    try
    {
        fullscreen = !fullscreen;
        if (fullscreen)
        {
            lwjgl::Display::setFullscreen(true);
            displayWidth  = lwjgl::Display::getDisplayMode().getWidth();
            displayHeight = lwjgl::Display::getDisplayMode().getHeight();
            if (displayWidth  <= 0) displayWidth  = 1;
            if (displayHeight <= 0) displayHeight = 1;
        }
        else
        {
            displayWidth  = tempDisplayWidth;
            displayHeight = tempDisplayHeight;
            if (displayWidth  <= 0) displayWidth  = 1;
            if (displayHeight <= 0) displayHeight = 1;
            lwjgl::Display::setDisplayMode(lwjgl::DisplayMode(displayWidth, displayHeight));
        }
        if (currentScreen != nullptr)
            resize(displayWidth, displayHeight);

        lwjgl::Display::setFullscreen(fullscreen);
        lwjgl::Display::update();
    }
    catch (...) {}
}

void Minecraft::resize(int_t i, int_t j)
{
    if (i <= 0) i = 1;
    if (j <= 0) j = 1;
    displayWidth  = i;
    displayHeight = j;
    if (currentScreen != nullptr)
    {
        ScaledResolution scaledresolution(gameSettings, i, j);
        int_t k = scaledresolution.getScaledWidth();
        int_t lv = scaledresolution.getScaledHeight();
        currentScreen->setWorldAndResolution(this, k, lv);
    }
}

// ─── func_28001_B (check paid) ───────────────────────────────────────────────

void Minecraft::func_28001_B()
{
    (new ThreadCheckHasPaid(this))->start();
}

// ─── runTick ─────────────────────────────────────────────────────────────────

void Minecraft::runTick()
{
    // Safe point to free GUI screens scrapped by purgeOwnedGuiScreens(): no screen
    // handler is on the call stack here.
    if (!guiScreensToDelete.empty())
    {
        for (GuiScreen *s : guiScreensToDelete)
            delete s;
        guiScreensToDelete.clear();
    }

    // Safe point to free worlds abandoned by changeWorld(): no World::tick() is on the
    // call stack here (the MP dimension-change path defers the delete to right here).
    if (!worldsToDelete.empty())
    {
        // The local player is the one object that can cross a World boundary.  Scrub it
        // again immediately before destroying the abandoned worlds: packet handling may
        // have replaced/respawned it after changeWorld() performed its first detach.
        // This also makes the ownership rule explicit for every path that queues a World.
        if (thePlayer != nullptr && theWorld != nullptr)
        {
            for (World *w : worldsToDelete)
                if (w != nullptr && w != theWorld)
                    w->detachEntityForWorldChange(thePlayer);

            thePlayer->setWorld(theWorld);
            renderViewEntity = thePlayer;
        }

        for (World *w : worldsToDelete)
            delete w;
        worldsToDelete.clear();
    }

    if (ticksRan == 6000)
        func_28001_B();

    {
        ::Profiling::Auto _p(::Profiling::S_TICK_STAT);
        statFileWriter->func_27178_d();
    }
    {
        ::Profiling::Auto _p(::Profiling::S_TICK_INGAME);
        ingameGUI->updateTick();
    }
    {
        ::Profiling::Auto _p(::Profiling::S_TICK_MOUSEOVER);
        entityRenderer->getMouseOver(1.0f);
    }

    if (thePlayer != nullptr)
    {
        IChunkProvider *ichunkprovider = theWorld->getIChunkProvider();
        ChunkProviderLoadOrGenerate *cplg =
            dynamic_cast<ChunkProviderLoadOrGenerate *>(ichunkprovider);
        if (cplg != nullptr)
        {
            int_t jv = MathHelper::floor_float((float)thePlayer->posX) >> 4;
            int_t i1 = MathHelper::floor_float((float)thePlayer->posZ) >> 4;
            cplg->setCurrentChunkOver(jv, i1);
        }
    }

    if (!isGamePaused && theWorld != nullptr)
    {
        ::Profiling::Auto _p(::Profiling::S_TICK_CONTROLLER);
        playerController->updateController();
    }

    glBindTexture(GL_TEXTURE_2D, renderEngine->getTexture("/terrain.png"));

    if (!isGamePaused)
    {
        ::Profiling::Auto _p(::Profiling::S_TICK_TEXFX);
        renderEngine->updateDynamicTextures();
    }

    if (currentScreen == nullptr && thePlayer != nullptr)
    {
        if (thePlayer->health <= 0)
            displayGuiScreen(nullptr);
        else if (thePlayer->isPlayerSleeping() && theWorld != nullptr && theWorld->multiplayerWorld)
            displayGuiScreen(new GuiSleepMP());
    }
    else if (currentScreen != nullptr &&
             dynamic_cast<GuiSleepMP *>(currentScreen) != nullptr &&
             !thePlayer->isPlayerSleeping())
    {
        displayGuiScreen(nullptr);
    }

    if (currentScreen != nullptr)
    {
        leftClickCounter = 10000;
        mouseTicksRan    = ticksRan + 10000;
    }

    if (currentScreen != nullptr)
    {
        leftClickCounter = 10000;
        mouseTicksRan    = ticksRan + 10000;
    }
#ifdef MC_IOS
    // iOS keyboard alert: replay an exact replacement (BACKSPACEs for the
    // snapshot length, then the new characters) into the key queue so the
    // focused widget (chat line, username field, ...) receives it naturally.
    {
        std::string inject;
        int backs = 0;
        bool withReturn = false;
        if (::iosapp::consumePendingTextInject(inject, backs, withReturn))
        {
            GuiTextField *focusedField =
                (currentScreen != nullptr) ? currentScreen->getFocusedTextField() : nullptr;
            if (focusedField != nullptr)
            {
                // The native field is the source of truth: replace the whole
                // field (updates live while typing) instead of replaying keys,
                // which could only ever append.
                focusedField->setText(inject);
                fprintf(stderr, "Beta++: INJECT set field -> '%s'\n", inject.c_str());
            }
            else
            {
                for (int b = 0; b < backs; ++b)
                {
                    ::lwjgl::Keyboard::detail::pushLwjglKey(0x0E, true);
                    ::lwjgl::Keyboard::detail::pushLwjglKey(0x0E, false);
                }
                for (unsigned char ch : inject)
                    ::lwjgl::Keyboard::detail::pushUnicodeChar(static_cast<char32_t>(ch));
                if (withReturn)
                {
                    ::lwjgl::Keyboard::detail::pushLwjglKey(0x1C, true);
                    ::lwjgl::Keyboard::detail::pushLwjglKey(0x1C, false);
                }
            }
        }
    }
#endif // MC_IOS

    if (currentScreen != nullptr)
    {
        currentScreen->handleInput();
        if (currentScreen != nullptr)
        {
            if (currentScreen->guiParticles != nullptr)
                currentScreen->guiParticles->updateParticles();
            currentScreen->updateScreen();
        }
    }

    if (currentScreen == nullptr || currentScreen->field_948_f)
    {
        while (lwjgl::Mouse::next())
        {
            long_t lv = System::currentTimeMillis() - systemTime;
            if (lv <= 200L)
            {
                int_t k = lwjgl::Mouse::getEventDWheel();
                if (k != 0)
                {
                    thePlayer->inventory->changeCurrentItem(k);
                    if (gameSettings->field_22275_C)
                    {
                        if (k > 0) k = 1;
                        if (k < 0) k = -1;
                        gameSettings->field_22272_F += (float)k * 0.25f;
                    }
                }

                if (currentScreen == nullptr)
                {
                    if (!inGameHasFocus && lwjgl::Mouse::getEventButtonState())
                    {
                        setIngameFocus();
                    }
                    else
                    {
                        if (lwjgl::Mouse::getEventButton() == 0 && lwjgl::Mouse::getEventButtonState())
                        {
                            clickMouse(0);
                            mouseTicksRan = ticksRan;
                        }
                        if (lwjgl::Mouse::getEventButton() == 1 && lwjgl::Mouse::getEventButtonState())
                        {
                            clickMouse(1);
                            mouseTicksRan = ticksRan;
                        }
                        if (lwjgl::Mouse::getEventButton() == 2 && lwjgl::Mouse::getEventButtonState())
                        {
                            clickMiddleMouseButton();
                        }
                    }
                }
                else if (currentScreen != nullptr)
                {
                    currentScreen->handleMouseInput();
                }
            }
        }

        if (leftClickCounter > 0)
            leftClickCounter--;

        // F3 alone toggles the debug screen on RELEASE; F3+G toggles the chunk
        // border overlay instead and suppresses the debug screen for that hold.
        static bool s_f3Down = false;
        static bool s_f3Combo = false;
        while (lwjgl::Keyboard::next())
        {
            thePlayer->handleKeyPress(lwjgl::Keyboard::getEventKey(),
                                      lwjgl::Keyboard::getEventKeyState());
            if (!lwjgl::Keyboard::getEventKeyState() &&
                lwjgl::Keyboard::getEventKey() == 0x3D)
            {
                if (!s_f3Combo)
                    gameSettings->showDebugInfo = !gameSettings->showDebugInfo;
                s_f3Down = false;
            }
            if (lwjgl::Keyboard::getEventKeyState())
            {
                // F11 = 0x57
                if (lwjgl::Keyboard::getEventKey() == 0x57)
                {
                    toggleFullscreen();
                }
                else
                {
                    if (currentScreen != nullptr)
                    {
                        currentScreen->handleKeyboardInput();
                    }
                    else
                    {
                        // ESC = 0x01
                        if (lwjgl::Keyboard::getEventKey() == 0x01)
                            displayInGameMenu();
                        // S + F3 (0x1F + 0x3D)
                        if (lwjgl::Keyboard::getEventKey() == 0x1F && lwjgl::Keyboard::isKeyDown(0x3D))
                            forceReload();
                        // F1 = 0x3B
                        if (lwjgl::Keyboard::getEventKey() == 0x3B)
                            gameSettings->hideGUI = !gameSettings->hideGUI;
                        // F3 = 0x3D (deferred to release so F3+G can combo)
                        if (lwjgl::Keyboard::getEventKey() == 0x3D)
                        {
                            s_f3Down = true;
                            s_f3Combo = false;
                        }
                        // G = 0x22 with F3 held -> chunk borders
                        if (lwjgl::Keyboard::getEventKey() == 0x22 && lwjgl::Keyboard::isKeyDown(0x3D))
                        {
                            s_f3Combo = true;
                            gameSettings->ofShowChunkBorders = !gameSettings->ofShowChunkBorders;
                        }
                        // F5 = 0x3F
                        if (lwjgl::Keyboard::getEventKey() == 0x3F)
                            gameSettings->thirdPersonView = !gameSettings->thirdPersonView;
                        // B = 0x30
                        if (lwjgl::Keyboard::getEventKey() == 0x30)
                            gameSettings->smoothCamera = !gameSettings->smoothCamera;

                        if (lwjgl::Keyboard::getEventKey() == gameSettings->keyBindInventory->keyCode)
                            displayGuiScreen(new GuiInventory(thePlayer));
                        if (lwjgl::Keyboard::getEventKey() == gameSettings->keyBindDrop->keyCode)
                            thePlayer->dropCurrentItem();
                        if (isMultiplayerWorld() &&
                            lwjgl::Keyboard::getEventKey() == gameSettings->keyBindChat->keyCode)
                            displayGuiScreen(new GuiChat());
                    }

                    for (int_t ii = 0; ii < 9; ii++)
                    {
                        if (lwjgl::Keyboard::getEventKey() == 0x02 + ii)
                            thePlayer->inventory->currentItem = ii;
                    }

                    if (lwjgl::Keyboard::getEventKey() == gameSettings->keyBindToggleFog->keyCode)
                    {
                        bool shift = lwjgl::Keyboard::isKeyDown(0x2A) || lwjgl::Keyboard::isKeyDown(0x36);
                        gameSettings->setOptionValue(EnumOptions::RENDER_DISTANCE, shift ? -1 : 1);
                    }
                }
            }
        }

        if (currentScreen == nullptr)
        {
            if (lwjgl::Mouse::isButtonDown(0) &&
                (float)(ticksRan - mouseTicksRan) >= timer->ticksPerSecond / 4.0f &&
                inGameHasFocus)
            {
                clickMouse(0);
                mouseTicksRan = ticksRan;
            }
            if (lwjgl::Mouse::isButtonDown(1) &&
                (float)(ticksRan - mouseTicksRan) >= timer->ticksPerSecond / 4.0f &&
                inGameHasFocus)
            {
                clickMouse(1);
                mouseTicksRan = ticksRan;
            }
        }

        func_6254_a(0, currentScreen == nullptr && lwjgl::Mouse::isButtonDown(0) && inGameHasFocus);
    }

    if (theWorld != nullptr)
    {
        if (thePlayer != nullptr)
        {
            joinPlayerCounter++;
            if (joinPlayerCounter == 30)
            {
                joinPlayerCounter = 0;
                theWorld->joinEntityInSurroundings(thePlayer);
            }
        }
        theWorld->difficultySetting = gameSettings->difficulty;
        if (theWorld->multiplayerWorld)
            theWorld->difficultySetting = 3;

        if (!isGamePaused)
            entityRenderer->updateRenderer();
        if (!isGamePaused)
            renderGlobal->updateClouds();
        if (!isGamePaused)
        {
            if (theWorld->field_27172_i > 0)
                theWorld->field_27172_i--;
            {
                ::Profiling::Auto _p(::Profiling::S_TICK_ENTITIES);
                theWorld->updateEntities();
            }
        }
        if (!isGamePaused || isMultiplayerWorld())
        {
            theWorld->setAllowedMobSpawns(gameSettings->difficulty > 0, true);
            {
                ::Profiling::Auto _p(::Profiling::S_TICK_WORLD);
                theWorld->tick();
            }
        }
        if (!isGamePaused && theWorld != nullptr)
        {
            ::Profiling::Auto _p(::Profiling::S_TICK_RANDOMDISP);
            theWorld->randomDisplayUpdates(
                MathHelper::floor_double(thePlayer->posX),
                MathHelper::floor_double(thePlayer->posY),
                MathHelper::floor_double(thePlayer->posZ));
        }
        if (!isGamePaused)
        {
            ::Profiling::Auto _p(::Profiling::S_TICK_EFFECTS);
            effectRenderer->updateEffects();
        }
    }

    systemTime = System::currentTimeMillis();
}

// ─── forceReload ─────────────────────────────────────────────────────────────

void Minecraft::forceReload()
{
    std::cout << "FORCING RELOAD!" << std::endl;
    sndManager = new SoundManager();
    sndManager->loadSoundSettings(gameSettings);
    downloadResourcesThread->reloadResources();
}

// ─── isMultiplayerWorld ──────────────────────────────────────────────────────

bool Minecraft::isMultiplayerWorld()
{
    return theWorld != nullptr && theWorld->multiplayerWorld;
}

// ─── World loading ───────────────────────────────────────────────────────────

void Minecraft::startWorld(const std::string &s, const std::string &s1, long_t l)
{
    fprintf(stderr, "Beta++: startWorld folder='%s' name='%s' seed=%ld\n",
            s.c_str(), s1.c_str(), l);
    changeWorld1(nullptr);
    fprintf(stderr, "Beta++: sw[1] old world cleared\n");
    if (saveLoader->isOldMapFormat(s))
    {
        fprintf(stderr, "Beta++: sw[2] old format -> convert\n");
        convertMapFormat(s, s1);
    }
    else
    {
        fprintf(stderr, "Beta++: sw[3] getting save loader\n");
        ISaveHandler *isavehandler = saveLoader->getSaveLoader(s, false);
        fprintf(stderr, "Beta++: sw[4] constructing World\n");
        World *world = new World(isavehandler, String::fromUTF8(s1), l);
        fprintf(stderr, "Beta++: sw[5] World constructed\n");
        if (world->isNewWorld)
        {
            statFileWriter->readStat(StatList::createWorldStat, 1);
            statFileWriter->readStat(StatList::startGameStat, 1);
            changeWorld2(world, "Generating level");
        }
        else
        {
            statFileWriter->readStat(StatList::loadWorldStat, 1);
            statFileWriter->readStat(StatList::startGameStat, 1);
            changeWorld2(world, "Loading level");
        }
    }
}

void Minecraft::usePortal()
{
    std::cout << "Toggling dimension!!" << std::endl;
    if (thePlayer->dimension == -1)
        thePlayer->dimension = 0;
    else
        thePlayer->dimension = -1;

    theWorld->setEntityDead(thePlayer);
    thePlayer->isDead = false;

    double d  = thePlayer->posX;
    double d1 = thePlayer->posZ;
    double d2 = 8.0;

    if (thePlayer->dimension == -1)
    {
        d  /= d2;
        d1 /= d2;
        thePlayer->setLocationAndAngles(d, thePlayer->posY, d1, thePlayer->rotationYaw, thePlayer->rotationPitch);
        if (thePlayer->isEntityAlive())
            theWorld->updateEntityWithOptionalForce(thePlayer, false);
        World *world = new World(theWorld, WorldProvider::getProviderForDimension(-1));
        changeWorld(world, "Entering the Nether", thePlayer);
    }
    else
    {
        d  *= d2;
        d1 *= d2;
        thePlayer->setLocationAndAngles(d, thePlayer->posY, d1, thePlayer->rotationYaw, thePlayer->rotationPitch);
        if (thePlayer->isEntityAlive())
            theWorld->updateEntityWithOptionalForce(thePlayer, false);
        World *world1 = new World(theWorld, WorldProvider::getProviderForDimension(0));
        changeWorld(world1, "Leaving the Nether", thePlayer);
    }

    thePlayer->worldObj = theWorld;
    if (thePlayer->isEntityAlive())
    {
        thePlayer->setLocationAndAngles(d, thePlayer->posY, d1, thePlayer->rotationYaw, thePlayer->rotationPitch);
        theWorld->updateEntityWithOptionalForce(thePlayer, false);
        Teleporter().func_4107_a(theWorld, thePlayer);
    }
}

void Minecraft::changeWorld1(World *world)
{
    changeWorld2(world, "");
}

void Minecraft::changeWorld2(World *world, const std::string &s)
{
    changeWorld(world, s, nullptr);
}

static void *g_cwStack[64];
static std::atomic<int> g_cwAlive{0};

static void cwWatchdog()
{
    for (int i = 0; i < 6; ++i)
    {
        struct timespec ts = {2, 0};
        nanosleep(&ts, nullptr);
        if (!g_cwAlive)
            return;
    }
    void *bt[32];
    int n = backtrace(bt, 32);
    fprintf(stderr, "Beta++: WATCHDOG changeWorld still active, %d frames:\n", n);
    backtrace_symbols_fd(bt, n, STDERR_FILENO);
}

void Minecraft::changeWorld(World *world, const std::string &s, EntityPlayerSP *entityplayer)
{
    g_cwAlive = 1;
    std::thread([] { cwWatchdog(); }).detach();
    fprintf(stderr, "Beta++: changeWorld enter\n");
    World *oldWorld = theWorld;
    EntityPlayerSP *transferredPlayer = entityplayer;
    if (transferredPlayer == nullptr && world != nullptr && world->multiplayerWorld)
        transferredPlayer = thePlayer;
    if (oldWorld != nullptr && transferredPlayer != nullptr)
        oldWorld->detachEntityForWorldChange(transferredPlayer);

    fprintf(stderr, "Beta++: cw[1] stats\n");
    statFileWriter->func_27175_b();
    statFileWriter->syncStats();
    fprintf(stderr, "Beta++: cw[2] loadscreen\n");
    renderViewEntity = nullptr;
    loadingScreen->printText(s);
    loadingScreen->displayLoadingString("");
    fprintf(stderr, "Beta++: cw[3] sound\n");
    sndManager->playStreaming(u"", 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    fprintf(stderr, "Beta++: cw[4] player lookup\n");
    fprintf(stderr, "Beta++: x1 oldWorld=%p world=%p\n", (void*)oldWorld, (void*)world);

    if (oldWorld != nullptr)
    {
        fprintf(stderr, "Beta++: x2 saving old world\n");
        oldWorld->saveWorldIndirectly(loadingScreen);
        fprintf(stderr, "Beta++: x3 old world saved\n");
    }

    theWorld = world;
    fprintf(stderr, "Beta++: x4 theWorld set\n");

    // Un jugador TRANSFERIDO (portal SP, o cambio de dimension MP) conserva el mismo
    // objeto EntityPlayer, pero su worldObj seguia apuntando al mundo viejo. Como ese
    // mundo se libera (diferido) al terminar el cambio, dejaba worldObj colgando y el
    // siguiente getMouseOver()->rayTrace()->worldObj->rayTraceBlocks() crasheaba con
    // this=0xFFFF... Reasignarlo al nuevo mundo aqui. (En la carga normal SP el jugador
    // se obtiene del propio mundo via func_4085_a, asi que ya trae el worldObj correcto.)
    if (transferredPlayer != nullptr && world != nullptr)
        transferredPlayer->setWorld(world);

    fprintf(stderr, "Beta++: x5 pre-worldblock\n");
    if (world != nullptr)
    {
        fprintf(stderr, "Beta++: x6 controller\n");
        playerController->func_717_a(world);
        fprintf(stderr, "Beta++: x7 mp-check\n");
        bool isMp = isMultiplayerWorld();
        fprintf(stderr, "Beta++: x8 isMp=%d\n", isMp);
        if (!isMp)
        {
            if (entityplayer == nullptr)
            {
                fprintf(stderr, "Beta++: x9 lookup4085\n");
                thePlayer = (EntityPlayerSP *)world->func_4085_a(typeid(EntityPlayerSP));
                fprintf(stderr, "Beta++: xa lookup done (%p)\n", (void*)thePlayer);
            }
        }
        else if (thePlayer != nullptr)
        {
            thePlayer->preparePlayerToSpawn();
            if (world != nullptr)
                world->entityJoinedWorld(thePlayer);
        }

        fprintf(stderr, "Beta++: cw[5] terrain prep\n");
        if (!world->multiplayerWorld)
        {
            fprintf(stderr, "Beta++: func_6255_d Building terrain begin\n");
            func_6255_d(s);
            fprintf(stderr, "Beta++: func_6255_d done\n");
        }
        if (thePlayer == nullptr)
        {
            thePlayer = (EntityPlayerSP *)playerController->createPlayer(world);
            thePlayer->preparePlayerToSpawn();
            playerController->flipPlayer(thePlayer);
        }

        delete thePlayer->movementInput;
        thePlayer->movementInput = new MovementInputFromOptions(gameSettings);

        playerController->func_6473_b(thePlayer);

        if (entityplayer != nullptr)
            world->emptyMethod1();

        IChunkProvider *ichunkprovider = world->getIChunkProvider();
        ChunkProviderLoadOrGenerate *cplg =
            dynamic_cast<ChunkProviderLoadOrGenerate *>(ichunkprovider);
        if (cplg != nullptr)
        {
            int_t ii = MathHelper::floor_float((float)thePlayer->posX) >> 4;
            int_t jj = MathHelper::floor_float((float)thePlayer->posZ) >> 4;
            cplg->setCurrentChunkOver(ii, jj);
        }
        world->spawnPlayerWithLoadedChunks(thePlayer);
        if (world->isNewWorld)
            world->saveWorldIndirectly(loadingScreen);

        renderViewEntity = thePlayer;
    }
    else
    {
        thePlayer = nullptr;
    }

    if (renderGlobal != nullptr)
        renderGlobal->changeWorld(world);
    if (effectRenderer != nullptr)
        effectRenderer->clearEffects(world);

    // El borrado del mundo viejo se DIFIERE al inicio del proximo runTick. Motivo:
    // en multiplayer, changeWorld puede ejecutarse en medio del tick del mundo viejo
    // (WorldClient::tick() -> processReadPackets() -> handleRespawn() -> changeWorld()
    // al cambiar de dimension). Borrarlo aqui de forma sincrona liberaria el World cuyo
    // tick() sigue en la pila -> use-after-free al volver (this->pendingBlockChanges, etc.).
    // Mismo patron que guiScreensToDelete.
    if (oldWorld != nullptr && oldWorld != world)
        worldsToDelete.push_back(oldWorld);
    if (world == nullptr)
    {
        AxisAlignedBB::getStatMap();
        Vec3D::getCraftingManager();
    }

    systemTime = 0L;
}

// ─── respawn ─────────────────────────────────────────────────────────────────

void Minecraft::respawn(bool flag, int_t i)
{
    if (!theWorld->multiplayerWorld && !theWorld->worldProvider->canRespawnHere())
        usePortal();

    ChunkCoordinates *chunkcoordinates  = nullptr;
    ChunkCoordinates *chunkcoordinates1 = nullptr;
    bool flag1 = true;

    if (thePlayer != nullptr && !flag)
    {
        chunkcoordinates = thePlayer->getPlayerSpawnCoordinate();
        if (chunkcoordinates != nullptr)
        {
            chunkcoordinates1 = EntityPlayer::func_25060_a(theWorld, chunkcoordinates);
            if (chunkcoordinates1 == nullptr)
                thePlayer->addChatMessage("tile.bed.notValid");
        }
    }

    if (chunkcoordinates1 == nullptr)
    {
        chunkcoordinates1 = theWorld->getSpawnPoint();
        flag1 = false;
    }

    IChunkProvider *ichunkprovider = theWorld->getIChunkProvider();
    ChunkProviderLoadOrGenerate *cplg =
        dynamic_cast<ChunkProviderLoadOrGenerate *>(ichunkprovider);
    if (cplg != nullptr)
        cplg->setCurrentChunkOver(chunkcoordinates1->x >> 4, chunkcoordinates1->z >> 4);

    theWorld->setSpawnLocation();
    theWorld->updateEntityList();
    int_t j = 0;
    if (thePlayer != nullptr)
    {
        j = thePlayer->entityId;
        theWorld->setEntityDead(thePlayer);
    }

    renderViewEntity = nullptr;
    thePlayer = (EntityPlayerSP *)playerController->createPlayer(theWorld);
    thePlayer->dimension = i;
    renderViewEntity     = thePlayer;
    thePlayer->preparePlayerToSpawn();

    if (flag1)
    {
        thePlayer->setPlayerSpawnCoordinate(chunkcoordinates);
        thePlayer->setLocationAndAngles(
            (float)chunkcoordinates1->x + 0.5f,
            (float)chunkcoordinates1->y + 0.1f,
            (float)chunkcoordinates1->z + 0.5f,
            0.0f, 0.0f);
    }

    playerController->flipPlayer(thePlayer);
    theWorld->spawnPlayerWithLoadedChunks(thePlayer);
    thePlayer->movementInput = new MovementInputFromOptions(gameSettings);
    thePlayer->entityId      = j;
    thePlayer->func_6420_o();
    playerController->func_6473_b(thePlayer);
    func_6255_d("Respawning");

    if (dynamic_cast<GuiGameOver *>(currentScreen) != nullptr)
        displayGuiScreen(nullptr);
}

// ─── convertMapFormat / func_6255_d ──────────────────────────────────────────

void Minecraft::convertMapFormat(const std::string &s, const std::string &s1)
{
    loadingScreen->printText("Converting World to " + saveLoader->func_22178_a());
    loadingScreen->displayLoadingString("This may take a while :)");
    saveLoader->convertMapFormat(s, loadingScreen);
    startWorld(s, s1, 0L);
}

void Minecraft::func_6255_d(const std::string &s)
{
    fprintf(stderr, "Beta++: fw begin\n");
    loadingScreen->printText(s);
    loadingScreen->displayLoadingString("Building terrain");

#if defined(__ARM_ARCH)
    // Phone build: vanilla prepares a +/-128-block spawn ring synchronously
    // (289 full chunks before the first frame). That is minutes of blocking
    // work plus tens of MB of chunks -> memory warning + blank dirt screen.
    // A +/-48 ring is plenty for spawn; the rest streams in via the
    // background loader.
    int_t c  = 48;
#else
    int_t c  = 128;
#endif
    int_t i  = 0;
    int_t j  = (c * 2) / 16 + 1;
    j *= j;

    IChunkProvider    *ichunkprovider   = theWorld->getIChunkProvider();
    ChunkCoordinates  *chunkcoordinates = theWorld->getSpawnPoint();
    if (thePlayer != nullptr)
    {
        chunkcoordinates->x = (int_t)thePlayer->posX;
        chunkcoordinates->z = (int_t)thePlayer->posZ;
    }

    ChunkProviderLoadOrGenerate *cplg =
        dynamic_cast<ChunkProviderLoadOrGenerate *>(ichunkprovider);
    if (cplg != nullptr)
        cplg->setCurrentChunkOver(chunkcoordinates->x >> 4, chunkcoordinates->z >> 4);

    for (int_t k = -c; k <= c; k += 16)
    {
        for (int_t l = -c; l <= c; l += 16)
        {
            loadingScreen->setLoadingProgress((i++ * 100) / j);
            if ((i % 10) == 0)
                fprintf(stderr, "Beta++: terrain prep %d/%d\n", i, j);
            long_t t0 = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
            theWorld->getBlockId(chunkcoordinates->x + k, 64, chunkcoordinates->z + l);
            while (theWorld->updatingLighting()) {}
            long_t dt = (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() - t0) / 1000000LL;
            if (dt > 2000)
                fprintf(stderr, "Beta++: slow chunk (%d,%d): %ld ms\n",
                        k >> 4, l >> 4, dt);
        }
    }

    loadingScreen->displayLoadingString("Simulating world for a bit");
    fprintf(stderr, "Beta++: simulate begin\n");
    theWorld->func_656_j();
    fprintf(stderr, "Beta++: simulate done\n");
    g_cwAlive = 0;
}

// ─── installResource ─────────────────────────────────────────────────────────

void Minecraft::installResource(const std::string &s, const std::string &file)
{
    size_t idx = s.find('/');
    if (idx == std::string::npos) return;
    std::string s1 = s.substr(0, idx);
    std::string s2 = s.substr(idx + 1);

    if (s1 == "sound" || s1 == "newsound")
        sndManager->addSound(String::fromUTF8(s2), file);
    else if (s1 == "streaming")
        sndManager->addStreaming(String::fromUTF8(s2), file);
    else if (s1 == "music" || s1 == "newmusic")
        sndManager->addMusic(String::fromUTF8(s2), file);
}

// ─── Accessors ────────────────────────────────────────────────────────────────

ISaveFormat *Minecraft::getSaveLoader()
{
    return saveLoader;
}

OpenGlCapsChecker *Minecraft::getOpenGlCapsChecker()
{
    return glCapabilities;
}

std::string Minecraft::getDebugLine1() // func_6241_m
{
    return renderGlobal->getDebugInfoRenders();
}

std::string Minecraft::getDebugLine2() // func_6262_n
{
    return renderGlobal->getDebugInfoEntities();
}

std::string Minecraft::getDebugLine3() // func_21002_o
{
    return theWorld->func_21119_g();
}

std::string Minecraft::getDebugLine4() // func_6245_o
{
    return "P: " + effectRenderer->getStatistics() + ". T: " + theWorld->func_687_d();
}

// ─── Static helpers ──────────────────────────────────────────────────────────

bool Minecraft::isGuiEnabled()
{
    return theMinecraft == nullptr || !theMinecraft->gameSettings->hideGUI;
}

bool Minecraft::isFancyGraphicsEnabled()
{
    return theMinecraft != nullptr && theMinecraft->gameSettings->fancyGraphics;
}

bool Minecraft::isAmbientOcclusionEnabled()
{
    return theMinecraft != nullptr && theMinecraft->gameSettings->ambientOcclusion;
}

bool Minecraft::isDebugInfoEnabled()
{
    return theMinecraft != nullptr && theMinecraft->gameSettings->showDebugInfo;
}

bool Minecraft::lineIsCommand(const std::string &s)
{
    (void)s;
    return false;
}

NetClientHandler *Minecraft::getSendQueue()
{
    EntityClientPlayerMP *mp = dynamic_cast<EntityClientPlayerMP *>(thePlayer);
    if (mp != nullptr)
        return mp->sendQueue;
    return nullptr;
}
