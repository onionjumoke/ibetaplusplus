#include "lwjgl/Display.h"

#include <cstring>
#include <stdexcept>
#include <vector>

#include "lwjgl/GLContext.h"
#include "lwjgl/Mouse.h"
#include "lwjgl/Keyboard.h"

#include "terminal/Terminal.h"
#include "terminal/OffscreenGL.h"

#include <glad/glad.h>

namespace lwjgl
{
namespace Display
{

static bool close_requested = false;
static DisplayMode current_display_mode(854, 480);
static bool fullscreen_state = false;

void setDisplayMode(const DisplayMode &display_mode)
{
	// The render resolution is fixed by the terminal backend; window sizing
	// requests are recorded but the image is scaled by the terminal itself.
	current_display_mode = display_mode;
}

DisplayMode getDisplayMode()
{
	return current_display_mode;
}

void setTitle(const jstring &)
{
	// nothing to do without a window system
}

void setFullscreen(bool fullscreen)
{
	// always "fullscreen" — the frame fills the terminal
	fullscreen_state = fullscreen;
}

bool isCloseRequested()
{
	return close_requested || terminal::isCloseRequested();
}

bool isVisible()
{
	return true;
}

bool isActive()
{
	return true; // a focused terminal is our whole world
}

void processMessages()
{
	terminal::pump();
}

void swapBuffers()
{
	const int w = terminal::renderWidth();
	const int h = terminal::renderHeight();

	if (::strcmp(terminal::activeOffscreen().name(), "osmesa") == 0)
	{
		size_t stride = 0;
		if (void *pixels = terminal::activeOffscreen().frameBuffer(stride))
		{
			terminal::sendFrame(static_cast<const std::uint8_t *>(pixels), w, h);
			return;
		}
	}

	// EGL path: read the pbuffer back to CPU memory.
	static std::vector<std::uint8_t> pixels;
	pixels.resize(static_cast<size_t>(w) * h * 4);
	glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
	terminal::sendFrame(pixels.data(), w, h);
}

void update(bool doProcessMessages)
{
	swapBuffers();
	if (doProcessMessages)
		processMessages();
}

void create()
{
	// everything already set up during GLContext::instantiate()
}

int_t getX() { return 0; }
int_t getY() { return 0; }

int_t getWidth() { return current_display_mode.getWidth(); }
int_t getHeight() { return current_display_mode.getHeight(); }

} // namespace Display
} // namespace lwjgl
