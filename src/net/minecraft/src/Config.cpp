#include "Config.h"

#include <iostream>
#include "GameSettings.h"
#include "IBlockAccess.h"
#include "pc/OpenGL.h"

GameSettings *Config::gameSettings = nullptr;
Minecraft *Config::minecraft = nullptr;
float *Config::lightLevels = nullptr;

void Config::checkOpenGlCaps()
{
	log("");
	log(std::string("OpenGL: ") + (const char *)glGetString(GL_RENDERER) +
		" version " + (const char *)glGetString(GL_VERSION) + ", " + (const char *)glGetString(GL_VENDOR));
	if (!isFancyFogAvailable())
		log("OpenGL Fancy fog: Not available (GL_NV_fog_distance)");
	if (!isOcclusionAvailable())
		log("OpenGL Occlussion culling: Not available (GL_ARB_occlusion_query)");
}

bool Config::isFancyFogAvailable()
{
	return SDL_GL_ExtensionSupported("GL_NV_fog_distance") == SDL_TRUE;
}

bool Config::isOcclusionAvailable()
{
	return SDL_GL_ExtensionSupported("GL_ARB_occlusion_query") == SDL_TRUE;
}

void Config::setGameSettings(GameSettings *options)
{
	if (gameSettings == nullptr)
		checkOpenGlCaps();
	gameSettings = options;
}

bool Config::isFogFancy()
{
	if (!isFancyFogAvailable())
		return false;
	if (gameSettings == nullptr)
		return false;
	return gameSettings->ofFogFancy;
}

bool Config::isFogOff()
{
	if (gameSettings == nullptr)
		return false;
	return gameSettings->ofFogOff;
}

float Config::getFogStart()
{
	if (gameSettings == nullptr)
		return DEF_FOG_START;
	return gameSettings->ofFogStart;
}

bool Config::isUseAlphaFunc()
{
	return getAlphaFuncLevel() > DEF_ALPHA_FUNC_LEVEL + 1.0e-5f;
}

float Config::getAlphaFuncLevel()
{
	return DEF_ALPHA_FUNC_LEVEL;
}

bool Config::isOcclusionEnabled()
{
	if (gameSettings == nullptr)
		return DEF_OCCLUSION_ENABLED;
	return gameSettings->advancedOpengl; // campo 'h'
}

bool Config::isOcclusionFancy()
{
	if (!isOcclusionEnabled())
		return false;
	if (gameSettings == nullptr)
		return false;
	return gameSettings->ofOcclusionFancy;
}

bool Config::isLoadChunksFar()
{
	if (gameSettings == nullptr)
		return DEF_LOAD_CHUNKS_FAR;
	return gameSettings->ofLoadFar;
}

int_t Config::getPreloadedChunks()
{
	if (gameSettings == nullptr)
		return DEF_PRELOADED_CHUNKS;
	return gameSettings->ofPreloadedChunks;
}

int_t Config::getUpdatesPerFrame()
{
	if (gameSettings != nullptr)
		return gameSettings->ofChunkUpdates;
	return 1;
}

bool Config::isDynamicUpdates()
{
	if (gameSettings != nullptr)
		return gameSettings->ofChunkUpdatesDynamic;
	return true;
}

bool Config::isBackgroundChunkLoading()
{
	return true;
}

bool Config::isFarView()
{
	if (gameSettings == nullptr)
		return false;
	return gameSettings->ofFarView;
}

bool Config::isRainFancy()
{
	if (gameSettings->ofRain == 0)
		return gameSettings->fancyGraphics; // 'j'
	return gameSettings->ofRain == 2;
}

bool Config::isRainOff()
{
	return gameSettings->ofRain == 3;
}

bool Config::isWaterFancy()
{
	if (gameSettings->ofWater == 0)
		return gameSettings->fancyGraphics;
	return gameSettings->ofWater == 2;
}

bool Config::isCloudsFancy()
{
	if (gameSettings->ofClouds == 0)
		return gameSettings->fancyGraphics;
	return gameSettings->ofClouds == 2;
}

bool Config::isCloudsOff()
{
	return gameSettings->ofClouds == 3;
}

bool Config::isTreesFancy()
{
	if (gameSettings->ofTrees == 0)
		return gameSettings->fancyGraphics;
	return gameSettings->ofTrees == 2;
}

bool Config::isGrassFancy()
{
	if (gameSettings->ofGrass == 0)
		return gameSettings->fancyGraphics;
	return gameSettings->ofGrass == 2;
}

bool Config::isBetterGrass()
{
	if (gameSettings == nullptr)
		return false;
	return gameSettings->ofBetterGrass != 3;
}

bool Config::isBetterGrassFancy()
{
	if (gameSettings == nullptr)
		return false;
	return gameSettings->ofBetterGrass == 2;
}

int_t Config::getSideGrassTexture(IBlockAccess *blockAccess, int_t x, int_t y, int_t z, int_t side)
{
	if (!isBetterGrass())
		return 3;
	if (isBetterGrassFancy())
	{
		y--;
		switch (side)
		{
		case 2: z--; break;
		case 3: z++; break;
		case 4: x--; break;
		case 5: x++; break;
		}
		int_t blockId = blockAccess->getBlockId(x, y, z);
		if (blockId != 2)
			return 3;
	}
	return 0;
}

int_t Config::getSideSnowGrassTexture(IBlockAccess *blockAccess, int_t x, int_t y, int_t z, int_t side)
{
	if (!isBetterGrass())
		return 68;
	if (isBetterGrassFancy())
	{
		switch (side)
		{
		case 2: z--; break;
		case 3: z++; break;
		case 4: x--; break;
		case 5: x++; break;
		}
		int_t blockId = blockAccess->getBlockId(x, y, z);
		if (blockId != 78 && blockId != 80)
			return 68;
	}
	return 66;
}

bool Config::isAnimatedWater()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAnimatedWater != 2;
	return true;
}

bool Config::isGeneratedWater()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAnimatedWater == 1;
	return true;
}

bool Config::isAnimatedLava()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAnimatedLava != 2;
	return true;
}

bool Config::isGeneratedLava()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAnimatedLava == 1;
	return true;
}

bool Config::isAnimatedPortal()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAnimatedPortal;
	return true;
}

bool Config::isAnimatedFire()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAnimatedFire;
	return true;
}

bool Config::isAnimatedRedstone()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAnimatedRedstone;
	return true;
}

bool Config::isAnimatedExplosion()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAnimatedExplosion;
	return true;
}

bool Config::isAnimatedFlame()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAnimatedFlame;
	return true;
}

bool Config::isAnimatedSmoke()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAnimatedSmoke;
	return true;
}

float Config::getAmbientOcclusionLevel()
{
	if (gameSettings != nullptr)
		return gameSettings->ofAoLevel;
	return 0.0f;
}

float Config::fixAoLight(float light, float defLight)
{
	if (lightLevels == nullptr)
		return light;
	float level_0 = lightLevels[0];
	float level_1 = lightLevels[1];
	if (light > level_0)
		return light;
	if (defLight <= level_1)
		return light;
	float mul = 1.0f - getAmbientOcclusionLevel();
	return light + (defLight - light) * mul;
}

void Config::setLightLevels(float *levels)
{
	lightLevels = levels;
}

bool Config::isWeatherEnabled()
{
	if (gameSettings == nullptr)
		return true;
	return gameSettings->ofWeather;
}

bool Config::isSkyEnabled()
{
	if (gameSettings == nullptr)
		return true;
	return gameSettings->ofSky;
}

bool Config::isStarsEnabled()
{
	if (gameSettings == nullptr)
		return true;
	return gameSettings->ofStars;
}

bool Config::isTimeDayOnly()
{
	if (gameSettings == nullptr)
		return false;
	return gameSettings->ofTime == 1;
}

bool Config::isTimeNightOnly()
{
	if (gameSettings == nullptr)
		return false;
	return gameSettings->ofTime == 2;
}

bool Config::isClearWater()
{
	if (gameSettings == nullptr)
		return false;
	return gameSettings->ofClearWater;
}

int_t Config::limit(int_t val, int_t min, int_t max)
{
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

float Config::limit(float val, float min, float max)
{
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

void Config::setMinecraft(Minecraft *mc) { minecraft = mc; }
Minecraft *Config::getMinecraft() { return minecraft; }

void Config::dbg(const std::string &s) { std::cout << s << std::endl; }
void Config::log(const std::string &s) { dbg(s); }

void Config::sleep(long ms)
{
	SDL_Delay((Uint32)ms);
}
