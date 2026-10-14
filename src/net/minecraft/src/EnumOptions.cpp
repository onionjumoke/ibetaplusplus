#include "EnumOptions.h"

EnumOptions *EnumOptions::MUSIC = new EnumOptions(0, u"options.music", true, false);
EnumOptions *EnumOptions::SOUND = new EnumOptions(1, u"options.sound", true, false);
EnumOptions *EnumOptions::INVERT_MOUSE = new EnumOptions(2, u"options.invertMouse", false, true);
EnumOptions *EnumOptions::SENSITIVITY = new EnumOptions(3, u"options.sensitivity", true, false);
EnumOptions *EnumOptions::RENDER_DISTANCE = new EnumOptions(4, u"options.renderDistance", false, false);
EnumOptions *EnumOptions::VIEW_BOBBING = new EnumOptions(5, u"options.viewBobbing", false, true);
EnumOptions *EnumOptions::ANAGLYPH = new EnumOptions(6, u"options.anaglyph", false, true);
EnumOptions *EnumOptions::ADVANCED_OPENGL = new EnumOptions(7, u"options.advancedOpengl", false, true);
EnumOptions *EnumOptions::FRAMERATE_LIMIT = new EnumOptions(8, u"options.framerateLimit", false, false);
EnumOptions *EnumOptions::DIFFICULTY = new EnumOptions(9, u"options.difficulty", false, false);
EnumOptions *EnumOptions::GRAPHICS = new EnumOptions(10, u"options.graphics", false, false);
EnumOptions *EnumOptions::AMBIENT_OCCLUSION = new EnumOptions(11, u"options.ao", false, true);
EnumOptions *EnumOptions::GUI_SCALE = new EnumOptions(12, u"options.guiScale", false, false);

// --- OptiFine (mipmaps excluidos: sin MIPMAP_LEVEL/MIPMAP_TYPE) ---
// Ordinales secuenciales por orden de declaracion (ht::c() == ordinal()).
EnumOptions *EnumOptions::FOG_FANCY = new EnumOptions(13, u"Fog", false, false);
EnumOptions *EnumOptions::FOG_START = new EnumOptions(14, u"Fog Start", false, false);
EnumOptions *EnumOptions::LOAD_FAR = new EnumOptions(15, u"Load Far", false, false);
EnumOptions *EnumOptions::PRELOADED_CHUNKS = new EnumOptions(16, u"Preloaded Chunks", false, false);
EnumOptions *EnumOptions::SMOOTH_FPS = new EnumOptions(17, u"Smooth FPS", false, false);
EnumOptions *EnumOptions::BRIGHTNESS = new EnumOptions(18, u"Brightness", true, false);
EnumOptions *EnumOptions::CLOUDS = new EnumOptions(19, u"Clouds", false, false);
EnumOptions *EnumOptions::CLOUD_HEIGHT = new EnumOptions(20, u"Cloud Height", true, false);
EnumOptions *EnumOptions::TREES = new EnumOptions(21, u"Trees", false, false);
EnumOptions *EnumOptions::GRASS = new EnumOptions(22, u"Grass", false, false);
EnumOptions *EnumOptions::RAIN = new EnumOptions(23, u"Rain & Snow", false, false);
EnumOptions *EnumOptions::WATER = new EnumOptions(24, u"Water", false, false);
EnumOptions *EnumOptions::ANIMATED_WATER = new EnumOptions(25, u"Water Animated", false, false);
EnumOptions *EnumOptions::ANIMATED_LAVA = new EnumOptions(26, u"Lava Animated", false, false);
EnumOptions *EnumOptions::ANIMATED_FIRE = new EnumOptions(27, u"Fire Animated", false, false);
EnumOptions *EnumOptions::ANIMATED_PORTAL = new EnumOptions(28, u"Portal Animated", false, false);
EnumOptions *EnumOptions::AO_LEVEL = new EnumOptions(29, u"Smooth Lighting", true, false);
EnumOptions *EnumOptions::FAST_DEBUG_INFO = new EnumOptions(30, u"Fast Debug Info", false, false);
EnumOptions *EnumOptions::AUTOSAVE_TICKS = new EnumOptions(31, u"Autosave", false, false);
EnumOptions *EnumOptions::BETTER_GRASS = new EnumOptions(32, u"Better Grass", false, false);
EnumOptions *EnumOptions::ANIMATED_REDSTONE = new EnumOptions(33, u"Redstone Animated", false, false);
EnumOptions *EnumOptions::ANIMATED_EXPLOSION = new EnumOptions(34, u"Explosion Animated", false, false);
EnumOptions *EnumOptions::ANIMATED_FLAME = new EnumOptions(35, u"Flame Animated", false, false);
EnumOptions *EnumOptions::ANIMATED_SMOKE = new EnumOptions(36, u"Smoke Animated", false, false);
EnumOptions *EnumOptions::WEATHER = new EnumOptions(37, u"Weather", false, false);
EnumOptions *EnumOptions::CAVE_SOUNDS = new EnumOptions(46, u"Cave Sounds", false, false);
EnumOptions *EnumOptions::MOBS = new EnumOptions(47, u"Mobs", false, false);
EnumOptions *EnumOptions::PARTICLES = new EnumOptions(48, u"Particles", false, false);
EnumOptions *EnumOptions::FOV = new EnumOptions(49, u"FOV", true, false);
EnumOptions *EnumOptions::SKY = new EnumOptions(38, u"Sky", false, false);
EnumOptions *EnumOptions::STARS = new EnumOptions(39, u"Stars", false, false);
EnumOptions *EnumOptions::FAR_VIEW = new EnumOptions(40, u"Far View", false, false);
EnumOptions *EnumOptions::CHUNK_UPDATES = new EnumOptions(41, u"Chunk Updates", false, false);
EnumOptions *EnumOptions::CHUNK_UPDATES_DYNAMIC = new EnumOptions(42, u"Dynamic Updates", false, false);
EnumOptions *EnumOptions::TIME = new EnumOptions(43, u"Time", false, false);
EnumOptions *EnumOptions::CLEAR_WATER = new EnumOptions(44, u"Clear Water", false, false);
EnumOptions *EnumOptions::SMOOTH_INPUT = new EnumOptions(45, u"Smooth Input", false, false);

static EnumOptions *s_allOptions[] = {
    EnumOptions::MUSIC, EnumOptions::SOUND, EnumOptions::INVERT_MOUSE,
    EnumOptions::SENSITIVITY, EnumOptions::RENDER_DISTANCE, EnumOptions::VIEW_BOBBING,
    EnumOptions::ANAGLYPH, EnumOptions::ADVANCED_OPENGL, EnumOptions::FRAMERATE_LIMIT,
    EnumOptions::DIFFICULTY, EnumOptions::GRAPHICS, EnumOptions::AMBIENT_OCCLUSION,
    EnumOptions::GUI_SCALE,
    EnumOptions::FOG_FANCY, EnumOptions::FOG_START, EnumOptions::LOAD_FAR,
    EnumOptions::PRELOADED_CHUNKS, EnumOptions::SMOOTH_FPS, EnumOptions::BRIGHTNESS,
    EnumOptions::CLOUDS, EnumOptions::CLOUD_HEIGHT, EnumOptions::TREES,
    EnumOptions::GRASS, EnumOptions::RAIN, EnumOptions::WATER,
    EnumOptions::ANIMATED_WATER, EnumOptions::ANIMATED_LAVA, EnumOptions::ANIMATED_FIRE,
    EnumOptions::ANIMATED_PORTAL, EnumOptions::AO_LEVEL, EnumOptions::FAST_DEBUG_INFO,
    EnumOptions::AUTOSAVE_TICKS, EnumOptions::BETTER_GRASS, EnumOptions::ANIMATED_REDSTONE,
    EnumOptions::ANIMATED_EXPLOSION, EnumOptions::ANIMATED_FLAME, EnumOptions::ANIMATED_SMOKE,
    EnumOptions::WEATHER, EnumOptions::SKY, EnumOptions::STARS,
    EnumOptions::FAR_VIEW, EnumOptions::CHUNK_UPDATES, EnumOptions::CHUNK_UPDATES_DYNAMIC,
    EnumOptions::TIME, EnumOptions::CLEAR_WATER, EnumOptions::SMOOTH_INPUT,
    EnumOptions::CAVE_SOUNDS, EnumOptions::MOBS, EnumOptions::PARTICLES, EnumOptions::FOV
};

EnumOptions *EnumOptions::getEnumOptions(int_t i)
{
    for (EnumOptions *opt : s_allOptions)
        if (opt->returnEnumOrdinal() == i)
            return opt;
    return nullptr;
}
