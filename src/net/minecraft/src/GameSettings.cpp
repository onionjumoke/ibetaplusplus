#include "GameSettings.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include "EnumOptions.h"
#include "EnumOptionsMappingHelper.h"
#include "KeyBinding.h"
#include "StringTranslate.h"
#include "Config.h"
#include "Minecraft.h"
#include "EntityRenderer.h"
#include "RenderGlobal.h"
#include "RenderBlocks.h"

const char *GameSettings::RENDER_DISTANCES[4] = {
	"options.renderDistance.far", "options.renderDistance.normal", "options.renderDistance.short", "options.renderDistance.tiny"
};

const char *GameSettings::DIFFICULTIES[4] = {
	"options.difficulty.peaceful", "options.difficulty.easy", "options.difficulty.normal", "options.difficulty.hard"
};

const char *GameSettings::GUISCALES[4] = {
	"options.guiScale.auto", "options.guiScale.small", "options.guiScale.normal", "options.guiScale.large"
};

const char *GameSettings::LIMIT_FRAMERATES[3] = {
	"performance.max", "performance.balanced", "performance.powersaver"
};

GameSettings::GameSettings(Minecraft *minecraft, const std::string &file)
{
	setDefaults();
	mc = minecraft;
	optionsFile = file + "/options.txt";
	loadOptions();
	Config::setGameSettings(this);
}

GameSettings::GameSettings()
{
	setDefaults();
}

void GameSettings::setDefaults()
{
	musicVolume = 1.0f;
	soundVolume = 1.0f;
	mouseSensitivity = 0.5f;
	invertMouse = false;
#if defined(__ARM_ARCH)
	renderDistance = 3;   // TINY (~4 chunks): the only playable tier on 4S
#else
	renderDistance = 0;
#endif
	viewBobbing = true;
	anaglyph = false;
	advancedOpengl = false;
#if defined(__ARM_ARCH)
	limitFramerate = 0;    // no busy-wait cap burning CPU on a phone
	fancyGraphics = false; // fancy = extra water/leaf passes SGX543 can't spare
#else
	limitFramerate = 1;
	fancyGraphics = true;
#endif
	ambientOcclusion = true;
	skin = "Default";
	keyBindForward = new KeyBinding("key.forward", 17);
	keyBindLeft = new KeyBinding("key.left", 30);
	keyBindBack = new KeyBinding("key.back", 31);
	keyBindRight = new KeyBinding("key.right", 32);
	keyBindJump = new KeyBinding("key.jump", 57);
	keyBindInventory = new KeyBinding("key.inventory", 18);
	keyBindDrop = new KeyBinding("key.drop", 16);
	keyBindChat = new KeyBinding("key.chat", 20);
	keyBindToggleFog = new KeyBinding("key.fog", 33);
	keyBindSneak = new KeyBinding("key.sneak", 42);
	keyBindings = {
		keyBindForward, keyBindLeft, keyBindBack, keyBindRight, keyBindJump, keyBindSneak,
		keyBindDrop, keyBindInventory, keyBindChat, keyBindToggleFog
	};
	mc = nullptr;
	optionsFile = "options.txt";
	difficulty = 2;
	hideGUI = false;
	thirdPersonView = false;
	showDebugInfo = false;
	lastServer = "";
	field_22275_C = false;
	smoothCamera = false;
	field_22273_E = false;
	field_22272_F = 1.0f;
	field_22271_G = 1.0f;
	guiScale = 0;

	// --- OptiFine defaults (kv ctor) ---
	ofFogFancy = false;
	ofFogOff = false;
	ofFogStart = 0.8f;
	ofLoadFar = false;
	ofPreloadedChunks = 0;
	ofOcclusionFancy = false;
	ofSmoothFps = false;
	ofSmoothInput = false;
	ofBrightness = 0.0f;
	ofAoLevel = 0.0f;
	ofClouds = 0;
	ofCloudsHeight = 0.0f;
	ofTrees = 0;
	ofGrass = 0;
	ofRain = 0;
	ofWater = 0;
	ofBetterGrass = 3;
	ofAutoSaveTicks = 4000;
	ofFastDebugInfo = false;
	ofWeather = true;
	ofCaveSounds = true;
	ofMobs = true;
	ofFov = 70.0f;
	ofParticles = true;
	ofShowChunkBorders = false;
	ofSky = true;
	ofStars = true;
#if defined(__ARM_ARCH)
	// At phone fps the vanilla 1-2/frame budget never drains the 600+
	// renderer queue after crossing a chunk border — distant terrain stayed
	// invisible for tens of seconds. 8/frame keeps up.
	ofChunkUpdates = 8;
#else
	ofChunkUpdates = 1;
#endif
	ofChunkUpdatesDynamic = true;
	ofFarView = false;
	ofTime = 0;
	ofClearWater = false;
	ofAnimatedWater = 0;
	ofAnimatedLava = 0;
	ofAnimatedFire = true;
	ofAnimatedPortal = true;
	ofAnimatedRedstone = true;
	ofAnimatedExplosion = true;
	ofAnimatedFlame = true;
	ofAnimatedSmoke = true;
	ofKeyBindZoom = new KeyBinding("Zoom", 46);
	keyBindings.push_back(ofKeyBindZoom);
}

void GameSettings::reloadChunkRenderers()
{
	if (mc != nullptr && mc->renderGlobal != nullptr)
		mc->renderGlobal->loadRenderers();
}

// OptiFine: GameSettings.updateWorldLightLevels() -> mc.entityRenderer.updateWorldLightLevels()
void GameSettings::updateWorldLightLevels()
{
	if (mc != nullptr && mc->entityRenderer != nullptr)
		mc->entityRenderer->updateWorldLightLevels();
}

std::string GameSettings::getKeyBindingDescription(int_t i)
{
	return translateKey(keyBindings[i]->keyDescription);
}

std::string GameSettings::getOptionDisplayString(int_t i)
{
	return keyName(keyBindings[i]->keyCode);
}

void GameSettings::setKeyBinding(int_t i, int_t j)
{
	keyBindings[i]->keyCode = j;
	saveOptions();
}

void GameSettings::setOptionFloatValue(const EnumOptions *enumoptions, float f)
{
	if (enumoptions == EnumOptions::MUSIC)
		musicVolume = f;
	if (enumoptions == EnumOptions::SOUND)
		soundVolume = f;
	if (enumoptions == EnumOptions::SENSITIVITY)
		mouseSensitivity = f;
	if (enumoptions == EnumOptions::FOV)
		ofFov = 30.0f + f * 80.0f;
	// --- OptiFine ---
	if (enumoptions == EnumOptions::BRIGHTNESS)
	{
		ofBrightness = f;
		updateWorldLightLevels();
	}
	if (enumoptions == EnumOptions::CLOUD_HEIGHT)
		ofCloudsHeight = f;
	if (enumoptions == EnumOptions::AO_LEVEL)
	{
		ofAoLevel = f;
		ambientOcclusion = (ofAoLevel > 0.0f); // 'k'
		reloadChunkRenderers();
	}
}

void GameSettings::setOptionValue(const EnumOptions *enumoptions, int_t i)
{
	if (enumoptions == EnumOptions::INVERT_MOUSE)
		invertMouse = !invertMouse;
	if (enumoptions == EnumOptions::RENDER_DISTANCE)
		renderDistance = renderDistance + i & 3;
	if (enumoptions == EnumOptions::GUI_SCALE)
		guiScale = guiScale + i & 3;
	if (enumoptions == EnumOptions::VIEW_BOBBING)
		viewBobbing = !viewBobbing;
	// ADVANCED_OPENGL = occlusion culling (OFF -> Fast -> Fancy -> OFF), campo 'h'
	if (enumoptions == EnumOptions::ADVANCED_OPENGL)
	{
		if (!Config::isOcclusionAvailable())
		{
			ofOcclusionFancy = false;
			advancedOpengl = false;
		}
		else if (!advancedOpengl)
		{
			advancedOpengl = true;
			ofOcclusionFancy = false;
		}
		else if (!ofOcclusionFancy)
		{
			ofOcclusionFancy = true;
		}
		else
		{
			ofOcclusionFancy = false;
			advancedOpengl = false;
		}
		// TODO F5: mc->renderGlobal->setAllRenderesVisible()
	}
	if (enumoptions == EnumOptions::ANAGLYPH)
		anaglyph = !anaglyph;
	if (enumoptions == EnumOptions::FRAMERATE_LIMIT)
		limitFramerate = (limitFramerate + i + 3) % 3;
	if (enumoptions == EnumOptions::DIFFICULTY)
		difficulty = difficulty + i & 3;
	if (enumoptions == EnumOptions::GRAPHICS)
	{
		fancyGraphics = !fancyGraphics;
		reloadChunkRenderers();
	}
	if (enumoptions == EnumOptions::AMBIENT_OCCLUSION)
	{
		ambientOcclusion = !ambientOcclusion;
		reloadChunkRenderers();
	}
	// --- OptiFine ---
	if (enumoptions == EnumOptions::FOG_FANCY)
	{
		// Ciclo tri-estado: Fast -> Fancy -> OFF -> Fast.
		// (Si Fancy no esta disponible: Fast -> OFF -> Fast.)
		if (ofFogOff)
		{
			// OFF -> Fast
			ofFogOff = false;
			ofFogFancy = false;
		}
		else if (ofFogFancy)
		{
			// Fancy -> OFF
			ofFogOff = true;
			ofFogFancy = false;
		}
		else if (Config::isFancyFogAvailable())
		{
			// Fast -> Fancy
			ofFogFancy = true;
		}
		else
		{
			// Fast -> OFF (Fancy no disponible)
			ofFogOff = true;
		}
	}
	if (enumoptions == EnumOptions::FOG_START)
	{
		ofFogStart += 0.2f;
		if (ofFogStart > 0.81f)
			ofFogStart = 0.2f;
	}
	if (enumoptions == EnumOptions::LOAD_FAR)
	{
		ofLoadFar = !ofLoadFar;
		reloadChunkRenderers();
	}
	if (enumoptions == EnumOptions::PRELOADED_CHUNKS)
	{
		ofPreloadedChunks += 2;
		if (ofPreloadedChunks > 8)
			ofPreloadedChunks = 0;
		reloadChunkRenderers();
	}
	if (enumoptions == EnumOptions::SMOOTH_FPS)
		ofSmoothFps = !ofSmoothFps;
	if (enumoptions == EnumOptions::SMOOTH_INPUT)
		ofSmoothInput = !ofSmoothInput;
	if (enumoptions == EnumOptions::CLOUDS)
	{
		ofClouds++;
		if (ofClouds > 3)
			ofClouds = 0;
	}
	if (enumoptions == EnumOptions::TREES)
	{
		ofTrees++;
		if (ofTrees > 2)
			ofTrees = 0;
		reloadChunkRenderers();
	}
	if (enumoptions == EnumOptions::GRASS)
	{
		ofGrass++;
		if (ofGrass > 2)
			ofGrass = 0;
		RenderBlocks::fancyGrass = Config::isGrassFancy();
		reloadChunkRenderers();
	}
	if (enumoptions == EnumOptions::RAIN)
	{
		ofRain++;
		if (ofRain > 3)
			ofRain = 0;
	}
	if (enumoptions == EnumOptions::WATER)
	{
		ofWater++;
		if (ofWater > 2)
			ofWater = 0;
	}
	if (enumoptions == EnumOptions::ANIMATED_WATER)
	{
		ofAnimatedWater++;
		if (ofAnimatedWater > 2)
			ofAnimatedWater = 0;
		// TODO F6: mc->renderEngine->refreshTextures()
	}
	if (enumoptions == EnumOptions::ANIMATED_LAVA)
	{
		ofAnimatedLava++;
		if (ofAnimatedLava > 2)
			ofAnimatedLava = 0;
		// TODO F6: refreshTextures()
	}
	if (enumoptions == EnumOptions::ANIMATED_FIRE)
		ofAnimatedFire = !ofAnimatedFire; // TODO F6: refreshTextures()
	if (enumoptions == EnumOptions::ANIMATED_PORTAL)
		ofAnimatedPortal = !ofAnimatedPortal; // TODO F6: refreshTextures()
	if (enumoptions == EnumOptions::ANIMATED_REDSTONE)
		ofAnimatedRedstone = !ofAnimatedRedstone;
	if (enumoptions == EnumOptions::ANIMATED_EXPLOSION)
		ofAnimatedExplosion = !ofAnimatedExplosion;
	if (enumoptions == EnumOptions::ANIMATED_FLAME)
		ofAnimatedFlame = !ofAnimatedFlame;
	if (enumoptions == EnumOptions::ANIMATED_SMOKE)
		ofAnimatedSmoke = !ofAnimatedSmoke;
	if (enumoptions == EnumOptions::FAST_DEBUG_INFO)
		ofFastDebugInfo = !ofFastDebugInfo;
	if (enumoptions == EnumOptions::AUTOSAVE_TICKS)
	{
		ofAutoSaveTicks *= 10;
		if (ofAutoSaveTicks > 40000)
			ofAutoSaveTicks = 40;
	}
	if (enumoptions == EnumOptions::BETTER_GRASS)
	{
		ofBetterGrass++;
		if (ofBetterGrass > 3)
			ofBetterGrass = 1;
		reloadChunkRenderers();
	}
	if (enumoptions == EnumOptions::WEATHER)
		ofWeather = !ofWeather;
	if (enumoptions == EnumOptions::CAVE_SOUNDS)
		ofCaveSounds = !ofCaveSounds;
	if (enumoptions == EnumOptions::MOBS)
		ofMobs = !ofMobs;
	if (enumoptions == EnumOptions::PARTICLES)
		ofParticles = !ofParticles;
	if (enumoptions == EnumOptions::SKY)
		ofSky = !ofSky;
	if (enumoptions == EnumOptions::STARS)
		ofStars = !ofStars;
	if (enumoptions == EnumOptions::CHUNK_UPDATES)
	{
		ofChunkUpdates++;
		if (ofChunkUpdates > 5)
			ofChunkUpdates = 1;
	}
	if (enumoptions == EnumOptions::CHUNK_UPDATES_DYNAMIC)
		ofChunkUpdatesDynamic = !ofChunkUpdatesDynamic;
	if (enumoptions == EnumOptions::FAR_VIEW)
	{
		ofFarView = !ofFarView;
		reloadChunkRenderers();
	}
	if (enumoptions == EnumOptions::TIME)
	{
		ofTime++;
		if (ofTime > 2)
			ofTime = 0;
	}
	if (enumoptions == EnumOptions::CLEAR_WATER)
		ofClearWater = !ofClearWater; // TODO F5: updateWaterOpacity()
	saveOptions();
}

float GameSettings::getOptionFloatValue(const EnumOptions *enumoptions)
{
	if (enumoptions == EnumOptions::MUSIC)
		return musicVolume;
	if (enumoptions == EnumOptions::SOUND)
		return soundVolume;
	if (enumoptions == EnumOptions::SENSITIVITY)
		return mouseSensitivity;
	if (enumoptions == EnumOptions::FOV)
		return (ofFov - 30.0f) / 80.0f;
	if (enumoptions == EnumOptions::BRIGHTNESS)
		return ofBrightness;
	if (enumoptions == EnumOptions::CLOUD_HEIGHT)
		return ofCloudsHeight;
	if (enumoptions == EnumOptions::AO_LEVEL)
		return ofAoLevel;
	return 0.0f;
}

bool GameSettings::getOptionOrdinalValue(const EnumOptions *enumoptions)
{
	switch (EnumOptionsMappingHelper::enumOptionsMappingHelperArray[enumoptions->returnEnumOrdinal()])
	{
	case 1:
		return invertMouse;
	case 2:
		return viewBobbing;
	case 3:
		return anaglyph;
	case 4:
		return advancedOpengl;
	case 5:
		return ambientOcclusion;
	}
	return false;
}

std::string GameSettings::getKeyBinding(const EnumOptions *enumoptions)
{
	std::string s = translateKey(enumoptions->getEnumString()) + ": ";
	if (enumoptions->getEnumFloat())
	{
		float f = getOptionFloatValue(enumoptions);
		if (enumoptions == EnumOptions::FOV)
			return s + std::to_string((int_t)ofFov);
		if (enumoptions == EnumOptions::SENSITIVITY)
		{
			if (f == 0.0f)
				return s + translateKey("options.sensitivity.min");
			if (f == 1.0f)
				return s + translateKey("options.sensitivity.max");
			return s + std::to_string((int_t)(f * 200.0f)) + "%";
		}
		if (f == 0.0f)
			return s + translateKey("options.off");
		return s + std::to_string((int_t)(f * 100.0f)) + "%";
	}
	// ADVANCED_OPENGL = occlusion (OFF / Fast / Fancy), antes del branch booleano
	if (enumoptions == EnumOptions::ADVANCED_OPENGL)
	{
		if (!advancedOpengl)
			return s + "OFF";
		if (ofOcclusionFancy)
			return s + "Fancy";
		return s + "Fast";
	}
	if (enumoptions->getEnumBoolean())
	{
		bool flag = getOptionOrdinalValue(enumoptions);
		if (flag)
			return s + translateKey("options.on");
		return s + translateKey("options.off");
	}
	if (enumoptions == EnumOptions::RENDER_DISTANCE)
		return s + translateKey(RENDER_DISTANCES[renderDistance]);
	if (enumoptions == EnumOptions::DIFFICULTY)
		return s + translateKey(DIFFICULTIES[difficulty]);
	if (enumoptions == EnumOptions::GUI_SCALE)
		return s + translateKey(GUISCALES[guiScale]);
	if (enumoptions == EnumOptions::FRAMERATE_LIMIT)
		return s + translateKey(LIMIT_FRAMERATES[limitFramerate]);
	if (enumoptions == EnumOptions::GRAPHICS)
	{
		if (fancyGraphics)
			return s + translateKey("options.graphics.fancy");
		return s + translateKey("options.graphics.fast");
	}
	// --- OptiFine ---
	if (enumoptions == EnumOptions::FOG_FANCY)
		return s + (ofFogOff ? "OFF" : (ofFogFancy ? "Fancy" : "Fast"));
	if (enumoptions == EnumOptions::FOG_START)
		return s + std::to_string(ofFogStart);
	if (enumoptions == EnumOptions::LOAD_FAR)
		return s + (ofLoadFar ? "ON" : "OFF");
	if (enumoptions == EnumOptions::PRELOADED_CHUNKS)
		return s + (ofPreloadedChunks == 0 ? std::string("OFF") : std::to_string(ofPreloadedChunks));
	if (enumoptions == EnumOptions::SMOOTH_FPS)
		return s + (ofSmoothFps ? "ON" : "OFF");
	if (enumoptions == EnumOptions::SMOOTH_INPUT)
		return s + (ofSmoothInput ? "ON" : "OFF");
	if (enumoptions == EnumOptions::CLOUDS)
	{
		switch (ofClouds)
		{
		case 1: return s + "Fast";
		case 2: return s + "Fancy";
		case 3: return s + "OFF";
		}
		return s + "Default";
	}
	if (enumoptions == EnumOptions::TREES)
	{
		switch (ofTrees)
		{
		case 1: return s + "Fast";
		case 2: return s + "Fancy";
		}
		return s + "Default";
	}
	if (enumoptions == EnumOptions::GRASS)
	{
		switch (ofGrass)
		{
		case 1: return s + "Fast";
		case 2: return s + "Fancy";
		}
		return s + "Default";
	}
	if (enumoptions == EnumOptions::RAIN)
	{
		switch (ofRain)
		{
		case 1: return s + "Fast";
		case 2: return s + "Fancy";
		case 3: return s + "OFF";
		}
		return s + "Default";
	}
	if (enumoptions == EnumOptions::WATER)
	{
		switch (ofWater)
		{
		case 1: return s + "Fast";
		case 2: return s + "Fancy";
		case 3: return s + "OFF";
		}
		return s + "Default";
	}
	if (enumoptions == EnumOptions::ANIMATED_WATER)
	{
		switch (ofAnimatedWater)
		{
		case 1: return s + "Dynamic";
		case 2: return s + "OFF";
		}
		return s + "ON";
	}
	if (enumoptions == EnumOptions::ANIMATED_LAVA)
	{
		switch (ofAnimatedLava)
		{
		case 1: return s + "Dynamic";
		case 2: return s + "OFF";
		}
		return s + "ON";
	}
	if (enumoptions == EnumOptions::ANIMATED_FIRE)
		return s + (ofAnimatedFire ? "ON" : "OFF");
	if (enumoptions == EnumOptions::ANIMATED_PORTAL)
		return s + (ofAnimatedPortal ? "ON" : "OFF");
	if (enumoptions == EnumOptions::ANIMATED_REDSTONE)
		return s + (ofAnimatedRedstone ? "ON" : "OFF");
	if (enumoptions == EnumOptions::ANIMATED_EXPLOSION)
		return s + (ofAnimatedExplosion ? "ON" : "OFF");
	if (enumoptions == EnumOptions::ANIMATED_FLAME)
		return s + (ofAnimatedFlame ? "ON" : "OFF");
	if (enumoptions == EnumOptions::ANIMATED_SMOKE)
		return s + (ofAnimatedSmoke ? "ON" : "OFF");
	if (enumoptions == EnumOptions::FAST_DEBUG_INFO)
		return s + (ofFastDebugInfo ? "ON" : "OFF");
	if (enumoptions == EnumOptions::AUTOSAVE_TICKS)
	{
		if (ofAutoSaveTicks <= 40)
			return s + "Default (2s)";
		if (ofAutoSaveTicks <= 400)
			return s + "20s";
		if (ofAutoSaveTicks <= 4000)
			return s + "3min";
		return s + "30min";
	}
	if (enumoptions == EnumOptions::BETTER_GRASS)
	{
		switch (ofBetterGrass)
		{
		case 1: return s + "Fast";
		case 2: return s + "Fancy";
		}
		return s + "OFF";
	}
	if (enumoptions == EnumOptions::WEATHER)
		return s + (ofWeather ? "ON" : "OFF");
	if (enumoptions == EnumOptions::CAVE_SOUNDS)
		return s + (ofCaveSounds ? "ON" : "OFF");
	if (enumoptions == EnumOptions::MOBS)
		return s + (ofMobs ? "ON" : "OFF");
	if (enumoptions == EnumOptions::PARTICLES)
		return s + (ofParticles ? "ON" : "OFF");
	if (enumoptions == EnumOptions::SKY)
		return s + (ofSky ? "ON" : "OFF");
	if (enumoptions == EnumOptions::STARS)
		return s + (ofStars ? "ON" : "OFF");
	if (enumoptions == EnumOptions::CHUNK_UPDATES)
		return s + std::to_string(ofChunkUpdates);
	if (enumoptions == EnumOptions::CHUNK_UPDATES_DYNAMIC)
		return s + (ofChunkUpdatesDynamic ? "ON" : "OFF");
	if (enumoptions == EnumOptions::FAR_VIEW)
		return s + (ofFarView ? "ON" : "OFF");
	if (enumoptions == EnumOptions::TIME)
	{
		if (ofTime == 1)
			return s + "Day Only";
		if (ofTime == 2)
			return s + "Night Only";
		return s + "Default";
	}
	if (enumoptions == EnumOptions::CLEAR_WATER)
		return s + (ofClearWater ? "ON" : "OFF");
	return s;
}

void GameSettings::loadOptions()
{
	std::ifstream bufferedreader(optionsFile);
	if (!bufferedreader)
		return;

	std::string s;
	while (std::getline(bufferedreader, s))
	{
		try
		{
			std::size_t pos = s.find(':');
			if (pos == std::string::npos)
				continue;
			std::string key = s.substr(0, pos);
			std::string value = s.substr(pos + 1);
			if (key == "music")
				musicVolume = parseFloat(value);
			if (key == "sound")
				soundVolume = parseFloat(value);
			if (key == "mouseSensitivity")
				mouseSensitivity = parseFloat(value);
			if (key == "invertYMouse")
				invertMouse = value == "true";
			if (key == "viewDistance")
				renderDistance = std::stoi(value);
			if (key == "guiScale")
				guiScale = std::stoi(value);
			if (key == "bobView")
				viewBobbing = value == "true";
			if (key == "anaglyph3d")
				anaglyph = value == "true";
			if (key == "advancedOpengl")
				advancedOpengl = value == "true";
			if (key == "fpsLimit")
				limitFramerate = std::stoi(value);
			if (key == "difficulty")
				difficulty = std::stoi(value);
			if (key == "fancyGraphics")
				fancyGraphics = value == "true";
			if (key == "ao")
				ambientOcclusion = value == "true";
			if (key == "skin")
				skin = value;
			if (key == "lastServer")
				lastServer = value;
			for (int_t i = 0; i < (int_t)keyBindings.size(); i++)
			{
				if (key == "key_" + keyBindings[i]->keyDescription)
					keyBindings[i]->keyCode = std::stoi(value);
			}
			// --- OptiFine (mipmaps excluidos) ---
			if (key == "ofFogFancy")
				ofFogFancy = value == "true";
			if (key == "ofFogOff")
				ofFogOff = value == "true";
			if (key == "ofFogStart")
			{
				ofFogStart = std::stof(value);
				if (ofFogStart < 0.2f)
					ofFogStart = 0.2f;
				if (ofFogStart > 0.81f)
					ofFogStart = 0.8f;
			}
			if (key == "ofLoadFar")
				ofLoadFar = value == "true";
			if (key == "ofPreloadedChunks")
				ofPreloadedChunks = Config::limit((int_t)std::stoi(value), 0, 8);
			if (key == "ofOcclusionFancy")
				ofOcclusionFancy = value == "true";
			if (key == "ofSmoothFps")
				ofSmoothFps = value == "true";
			if (key == "ofSmoothInput")
				ofSmoothInput = value == "true";
			if (key == "ofBrightness")
				// Se aplica al entrar al mundo via EntityRenderer (worldProvider-change check)
				ofBrightness = Config::limit(std::stof(value), 0.0f, 1.0f);
			if (key == "ofAoLevel")
			{
				ofAoLevel = Config::limit(std::stof(value), 0.0f, 1.0f);
				ambientOcclusion = (ofAoLevel > 0.0f);
			}
			if (key == "ofClouds")
				ofClouds = Config::limit((int_t)std::stoi(value), 0, 3);
			if (key == "ofCloudsHeight")
				ofCloudsHeight = Config::limit(std::stof(value), 0.0f, 1.0f);
			if (key == "ofTrees")
				ofTrees = Config::limit((int_t)std::stoi(value), 0, 2);
			if (key == "ofGrass")
				ofGrass = Config::limit((int_t)std::stoi(value), 0, 2);
			if (key == "ofRain")
				ofRain = Config::limit((int_t)std::stoi(value), 0, 3);
			if (key == "ofWater")
				ofWater = Config::limit((int_t)std::stoi(value), 0, 3);
			if (key == "ofAnimatedWater")
				ofAnimatedWater = Config::limit((int_t)std::stoi(value), 0, 2);
			if (key == "ofAnimatedLava")
				ofAnimatedLava = Config::limit((int_t)std::stoi(value), 0, 2);
			if (key == "ofAnimatedFire")
				ofAnimatedFire = value == "true";
			if (key == "ofAnimatedPortal")
				ofAnimatedPortal = value == "true";
			if (key == "ofAnimatedRedstone")
				ofAnimatedRedstone = value == "true";
			if (key == "ofAnimatedExplosion")
				ofAnimatedExplosion = value == "true";
			if (key == "ofAnimatedFlame")
				ofAnimatedFlame = value == "true";
			if (key == "ofAnimatedSmoke")
				ofAnimatedSmoke = value == "true";
			if (key == "ofFastDebugInfo")
				ofFastDebugInfo = value == "true";
			if (key == "ofAutoSaveTicks")
				ofAutoSaveTicks = Config::limit((int_t)std::stoi(value), 40, 40000);
			if (key == "ofBetterGrass")
				ofBetterGrass = Config::limit((int_t)std::stoi(value), 1, 3);
			if (key == "ofWeather")
				ofWeather = value == "true";
			if (key == "ofCaveSounds")
				ofCaveSounds = value == "true";
			if (key == "ofMobs")
				ofMobs = value == "true";
			if (key == "ofFov")
				ofFov = std::stof(value);
			if (key == "ofParticles")
				ofParticles = value == "true";
			if (key == "ofSky")
				ofSky = value == "true";
			if (key == "ofStars")
				ofStars = value == "true";
			if (key == "ofChunkUpdates")
				ofChunkUpdates = Config::limit((int_t)std::stoi(value), 1, 5);
			if (key == "ofChunkUpdatesDynamic")
				ofChunkUpdatesDynamic = value == "true";
			if (key == "ofFarView")
				ofFarView = value == "true";
			if (key == "ofTime")
				ofTime = Config::limit((int_t)std::stoi(value), 0, 2);
			if (key == "ofClearWater")
				ofClearWater = value == "true"; // TODO F5: updateWaterOpacity()
		}
		catch (...)
		{
			std::cout << "Skipping bad option: " << s << std::endl;
		}
	}
#if defined(__ARM_ARCH)
	// Stale options.txt must not restore unplayable tiers on phones.
	renderDistance = 3;   // TINY
	fancyGraphics = false;
	limitFramerate = 0;
	ofChunkUpdates = 2;   // vanilla-equivalent
	fprintf(stderr, "Beta++: settings forced: rd=%d fancy=%d limit=%d updates=%d\n",
	        renderDistance, fancyGraphics, limitFramerate, ofChunkUpdates);
#endif
}

float GameSettings::parseFloat(const std::string &s)
{
	if (s == "true")
		return 1.0f;
	if (s == "false")
		return 0.0f;
	return std::stof(s);
}

void GameSettings::saveOptions()
{
	std::ofstream printwriter(optionsFile, std::ios::trunc);
	if (!printwriter)
		return;

	printwriter << "music:" << musicVolume << "\n";
	printwriter << "sound:" << soundVolume << "\n";
	printwriter << "invertYMouse:" << (invertMouse ? "true" : "false") << "\n";
	printwriter << "mouseSensitivity:" << mouseSensitivity << "\n";
	printwriter << "viewDistance:" << renderDistance << "\n";
	printwriter << "guiScale:" << guiScale << "\n";
	printwriter << "bobView:" << (viewBobbing ? "true" : "false") << "\n";
	printwriter << "anaglyph3d:" << (anaglyph ? "true" : "false") << "\n";
	printwriter << "advancedOpengl:" << (advancedOpengl ? "true" : "false") << "\n";
	printwriter << "fpsLimit:" << limitFramerate << "\n";
	printwriter << "difficulty:" << difficulty << "\n";
	printwriter << "fancyGraphics:" << (fancyGraphics ? "true" : "false") << "\n";
	printwriter << "ao:" << (ambientOcclusion ? "true" : "false") << "\n";
	printwriter << "skin:" << skin << "\n";
	printwriter << "lastServer:" << lastServer << "\n";
	for (int_t i = 0; i < (int_t)keyBindings.size(); i++)
		printwriter << "key_" << keyBindings[i]->keyDescription << ":" << keyBindings[i]->keyCode << "\n";
	// --- OptiFine (mipmaps excluidos) ---
	printwriter << "ofFogFancy:" << (ofFogFancy ? "true" : "false") << "\n";
	printwriter << "ofFogOff:" << (ofFogOff ? "true" : "false") << "\n";
	printwriter << "ofFogStart:" << ofFogStart << "\n";
	printwriter << "ofLoadFar:" << (ofLoadFar ? "true" : "false") << "\n";
	printwriter << "ofPreloadedChunks:" << ofPreloadedChunks << "\n";
	printwriter << "ofOcclusionFancy:" << (ofOcclusionFancy ? "true" : "false") << "\n";
	printwriter << "ofSmoothFps:" << (ofSmoothFps ? "true" : "false") << "\n";
	printwriter << "ofSmoothInput:" << (ofSmoothInput ? "true" : "false") << "\n";
	printwriter << "ofBrightness:" << ofBrightness << "\n";
	printwriter << "ofAoLevel:" << ofAoLevel << "\n";
	printwriter << "ofClouds:" << ofClouds << "\n";
	printwriter << "ofCloudsHeight:" << ofCloudsHeight << "\n";
	printwriter << "ofTrees:" << ofTrees << "\n";
	printwriter << "ofGrass:" << ofGrass << "\n";
	printwriter << "ofRain:" << ofRain << "\n";
	printwriter << "ofWater:" << ofWater << "\n";
	printwriter << "ofAnimatedWater:" << ofAnimatedWater << "\n";
	printwriter << "ofAnimatedLava:" << ofAnimatedLava << "\n";
	printwriter << "ofAnimatedFire:" << (ofAnimatedFire ? "true" : "false") << "\n";
	printwriter << "ofAnimatedPortal:" << (ofAnimatedPortal ? "true" : "false") << "\n";
	printwriter << "ofAnimatedRedstone:" << (ofAnimatedRedstone ? "true" : "false") << "\n";
	printwriter << "ofAnimatedExplosion:" << (ofAnimatedExplosion ? "true" : "false") << "\n";
	printwriter << "ofAnimatedFlame:" << (ofAnimatedFlame ? "true" : "false") << "\n";
	printwriter << "ofAnimatedSmoke:" << (ofAnimatedSmoke ? "true" : "false") << "\n";
	printwriter << "ofFastDebugInfo:" << (ofFastDebugInfo ? "true" : "false") << "\n";
	printwriter << "ofAutoSaveTicks:" << ofAutoSaveTicks << "\n";
	printwriter << "ofBetterGrass:" << ofBetterGrass << "\n";
	printwriter << "ofWeather:" << (ofWeather ? "true" : "false") << "\n";
	printwriter << "ofCaveSounds:" << (ofCaveSounds ? "true" : "false") << "\n";
	printwriter << "ofMobs:" << (ofMobs ? "true" : "false") << "\n";
	printwriter << "ofFov:" << ofFov << "\n";
	printwriter << "ofParticles:" << (ofParticles ? "true" : "false") << "\n";
	printwriter << "ofSky:" << (ofSky ? "true" : "false") << "\n";
	printwriter << "ofStars:" << (ofStars ? "true" : "false") << "\n";
	printwriter << "ofChunkUpdates:" << ofChunkUpdates << "\n";
	printwriter << "ofChunkUpdatesDynamic:" << (ofChunkUpdatesDynamic ? "true" : "false") << "\n";
	printwriter << "ofFarView:" << (ofFarView ? "true" : "false") << "\n";
	printwriter << "ofTime:" << ofTime << "\n";
	printwriter << "ofClearWater:" << (ofClearWater ? "true" : "false") << "\n";
}

std::string GameSettings::translateKey(const std::string &s)
{
	return StringTranslate::getInstance()->translateKey(s);
}

std::string GameSettings::translateKey(const std::u16string &s)
{
	std::string out;
	out.reserve(s.size());
	for (char16_t c : s)
		out.push_back((char)c);
	return translateKey(out);
}

std::string GameSettings::keyName(int_t keyCode)
{
	return std::to_string(keyCode);
}
