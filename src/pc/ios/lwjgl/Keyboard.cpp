// ─────────────────────────────────────────────────────────────────────────────
// lwjgl/Keyboard.cpp (iOS) — virtual key/char events from the touch layer and
// the text-input field, plus a software key-state array for isKeyDown().
//
// Event pairing logic (a char event attaches to its preceding key event) is
// kept identical to the SDL build so GUI code behaves the same.
// ─────────────────────────────────────────────────────────────────────────────

#include "lwjgl/Keyboard.h"

#include <cstdio>
#include <mutex>
#include <queue>

#include "IosInput.h"

namespace lwjgl
{
namespace Keyboard
{

namespace
{
std::recursive_mutex &stateMutex()
{
	static std::recursive_mutex m;
	return m;
}
} // namespace

namespace detail
{

struct Event
{
	int_t key;
	int_t character;
	bool repeat;
	bool down;
};

static Event g_eventCurrent = {};
static std::queue<Event> g_eventQueue;

static bool g_hasRetainedEvent = false;
static Event g_eventRetained = {};

static const int kKeyStateSize = 256;
static unsigned char g_keyState[kKeyStateSize] = {};

void flushRetained()
{
	if (!g_hasRetainedEvent)
		return;

	g_hasRetainedEvent = false;
	g_eventQueue.push(g_eventRetained);
}

void handleKey(int_t key, bool repeat, bool down)
{
	if (key == 0x39) // SPACE: stuck-key forensics
		fprintf(stderr, "Beta++: SPACE %s\n", down ? "DOWN" : "UP");
	flushRetained();
	g_hasRetainedEvent = true;

	g_eventRetained.key = key;
	g_eventRetained.character = 0;
	g_eventRetained.repeat = repeat;
	g_eventRetained.down = down;

	if (key >= 0 && key < kKeyStateSize)
		g_keyState[key] = down ? 1 : 0;
}

void handleCharacter(int_t character)
{
	if (g_hasRetainedEvent && g_eventRetained.character != 0)
		flushRetained();
	if (!g_hasRetainedEvent)
		g_eventQueue.push(Event{KEY_NONE, character, false, true});
	else
		g_eventRetained.character = character;
}

// ── IosInput hooks (main thread producers) ───────────────────────────────────

void pushLwjglKey(int key, bool down)
{
	std::lock_guard<std::recursive_mutex> lock(stateMutex());
	handleKey(static_cast<int_t>(key), false, down);
}

void pushUnicodeChar(char32_t c)
{
	std::lock_guard<std::recursive_mutex> lock(stateMutex());
	handleCharacter(static_cast<int_t>(c));
}

} // namespace detail

// Keyboard functions
jstring getKeyName(int_t key)
{
	// LWJGL scancode naming for the subset the options menu can display.
	switch (key)
	{
		case KEY_ESCAPE: return u"ESC";
		case KEY_RETURN: return u"RETURN";
		case KEY_SPACE: return u"SPACE";
		case KEY_BACK: return u"BACK";
		case KEY_TAB: return u"TAB";
		case KEY_LSHIFT: return u"LSHIFT";
		case KEY_RSHIFT: return u"RSHIFT";
		case KEY_LCONTROL: return u"LCTRL";
		case KEY_RCONTROL: return u"RCTRL";
		case KEY_UP: return u"UP";
		case KEY_DOWN: return u"DOWN";
		case KEY_LEFT: return u"LEFT";
		case KEY_RIGHT: return u"RIGHT";
		default: break;
	}

	if (key >= 0x02 && key <= 0x0B)
	{
		static const char *kTopRow[10] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"};
		return String::fromUTF8(kTopRow[key - 0x02]);
	}
	if (key >= 0x10 && key <= 0x19)
	{
		static const char *kQwertyRow1[10] = {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"};
		return String::fromUTF8(kQwertyRow1[key - 0x10]);
	}
	if (key >= 0x1E && key <= 0x26)
	{
		static const char *kQwertyRow2[9] = {"A", "S", "D", "F", "G", "H", "J", "K", "L"};
		return String::fromUTF8(kQwertyRow2[key - 0x1E]);
	}
	if (key >= 0x2C && key <= 0x32)
	{
		static const char *kQwertyRow3[7] = {"Z", "X", "C", "V", "B", "N", "M"};
		return String::fromUTF8(kQwertyRow3[key - 0x2C]);
	}
	if (key == 0x33) return u"COMMA";
	if (key == 0x34) return u"PERIOD";
	if (key == 0x35) return u"SLASH";

	char fallback[16];
	snprintf(fallback, sizeof(fallback), "KEY 0x%02X", static_cast<unsigned>(key));
	return String::fromUTF8(fallback);
}

// Event handling
static bool g_allowRepeatEvents = false;

bool next()
{
	std::lock_guard<std::recursive_mutex> lock(stateMutex());

	detail::flushRetained();

	if (detail::g_eventQueue.empty())
		return false;

	if (!g_allowRepeatEvents)
	{
		while (!detail::g_eventQueue.empty() && detail::g_eventQueue.front().repeat)
			detail::g_eventQueue.pop();
	}

	if (detail::g_eventQueue.empty())
		return false;

	detail::g_eventCurrent = detail::g_eventQueue.front();
	detail::g_eventQueue.pop();
	return true;
}

void enableRepeatEvents(bool repeat)
{
	g_allowRepeatEvents = repeat;
}

bool areRepeatEventsEnabled()
{
	return g_allowRepeatEvents;
}

char_t getEventCharacter()
{
	return static_cast<char_t>(detail::g_eventCurrent.character);
}

int_t getEventKey()
{
	return detail::g_eventCurrent.key;
}

bool getEventKeyState()
{
	return detail::g_eventCurrent.down;
}

// Polling
bool isKeyDown(int_t key)
{
	std::lock_guard<std::recursive_mutex> lock(stateMutex());
	if (key < 0 || key >= detail::kKeyStateSize)
		return false;
	return detail::g_keyState[key] != 0;
}

}
}
