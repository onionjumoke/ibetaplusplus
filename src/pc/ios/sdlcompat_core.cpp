// sdlcompat_core.cpp — time, init, mouse state, GL extension probe and
// surface shims.
#include "sdlcompat/sdlcompat.h"

#include <OpenGLES/ES1/gl.h>

#include <mach/mach_time.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

// ── Init / error ─────────────────────────────────────────────────────────────

extern "C" int SDL_Init(Uint32 flags)
{
	(void)flags; // everything is initialised lazily by the platform layer
	return 0;
}

extern "C" const char *SDL_GetError(void)
{
	return "";
}

// Extension probe for Config's OptiFine toggles. ES 1.1 has neither
// GL_ARB_occlusion_query nor GL_NV_fog_distance, so this returns false and
// the game falls back to its vanilla paths.
extern "C" SDL_bool SDL_GL_ExtensionSupported(const char *extension)
{
	if (extension == nullptr)
		return SDL_FALSE;
	const GLubyte *extensions = glGetString(GL_EXTENSIONS);
	if (extensions == nullptr)
		return SDL_FALSE;
	const char *start = reinterpret_cast<const char *>(extensions);
	const size_t len = strlen(extension);
	const char *p = start;
	while ((p = strstr(p, extension)) != nullptr)
	{
		bool startOk = (p == start) || p[-1] == ' ';
		bool endOk = p[len] == '\0' || p[len] == ' ';
		if (startOk && endOk)
			return SDL_TRUE;
		p += len;
	}
	return SDL_FALSE;
}

// ── Time ─────────────────────────────────────────────────────────────────────

extern "C" Uint32 SDL_GetTicks(void)
{
	static mach_timebase_info_data_t tb;
	if (tb.denom == 0)
		mach_timebase_info(&tb);
	uint64_t nanos = mach_absolute_time() * tb.numer / tb.denom;
	return static_cast<Uint32>(nanos / 1000000ull);
}

extern "C" void SDL_Delay(Uint32 ms)
{
	usleep(static_cast<useconds_t>(ms) * 1000u);
}

// ── Mouse state ──────────────────────────────────────────────────────────────
// Routed through the lwjgl input layer so menus and EntityRenderer's scaled
// cursor agree with the touch handling.

#include "lwjgl/Mouse.h"
#include "lwjgl/Display.h"

extern "C" Uint32 SDL_GetMouseState(int *x, int *y)
{
	if (x)
		*x = static_cast<int>(lwjgl::Mouse::getX());
	if (y)
		*y = static_cast<int>(lwjgl::Display::getHeight()) - static_cast<int>(lwjgl::Mouse::getY()) - 1;

	Uint32 buttons = 0;
	if (lwjgl::Mouse::isButtonDown(0))
		buttons |= SDL_BUTTON(SDL_BUTTON_LEFT);
	if (lwjgl::Mouse::isButtonDown(1))
		buttons |= SDL_BUTTON(SDL_BUTTON_RIGHT);
	if (lwjgl::Mouse::isButtonDown(2))
		buttons |= SDL_BUTTON(SDL_BUTTON_MIDDLE);
	return buttons;
}

// ── Surfaces ─────────────────────────────────────────────────────────────────
// Only texture-pack thumbnails flow through here (the default pack passes
// nullptr). Treat incoming pixels as tightly packed RGBA8888. A NEW surface is
// always returned because RenderEngine frees the converted copy but not the
// original.

extern "C" SDL_Surface *SDL_ConvertSurfaceFormat(SDL_Surface *surface, Uint32 pixel_format, Uint32 flags)
{
	(void)flags;
	if (surface == nullptr || surface->pixels == nullptr || surface->w <= 0 || surface->h <= 0)
		return nullptr;

	SDL_Surface *out = static_cast<SDL_Surface *>(calloc(1, sizeof(SDL_Surface)));
	if (out == nullptr)
		return nullptr;

	out->w = surface->w;
	out->h = surface->h;
	out->pitch = surface->w * 4;
	out->format = pixel_format;
	out->pixels = malloc(static_cast<size_t>(out->pitch) * static_cast<size_t>(out->h));
	if (out->pixels == nullptr)
	{
		free(out);
		return nullptr;
	}

	const size_t srcPitch = surface->pitch > 0 ? static_cast<size_t>(surface->pitch) : static_cast<size_t>(surface->w) * 4;
	const uint8_t *src = static_cast<const uint8_t *>(surface->pixels);
	uint8_t *dst = static_cast<uint8_t *>(out->pixels);
	for (int row = 0; row < out->h; ++row)
		memcpy(dst + row * out->pitch, src + row * srcPitch, static_cast<size_t>(out->w) * 4);
	return out;
}

extern "C" void SDL_FreeSurface(SDL_Surface *surface)
{
	if (surface == nullptr)
		return;
	free(surface->pixels);
	free(surface);
}
