#pragma once

#include <string>
#include <vector>

#include "java/Type.h"
#include "java/String.h"
#include "java/System.h"

struct SDL_Window;

class File;

class PlayerController;
class World;
class RenderGlobal;
class EntityPlayerSP;
class EntityLiving;
class EffectRenderer;
class RenderEngine;
class FontRenderer;
class GuiScreen;
class LoadingScreenRenderer;
class EntityRenderer;
class GuiAchievement;
class GuiIngame;
class ModelBiped;
class MovingObjectPosition;
class GameSettings;
class SoundManager;
class MouseHelper;
class TexturePackList;
class ISaveFormat;
class StatFileWriter;
class TextureWaterFX;
class TextureLavaFX;
class OpenGlCapsChecker;
class ThreadDownloadResources;
class NetClientHandler;
class Session;
class Timer;
class UnexpectedThrowable;

// net.minecraft.client.Minecraft + net.minecraft.src.MinecraftImpl (merged)
class Minecraft
{
public:
    Minecraft(int_t width, int_t height, bool flag);
    ~Minecraft();

    static void start(const jstring *username, const jstring *sessionId);
    static Minecraft *getMinecraft();

    void run();
    void startGame();
    void runTick();
    void shutdown();

    void displayGuiScreen(GuiScreen *guiscreen);
    void purgeOwnedGuiScreens(); // free abandoned menu screens the Java GC would have collected
    void displayInGameMenu();
    void setIngameFocus();
    void setIngameNotInFocus();
    void toggleFullscreen();

    void onMinecraftCrash(UnexpectedThrowable *unexpectedthrowable);
    virtual void displayUnexpectedThrowable(UnexpectedThrowable *unexpectedthrowable);

    void setServer(const std::string &s, int_t i);

    ISaveFormat *getSaveLoader();
    OpenGlCapsChecker *getOpenGlCapsChecker();

    bool isMultiplayerWorld();

    void startWorld(const std::string &s, const std::string &s1, long_t l);
    void usePortal();
    void changeWorld1(World *world);
    void changeWorld2(World *world, const std::string &s);
    void changeWorld(World *world, const std::string &s, EntityPlayerSP *entityplayer);
    void respawn(bool flag, int_t i);

    void installResource(const std::string &s, const std::string &file);

    std::string getDebugLine1(); // func_6241_m
    std::string getDebugLine2(); // func_6262_n
    std::string getDebugLine3(); // func_21002_o
    std::string getDebugLine4(); // func_6245_o

    static bool isGuiEnabled();
    static bool isFancyGraphicsEnabled();
    static bool isAmbientOcclusionEnabled();
    static bool isDebugInfoEnabled();

    bool lineIsCommand(const std::string &s);

    static File *getMinecraftDir();

    NetClientHandler *getSendQueue();

    // Static fields
    static byte_t field_28006_b[0xa00000];
    static long_t frameTimes[512];
    static long_t tickTimes[512];
    static int_t numRecordedFrameTimes;
    static long_t hasPaidCheckTime;

    // Public fields
    PlayerController *playerController;
    int_t displayWidth;
    int_t displayHeight;
    World *theWorld;
    RenderGlobal *renderGlobal;
    EntityPlayerSP *thePlayer;
    EntityLiving *renderViewEntity;
    EffectRenderer *effectRenderer;
    Session *session;
    std::string minecraftUri;
    bool hideQuitButton;
    volatile bool isGamePaused;
    RenderEngine *renderEngine;
    FontRenderer *fontRenderer;
    GuiScreen *currentScreen;
    std::vector<GuiScreen *> ownedGuiScreens;    // every screen we showed; purged when the stack is abandoned
    std::vector<GuiScreen *> guiScreensToDelete; // deferred frees; flushed at runTick start (never delete a screen mid-handler)
    std::vector<World *> worldsToDelete;         // deferred world frees; flushed at runTick start (changeWorld can run mid-tick via MP packet handling -> never delete the world being ticked)
    LoadingScreenRenderer *loadingScreen;
    EntityRenderer *entityRenderer;
    int_t ticksRan;
    GuiAchievement *guiAchievement;
    GuiIngame *ingameGUI;
    bool skipRenderWorld;
    ModelBiped *field_9242_w;
    MovingObjectPosition *objectMouseOver;
    GameSettings *gameSettings;
    SoundManager *sndManager;
    MouseHelper *mouseHelper;
    TexturePackList *texturePackList;
    StatFileWriter *statFileWriter;
    volatile bool running;
    std::string debug;
    bool inGameHasFocus;
    bool isRaining;
    SDL_Window *window;

private:
    void loadScreen();
    void func_6274_a(int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1);
    void checkGLError(const std::string &s);
    void shutdownMinecraftApplet();
    void screenshotListener();
    void displayDebugInfo(long_t l);
    void func_6254_a(int_t i, bool flag);
    void clickMouse(int_t i);
    void clickMiddleMouseButton();
    void func_28001_B();
    void func_28002_e();
    void forceReload();
    void convertMapFormat(const std::string &s, const std::string &s1);
    void func_6255_d(const std::string &s);
    void resize(int_t i, int_t j);

    static File *getAppDir(const std::string &s);
    static Minecraft *theMinecraft;
    static File *minecraftDir;

    bool fullscreen;
    bool hasCrashed;
    OpenGlCapsChecker *glCapabilities;
    Timer *timer;
    ThreadDownloadResources *downloadResourcesThread;
    int_t leftClickCounter;
    int_t tempDisplayWidth;
    int_t tempDisplayHeight;
    File *mcDataDir;
    ISaveFormat *saveLoader;
    std::string serverName;
    int_t serverPort;
    TextureWaterFX *textureWaterFX;
    TextureLavaFX *textureLavaFX;
    bool isTakingScreenshot;
    long_t prevFrameTime;
    int_t mouseTicksRan;
    long_t systemTime;
    int_t joinPlayerCounter;
};
