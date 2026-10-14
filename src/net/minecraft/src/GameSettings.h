#pragma once

#include <string>
#include <vector>
#include "java/Type.h"

class EnumOptions;
class KeyBinding;
class Minecraft;

// net.minecraft.src.GameSettings
class GameSettings
{
public:
	GameSettings(Minecraft *minecraft, const std::string &file);
	GameSettings();

	std::string getKeyBindingDescription(int_t i);
	std::string getOptionDisplayString(int_t i);
	void setKeyBinding(int_t i, int_t j);
	void setOptionFloatValue(const EnumOptions *enumoptions, float f);
	void setOptionFloatValue(const EnumOptions &enumoptions, float f) { setOptionFloatValue(&enumoptions, f); }
	void setOptionValue(const EnumOptions *enumoptions, int_t i);
	void setOptionValue(const EnumOptions &enumoptions, int_t i) { setOptionValue(&enumoptions, i); }
	float getOptionFloatValue(const EnumOptions *enumoptions);
	float getOptionFloatValue(const EnumOptions &enumoptions) { return getOptionFloatValue(&enumoptions); }
	bool getOptionOrdinalValue(const EnumOptions *enumoptions);
	bool getOptionOrdinalValue(const EnumOptions &enumoptions) { return getOptionOrdinalValue(&enumoptions); }
	std::string getKeyBinding(const EnumOptions *enumoptions);
	std::string getKeyBinding(const EnumOptions &enumoptions) { return getKeyBinding(&enumoptions); }
	void loadOptions();
	void saveOptions();

private:
	void setDefaults();
	// Rebuild every chunk so options baked into the chunk display lists
	// (smooth lighting / AO, graphics quality, tree/grass detail, view distance)
	// take effect on already-compiled chunks instead of only newly built ones.
	void reloadChunkRenderers();
	// OptiFine: reaplica el brillo (ofBrightness) a la lightBrightnessTable del mundo.
	void updateWorldLightLevels();
	float parseFloat(const std::string &s);
	std::string translateKey(const std::string &s);
	std::string translateKey(const std::u16string &s);
	static std::string keyName(int_t keyCode);

	static const char *RENDER_DISTANCES[4];
	static const char *DIFFICULTIES[4];
	static const char *GUISCALES[4];
	static const char *LIMIT_FRAMERATES[3];

public:
	float musicVolume;
	float soundVolume;
	float mouseSensitivity;
	bool invertMouse;
	int_t renderDistance;
	bool viewBobbing;
	bool anaglyph;
	bool advancedOpengl;
	int_t limitFramerate;
	bool fancyGraphics;
	bool ambientOcclusion;
	std::string skin;
	KeyBinding *keyBindForward;
	KeyBinding *keyBindLeft;
	KeyBinding *keyBindBack;
	KeyBinding *keyBindRight;
	KeyBinding *keyBindJump;
	KeyBinding *keyBindInventory;
	KeyBinding *keyBindDrop;
	KeyBinding *keyBindChat;
	KeyBinding *keyBindToggleFog;
	KeyBinding *keyBindSneak;
	std::vector<KeyBinding *> keyBindings;
	Minecraft *mc;
	std::string optionsFile;
	int_t difficulty;
	bool hideGUI;
	bool thirdPersonView;
	bool showDebugInfo;
	std::string lastServer;
	bool field_22275_C;
	bool smoothCamera;
	bool field_22273_E;
	float field_22272_F;
	float field_22271_G;
	int_t guiScale;

	// --- OptiFine (mipmaps excluidos: sin ofMipmapLevel/ofMipmapLinear) ---
	bool ofFogFancy;          // Fog: false=Fast, true=Fancy (requiere GL_NV_fog_distance)
	bool ofFogOff;            // Fog: true = niebla de distancia deshabilitada (extension del port)
	float ofFogStart;         // 0.2 .. 0.8
	bool ofLoadFar;           // cargar chunks lejanos
	int_t ofPreloadedChunks;  // 0..8
	bool ofOcclusionFancy;    // occlusion culling fancy (campo 'h' = advancedOpengl = occlusion on/off)
	bool ofSmoothFps;
	bool ofSmoothInput;
	float ofBrightness;       // 0..1
	float ofAoLevel;          // Smooth Lighting 0..1
	int_t ofClouds;           // 0=Default,1=Fast,2=Fancy,3=OFF
	float ofCloudsHeight;     // 0..1
	int_t ofTrees;            // 0=Default,1=Fast,2=Fancy
	int_t ofGrass;            // 0=Default,1=Fast,2=Fancy
	int_t ofRain;             // 0=Default,1=Fast,2=Fancy,3=OFF
	int_t ofWater;            // 0=Default,1=Fast,2=Fancy,3=OFF
	int_t ofBetterGrass;      // 1=Fast,2=Fancy,3=OFF
	int_t ofAutoSaveTicks;    // 40..40000
	bool ofFastDebugInfo;
	bool ofWeather;
	bool ofCaveSounds;
	// Performance / QoL toggles.
	float ofFov;        // field of view in degrees (30..110)
	bool ofMobs;        // natural + worldgen mob spawning
	bool ofParticles;   // ambient random-display particles
	bool ofShowChunkBorders; // F3+G chunk grid overlay
	bool ofSky;
	bool ofStars;
	int_t ofChunkUpdates;     // 1..5
	bool ofChunkUpdatesDynamic;
	bool ofFarView;
	int_t ofTime;             // 0=Default,1=Day,2=Night
	bool ofClearWater;
	int_t ofAnimatedWater;    // 0=ON,1=Dynamic,2=OFF
	int_t ofAnimatedLava;     // 0=ON,1=Dynamic,2=OFF
	bool ofAnimatedFire;
	bool ofAnimatedPortal;
	bool ofAnimatedRedstone;
	bool ofAnimatedExplosion;
	bool ofAnimatedFlame;
	bool ofAnimatedSmoke;
	KeyBinding *ofKeyBindZoom;
};
