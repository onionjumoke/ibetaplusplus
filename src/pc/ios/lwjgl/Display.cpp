// ─────────────────────────────────────────────────────────────────────────────
// lwjgl/Display.cpp (iOS) — the game is permanently fullscreen; "display
// mode" is just the current drawable size. Events are pushed by the UIKit
// touch layer directly into the Mouse/Keyboard queues, so processMessages()
// only refreshes sizes.
// ─────────────────────────────────────────────────────────────────────────────

#include "lwjgl/Display.h"

#include "lwjgl/GLContext.h"
#include "IosBridge.h"

namespace lwjgl
{
namespace Display
{

static DisplayMode g_mode(480, 320);

// Display functions
void setDisplayMode(const DisplayMode &display_mode)
{
	// Windowed mode does not exist on iOS; adopt the requested size as the
	// logical size but keep the surface authoritative.
	g_mode = DisplayMode(iosapp::surfaceWidth(), iosapp::surfaceHeight());
	(void)display_mode;
}

DisplayMode getDisplayMode()
{
	return g_mode;
}

void setTitle(const jstring &string)
{
	(void)string; // no window title on iOS
}

void setFullscreen(bool fullscreen)
{
	(void)fullscreen; // always fullscreen
	g_mode = DisplayMode(iosapp::surfaceWidth(), iosapp::surfaceHeight());
}

bool isCloseRequested()
{
	return iosapp::isQuitRequested();
}

bool isVisible()
{
	return true; // backgrounding suspends the process entirely
}

bool isActive()
{
	return iosapp::isAppActive();
}

void processMessages()
{
	// Refresh the logical size if the surface changed (rotation is disabled,
	// so in practice this only fires once after launch).
	int w = iosapp::surfaceWidth();
	int h = iosapp::surfaceHeight();
	if (w != g_mode.getWidth() || h != g_mode.getHeight())
		g_mode = DisplayMode(w, h);
}

void swapBuffers()
{
	iosapp::presentFrame();
}

void update(bool doProcessMessages)
{
	swapBuffers();
	if (doProcessMessages)
		processMessages();
}

void create()
{
	g_mode = DisplayMode(iosapp::surfaceWidth(), iosapp::surfaceHeight());
}

int_t getX()
{
	return 0;
}

int_t getY()
{
	return 0;
}

int_t getWidth()
{
	return g_mode.getWidth();
}

int_t getHeight()
{
	return g_mode.getHeight();
}

}
}
