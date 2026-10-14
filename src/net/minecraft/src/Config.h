#pragma once

#include "java/Type.h"
#include <string>

class GameSettings;
class Minecraft;
class IBlockAccess;

// net.minecraft.src.Config (OptiFine)
// Port sin reflexion y sin TextureHD/mipmaps. Ver OPTIFINE_PORT.md.
class Config
{
public:
	// Defaults (Config.DEF_*)
	static constexpr bool DEF_FOG_FANCY = true;
	static constexpr float DEF_FOG_START = 0.2f;
	static constexpr bool DEF_OPTIMIZE_RENDER_DISTANCE = false;
	static constexpr bool DEF_OCCLUSION_ENABLED = false;
	static constexpr float DEF_ALPHA_FUNC_LEVEL = 0.1f;
	static constexpr bool DEF_LOAD_CHUNKS_FAR = false;
	static constexpr int_t DEF_PRELOADED_CHUNKS = 0;
	static constexpr int_t DEF_CHUNKS_LIMIT = 25;
	static constexpr int_t DEF_UPDATES_PER_FRAME = 3;
	static constexpr bool DEF_DYNAMIC_UPDATES = false;

	static void setGameSettings(GameSettings *options);
	static void setMinecraft(Minecraft *mc);
	static Minecraft *getMinecraft();

	// Capacidades OpenGL (via SDL_GL_ExtensionSupported)
	static bool isFancyFogAvailable();
	static bool isOcclusionAvailable();

	// Niebla
	static bool isFogFancy();
	static bool isFogOff();
	static float getFogStart();

	// Alpha / smooth
	static bool isUseAlphaFunc();
	static float getAlphaFuncLevel();

	// Occlusion culling
	static bool isOcclusionEnabled();
	static bool isOcclusionFancy();

	// Carga de chunks
	static bool isLoadChunksFar();
	static int_t getPreloadedChunks();
	static int_t getUpdatesPerFrame();
	static bool isDynamicUpdates();
	static bool isBackgroundChunkLoading();
	static bool isFarView();

	// Detalle
	static bool isRainFancy();
	static bool isRainOff();
	static bool isWaterFancy();
	static bool isCloudsFancy();
	static bool isCloudsOff();
	static bool isTreesFancy();
	static bool isGrassFancy();
	static bool isBetterGrass();
	static bool isBetterGrassFancy();
	static int_t getSideGrassTexture(IBlockAccess *blockAccess, int_t x, int_t y, int_t z, int_t side);
	static int_t getSideSnowGrassTexture(IBlockAccess *blockAccess, int_t x, int_t y, int_t z, int_t side);

	// Animaciones (toggles)
	static bool isAnimatedWater();
	static bool isGeneratedWater();
	static bool isAnimatedLava();
	static bool isGeneratedLava();
	static bool isAnimatedFire();
	static bool isAnimatedPortal();
	static bool isAnimatedRedstone();
	static bool isAnimatedExplosion();
	static bool isAnimatedFlame();
	static bool isAnimatedSmoke();

	// Smooth lighting (AO)
	static float getAmbientOcclusionLevel();
	static float fixAoLight(float light, float defLight);
	static void setLightLevels(float *levels);

	// Cielo / clima / tiempo / agua
	static bool isWeatherEnabled();
	static bool isSkyEnabled();
	static bool isStarsEnabled();
	static bool isTimeDayOnly();
	static bool isTimeNightOnly();
	static bool isClearWater();

	// Util
	static int_t limit(int_t val, int_t min, int_t max);
	static float limit(float val, float min, float max);
	static void dbg(const std::string &s);
	static void log(const std::string &s);
	static void sleep(long ms);

private:
	static void checkOpenGlCaps();

	static GameSettings *gameSettings;
	static Minecraft *minecraft;
	static float *lightLevels;
};
