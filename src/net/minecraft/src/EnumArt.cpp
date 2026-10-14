#include "EnumArt.h"

const EnumArt EnumArt::Kebab        (String::fromUTF8("Kebab"),         16, 16,   0,   0);
const EnumArt EnumArt::Aztec        (String::fromUTF8("Aztec"),         16, 16,  16,   0);
const EnumArt EnumArt::Alban        (String::fromUTF8("Alban"),         16, 16,  32,   0);
const EnumArt EnumArt::Aztec2       (String::fromUTF8("Aztec2"),        16, 16,  48,   0);
const EnumArt EnumArt::Bomb         (String::fromUTF8("Bomb"),          16, 16,  64,   0);
const EnumArt EnumArt::Plant        (String::fromUTF8("Plant"),         16, 16,  80,   0);
const EnumArt EnumArt::Wasteland    (String::fromUTF8("Wasteland"),     16, 16,  96,   0);
const EnumArt EnumArt::Pool         (String::fromUTF8("Pool"),          32, 16,   0,  32);
const EnumArt EnumArt::Courbet      (String::fromUTF8("Courbet"),       32, 16,  32,  32);
const EnumArt EnumArt::Sea          (String::fromUTF8("Sea"),           32, 16,  64,  32);
const EnumArt EnumArt::Sunset       (String::fromUTF8("Sunset"),        32, 16,  96,  32);
const EnumArt EnumArt::Creebet      (String::fromUTF8("Creebet"),       32, 16, 128,  32);
const EnumArt EnumArt::Wanderer     (String::fromUTF8("Wanderer"),      16, 32,   0,  64);
const EnumArt EnumArt::Graham       (String::fromUTF8("Graham"),        16, 32,  16,  64);
const EnumArt EnumArt::Match        (String::fromUTF8("Match"),         32, 32,   0, 128);
const EnumArt EnumArt::Bust         (String::fromUTF8("Bust"),          32, 32,  32, 128);
const EnumArt EnumArt::Stage        (String::fromUTF8("Stage"),         32, 32,  64, 128);
const EnumArt EnumArt::Void         (String::fromUTF8("Void"),          32, 32,  96, 128);
const EnumArt EnumArt::SkullAndRoses(String::fromUTF8("SkullAndRoses"), 32, 32, 128, 128);
const EnumArt EnumArt::Fighters     (String::fromUTF8("Fighters"),      64, 32,   0,  96);
const EnumArt EnumArt::Pointer      (String::fromUTF8("Pointer"),       64, 64,   0, 192);
const EnumArt EnumArt::Pigscene     (String::fromUTF8("Pigscene"),      64, 64,  64, 192);
const EnumArt EnumArt::BurningSkull (String::fromUTF8("BurningSkull"),  64, 64, 128, 192);
const EnumArt EnumArt::Skeleton     (String::fromUTF8("Skeleton"),      64, 48, 192,  64);
const EnumArt EnumArt::DonkeyKong   (String::fromUTF8("DonkeyKong"),    64, 48, 192, 112);

const EnumArt *EnumArt::values[25] = {
	&EnumArt::Kebab,    &EnumArt::Aztec,    &EnumArt::Alban,   &EnumArt::Aztec2,
	&EnumArt::Bomb,     &EnumArt::Plant,    &EnumArt::Wasteland, &EnumArt::Pool,
	&EnumArt::Courbet,  &EnumArt::Sea,      &EnumArt::Sunset,  &EnumArt::Creebet,
	&EnumArt::Wanderer, &EnumArt::Graham,   &EnumArt::Match,   &EnumArt::Bust,
	&EnumArt::Stage,    &EnumArt::Void,     &EnumArt::SkullAndRoses, &EnumArt::Fighters,
	&EnumArt::Pointer,  &EnumArt::Pigscene, &EnumArt::BurningSkull, &EnumArt::Skeleton,
	&EnumArt::DonkeyKong
};

const int_t EnumArt::maxArtTitleLength = 13; // "SkullAndRoses".length()
