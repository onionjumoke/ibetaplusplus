#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// sdlcompat — the exact slice of SDL2/SDL_net that BetaPlusPlus's portable
// code touches, reimplemented on top of UIKit-era iOS APIs:
//
//   * ticks/delay/basepath        → mach_absolute_time / NSHomeDirectory
//   * clipboard                   → UIPasteboard
//   * SDL_OpenURL                 → UIApplication openURL (posted to main)
//   * SDL_ShowSimpleMessageBox    → UIAlertView on the main thread
//   * mouse state                 → lwjgl input layer (see lwjgl/Mouse.cpp)
//   * SDL_Surface + Convert       → tiny RGBA surface struct (only used for
//                                    texture-pack thumbnails; default is null)
//   * WAV loading + audio CVT     → built-in RIFF parser + format converter
//   * audio device                → AudioUnit RemoteIO output driving the
//                                    game's own mixer callback
//   * SDLNet_*                    → BSD sockets (TCP client only)
//   * GL extension probe          → glGetString(GL_EXTENSIONS) substring scan
//
// Header shims live next to this file (SDL.h, SDL_events.h, ..., SDL_net.h,
// SDL2/SDL.h, glad/glad.h) and are picked up through include-path precedence;
// no shared game or platform header had to change.
// ─────────────────────────────────────────────────────────────────────────────

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t Uint8;
typedef uint16_t Uint16;
typedef uint32_t Uint32;
typedef int8_t Sint8;
typedef int16_t Sint16;
typedef int32_t Sint32;

typedef int SDL_bool;
#define SDL_TRUE 1
#define SDL_FALSE 0

// ── Init / error ─────────────────────────────────────────────────────────────
// Flag values mirror SDL2 but only matter for logging.
enum
{
	SDL_INIT_TIMER = 0x00000001u,
	SDL_INIT_AUDIO = 0x00000010u,
	SDL_INIT_VIDEO = 0x00000020u,
	SDL_INIT_EVENTS = 0x00004000u
};

int SDL_Init(Uint32 flags);
const char *SDL_GetError(void);

// ── Time ─────────────────────────────────────────────────────────────────────
Uint32 SDL_GetTicks(void);
void SDL_Delay(Uint32 ms);

// ── Paths / misc ─────────────────────────────────────────────────────────────
char *SDL_GetBasePath(void); // app sandbox root, trailing separator, malloc'd
void SDL_free(void *p);

// Clipboard (returns malloc'd UTF-8, caller frees via SDL_free)
char *SDL_GetClipboardText(void);

// Opens in Mobile Safari; safe to call from any thread.
int SDL_OpenURL(const char *url);

// Crash reporting: logs loudly + shows a UIAlertView on the main thread and
// blocks until dismissed. window parameter ignored.
enum
{
	SDL_MESSAGEBOX_ERROR = 0x00000010u
};
int SDL_ShowSimpleMessageBox(Uint32 flags, const char *title, const char *message, void *window);

// ── Mouse state ──────────────────────────────────────────────────────────────
// Backed by the lwjgl touch layer so EntityRenderer's scaled-cursor maths see
// the same position the GUI does. Coordinates are in SDL-space (origin top-left).
#define SDL_BUTTON_LEFT 1
#define SDL_BUTTON_MIDDLE 2
#define SDL_BUTTON_RIGHT 3
#define SDL_BUTTON(X) (1 << ((X) - 1))

Uint32 SDL_GetMouseState(int *x, int *y);

// ── Window / context handles ────────────────────────────────────────────────
// Opaque on iOS; the real objects live behind lwjgl::GLContext::detail.
// SDL_Window is a tagged struct so game headers' `struct SDL_Window;` forward
// declarations stay compatible.
typedef struct SDL_Window SDL_Window;
typedef void *SDL_GLContext;

SDL_bool SDL_GL_ExtensionSupported(const char *extension);

// ── Events ───────────────────────────────────────────────────────────────────
// Only the subset the iOS input layer pushes through lwjgl::Mouse /
// lwjgl::Keyboard is modelled. Field names mirror SDL2 so the PC sources read
// as the reference implementation.
enum
{
	SDL_QUIT = 0x100,
	SDL_WINDOWEVENT = 0x200,
	SDL_KEYDOWN = 0x300,
	SDL_KEYUP = 0x301,
	SDL_TEXTINPUT = 0x303,
	SDL_MOUSEMOTION = 0x400,
	SDL_MOUSEBUTTONDOWN = 0x401,
	SDL_MOUSEBUTTONUP = 0x402,
	SDL_MOUSEWHEEL = 0x403
};

#define SDL_PRESSED 1
#define SDL_RELEASED 0

typedef struct SDL_Keysym
{
	int scancode;
	int sym;
	Uint16 mod;
	Uint32 unicode;
} SDL_Keysym;

typedef struct SDL_Event
{
	Uint32 type;
	union
	{
		struct
		{
			Sint32 x, y;
			Sint32 xrel, yrel;
		} motion;

		struct
		{
			Uint8 button;
			Uint8 state;
			Sint32 x, y;
		} button;

		struct
		{
			Sint32 x, y;
		} wheel;

		struct
		{
			Uint8 state;
			Uint8 repeat;
			SDL_Keysym keysym;
		} key;

		struct
		{
			char text[64];
		} text;
	};
} SDL_Event;

// ── Surfaces (texture-pack thumbnails only) ─────────────────────────────────
enum
{
	SDL_PIXELFORMAT_RGBA32 = 0x16462004u
};

typedef struct SDL_Surface
{
	int w, h;
	int pitch;
	Uint32 format;
	void *pixels;
} SDL_Surface;

SDL_Surface *SDL_ConvertSurfaceFormat(SDL_Surface *surface, Uint32 pixel_format, Uint32 flags);
void SDL_FreeSurface(SDL_Surface *surface);

// ── Audio formats ────────────────────────────────────────────────────────────
enum
{
	AUDIO_U8 = 0x0008,
	AUDIO_S8 = 0x8008,
	AUDIO_S16LSB = 0x8010,
	AUDIO_S16MSB = 0x9010,
	AUDIO_S32LSB = 0x8020,
	AUDIO_S32MSB = 0x9020,
	AUDIO_F32LSB = 0x8120,
	AUDIO_F32MSB = 0x9120
};
#if defined(__LITTLE_ENDIAN__) || defined(__i386__) || defined(__arm__)
#define AUDIO_S16SYS AUDIO_S16LSB
#define AUDIO_S32SYS AUDIO_S32LSB
#define AUDIO_F32SYS AUDIO_F32LSB
#else
#define AUDIO_S16SYS AUDIO_S16MSB
#define AUDIO_S32SYS AUDIO_S32MSB
#define AUDIO_F32SYS AUDIO_F32MSB
#endif

// ── Audio device (AudioUnit RemoteIO) ────────────────────────────────────────
typedef void (*SDL_AudioCallback)(void *userdata, Uint8 *stream, int len);

typedef struct SDL_AudioSpec
{
	int freq;
	int format; // one of the AUDIO_* constants
	Uint8 channels;
	Uint8 silence;
	Uint16 samples; // frames per callback
	Uint32 size;    // bytes per callback buffer
	SDL_AudioCallback callback;
	void *userdata;
} SDL_AudioSpec;

typedef Uint32 SDL_AudioDeviceID;

// Opens the RemoteIO output unit running `want->callback` from a realtime
// audio thread. Only Float32 stereo at the requested rate is supported for
// playback; `have` receives the effective spec.
SDL_AudioDeviceID SDL_OpenAudioDevice(const char *device, int iscapture,
                                      const SDL_AudioSpec *want, SDL_AudioSpec *have,
                                      int allowed_changes);
void SDL_PauseAudioDevice(SDL_AudioDeviceID dev, int pause_on);
void SDL_CloseAudioDevice(SDL_AudioDeviceID dev);

// ── WAV loading + conversion ─────────────────────────────────────────────────
typedef struct SDL_AudioCVT
{
	int needed;
	int src_format;
	int dst_format;
	double rate_incr;
	Uint8 channels_in;
	Uint8 channels_out;
	int freq_in;
	int freq_out;
	int len_mult; // buffer must be len * len_mult bytes
	int len_cvt;  // converted length after SDL_ConvertAudio
	int len;
	Uint8 *buf;
} SDL_AudioCVT;

// Loads PCM WAV (RIFF). Returns the sample buffer (malloc'd; free with
// SDL_FreeWAV) and fills spec; returns NULL on failure.
Uint8 *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec, Uint8 **audio_buf, Uint32 *audio_len);
void SDL_FreeWAV(Uint8 *audio_buf);

int SDL_BuildAudioCVT(SDL_AudioCVT *cvt,
                      int src_format, Uint8 src_channels, int src_rate,
                      int dst_format, Uint8 dst_channels, int dst_rate);
int SDL_ConvertAudio(SDL_AudioCVT *cvt);

#ifdef __cplusplus
} // extern "C"
#endif
