// ─────────────────────────────────────────────────────────────────────────────
// lwjgl/Mouse.cpp (iOS) — touch events land here from the UIKit layer via the
// IosInput hooks. Screen coordinates (top-left origin) are converted to the
// GL convention the game expects (bottom-left origin) exactly like the SDL
// build does inside its own pushEvent.
//
// Unlike the PC build, producers run on the main thread and consumers on the
// game thread, so the queue and state carry a mutex.
// ─────────────────────────────────────────────────────────────────────────────

#include "lwjgl/Mouse.h"

#include <mutex>
#include <queue>

#include "lwjgl/Display.h"
#include "IosInput.h"
#include "IosBridge.h"

namespace lwjgl
{
namespace Mouse
{

namespace detail
{

struct Event
{
	Sint8 button, down;
	Sint32 x, y;
	Sint32 xrel, yrel;
	Sint32 wheel;

	Event(Sint8 button = 0, Sint8 down = 0, Sint32 x = 0, Sint32 y = 0,
	      Sint32 xrel = 0, Sint32 yrel = 0, Sint32 wheel = 0)
		: button(button), down(down), x(x), y(y), xrel(xrel), yrel(yrel), wheel(wheel)
	{ }
};

static Event g_eventCurrent;
static std::queue<Event> g_eventQueue;

// Staging deltas accumulate across motion events and are drained once per
// frame by MouseHelper::mouseXYChange() via getDX()/getDY().
static int_t g_stagingDX = 0;
static int_t g_stagingDY = 0;

} // namespace detail

// ── Shared state ─────────────────────────────────────────────────────────────

namespace
{
std::recursive_mutex &stateMutex()
{
	static std::recursive_mutex m;
	return m;
}

int g_posX = 0, g_posY = 0;                 // screen coords, top-left origin
bool g_buttons[3] = {false, false, false};  // LWJGL order L/R/M
bool g_grabbed = false;
}

// ── IosInput hooks (main thread producers) ───────────────────────────────────

namespace detail
{

void pushScreenMotion(int x, int y, int xrel, int yrel)
{
	std::lock_guard<std::recursive_mutex> lock(stateMutex());
	g_posX = x;
	g_posY = y;
	g_stagingDX += static_cast<int_t>(xrel);
	g_stagingDY -= static_cast<int_t>(yrel);

	const int_t height = lwjgl::Display::getHeight();
	g_eventQueue.emplace(-1, 0,
	                     static_cast<Sint32>(x),
	                     static_cast<Sint32>(height - y - 1),
	                     static_cast<Sint32>(xrel),
	                     static_cast<Sint32>(-yrel),
	                     0);
}

void pushScreenButton(int button, bool down, int x, int y)
{
	fprintf(stderr, "Beta++: MB%d %s @%d,%d\n", button, down ? "DOWN":"UP", x, y);

	std::lock_guard<std::recursive_mutex> lock(stateMutex());
	if (button >= 0 && button < 3)
		g_buttons[button] = down;

	g_posX = x;
	g_posY = y;

	const int_t height = lwjgl::Display::getHeight();
	g_eventQueue.emplace(static_cast<Sint8>(button), down ? 1 : 0,
	                     static_cast<Sint32>(x),
	                     static_cast<Sint32>(height - y - 1),
	                     0, 0, 0);
}

void pushWheelDelta(int dz)
{
	std::lock_guard<std::recursive_mutex> lock(stateMutex());
	g_eventQueue.emplace(-1, 0, static_cast<Sint32>(g_posX),
	                     static_cast<Sint32>(lwjgl::Display::getHeight() - g_posY - 1),
	                     0, 0, static_cast<Sint32>(dz));
}

} // namespace detail

void setCursorPosition(int_t x, int_t y)
{
	std::lock_guard<std::recursive_mutex> lock(stateMutex());
	g_posX = static_cast<int>(x);
	g_posY = static_cast<int>(y);
}

// Event handling (game thread consumer)
bool next()
{
	std::lock_guard<std::recursive_mutex> lock(stateMutex());
	if (detail::g_eventQueue.empty())
		return false;
	detail::g_eventCurrent = detail::g_eventQueue.front();
	detail::g_eventQueue.pop();
	return true;
}

int_t getEventButton()
{
	return detail::g_eventCurrent.button;
}

bool getEventButtonState()
{
	return detail::g_eventCurrent.down != 0;
}

int_t getEventDX()
{
	return detail::g_eventCurrent.xrel;
}

int_t getEventDY()
{
	return detail::g_eventCurrent.yrel;
}

int_t getEventX()
{
	return detail::g_eventCurrent.x;
}

int_t getEventY()
{
	return detail::g_eventCurrent.y;
}

int_t getEventDWheel()
{
	return detail::g_eventCurrent.wheel;
}

// State
int_t getX()
{
	std::lock_guard<std::recursive_mutex> lock(stateMutex());
	return g_posX;
}

int_t getY()
{
	std::lock_guard<std::recursive_mutex> lock(stateMutex());
	return lwjgl::Display::getHeight() - g_posY - 1;
}

int_t getDX()
{
	std::lock_guard<std::recursive_mutex> lock(stateMutex());
	int_t result = detail::g_stagingDX;
	detail::g_stagingDX = 0;
	return result;
}

int_t getDY()
{
	std::lock_guard<std::recursive_mutex> lock(stateMutex());
	int_t result = detail::g_stagingDY;
	detail::g_stagingDY = 0;
	return result;
}

int_t getDWheel()
{
	return 0; // wheel arrives per-event via getEventDWheel(), as with SDL
}

bool isButtonDown(int_t button)
{
	std::lock_guard<std::recursive_mutex> lock(stateMutex());
	return button >= 0 && button < 3 && g_buttons[button];
}

bool isGrabbed()
{
	std::lock_guard<std::recursive_mutex> lock(stateMutex());
	return g_grabbed;
}

void setGrabbed(bool grabbed)
{
	std::lock_guard<std::recursive_mutex> lock(stateMutex());
	g_grabbed = grabbed;
	detail::g_stagingDX = 0;
	detail::g_stagingDY = 0;
}

}
}
