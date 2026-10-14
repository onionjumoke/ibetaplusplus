#include "Terminal.h"

#include <atomic>
#include <condition_variable>
#include <cstring>
#include <deque>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <chrono>

#include <zlib.h>

#include <SDL_events.h>

#include "lwjgl/Keyboard.h"
#include "lwjgl/Mouse.h"

namespace lwjgl
{
namespace terminal
{

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

static std::string envOr(const char *name, const std::string &fallback)
{
	const char *v = ::getenv(name);
	return (v != nullptr && *v != '\0') ? std::string(v) : fallback;
}

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

static int g_tty = -1;                 // control fd (/dev/tty), -1 when unused
static termios g_saved_termios;
static bool g_raw_mode = false;

static int g_render_w = 480;
static int g_render_h = 300;
static bool g_halfblock = false; // fallback renderer (no kitty graphics)

static std::atomic<bool> g_close_requested{false};
static std::atomic<bool> g_quit_thread{false};

// Mouse emulation state (guarded by g_state_mutex)
static std::mutex g_state_mutex;
static bool g_grabbed = false;
static float g_cursor_x = 0.0f;        // virtual pixels, top-left origin
static float g_cursor_y = 0.0f;
static bool g_cursor_valid = false;
static bool g_buttons[8] = {};
static int g_wheel_accum = 0;
static int g_cols = 80, g_rows = 24;

struct PendingEvent
{
	SDL_Event e;
};
static std::mutex g_queue_mutex;
static std::deque<PendingEvent> g_pending;

// Auto-release bookkeeping: legacy terminals give no key-up events, so every
// press is scheduled for an automatic release unless refreshed.
struct KeyDeadline
{
	int sdl_keycode;
	std::chrono::steady_clock::time_point deadline;
};
static std::unordered_map<int, std::chrono::steady_clock::time_point> g_held_keys;
static constexpr int AUTO_RELEASE_MS = 350;

// Modifier tracking for synthesized events
enum
{
	MOD_SHIFT = 1,
	MOD_CTRL = 2,
	MOD_ALT = 4
};
static int g_modifiers = 0;

// Kitty keyboard protocol support detected at startup?
static bool g_kitty_keyboard = false;

static std::thread g_input_thread;

// lightweight progress tracing to the redirected stderr (MC_LOG)
#define TMARK(msg) do { std::fprintf(stderr, "[bpp] %s\n", msg); std::fflush(stderr); } while (0)

// ---------------------------------------------------------------------------
// Low-level tty helpers
// ---------------------------------------------------------------------------

static void ttyWrite(const char *data, size_t len)
{
	if (g_tty >= 0)
	{
		size_t off = 0;
		while (off < len)
		{
			ssize_t n = ::write(g_tty, data + off, len - off);
			if (n <= 0)
			{
				if (errno == EINTR)
					continue;
				break;
			}
			off += static_cast<size_t>(n);
		}
	}
}

static void ttyWriteStr(const std::string &s)
{
	ttyWrite(s.data(), s.size());
}

static void updateTermSize()
{
	winsize ws{};
	if (::ioctl(g_tty, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0 && ws.ws_row > 0)
	{
		std::lock_guard<std::mutex> lock(g_state_mutex);
		g_cols = ws.ws_col;
		g_rows = ws.ws_row;
	}
}

// ---------------------------------------------------------------------------
// Event synthesis -> lwjgl queues
// ---------------------------------------------------------------------------

static void queueEvent(const SDL_Event &e)
{
	std::lock_guard<std::mutex> lock(g_queue_mutex);
	g_pending.push_back(PendingEvent{e});
}

static void pushKeyboard(int sdlKeycode, bool down, bool repeat)
{
	if (sdlKeycode == 0)
		return;

	SDL_Event e = {};
	e.type = down ? SDL_KEYDOWN : SDL_KEYUP;
	e.key.keysym.sym = sdlKeycode;
	e.key.state = down ? SDL_PRESSED : SDL_RELEASED;
	e.key.repeat = repeat ? 1 : 0;
	queueEvent(e);

	if (down && !repeat)
		g_held_keys[sdlKeycode] = std::chrono::steady_clock::now() + std::chrono::milliseconds(AUTO_RELEASE_MS);
	else if (!down)
		g_held_keys.erase(sdlKeycode);
}

static void pushText(const std::u32string &cps)
{
	if (cps.empty())
		return;

	SDL_Event e = {};
	e.type = SDL_TEXTINPUT;
	size_t pos = 0;
	auto appendUtf8 = [&](std::string utf8) {
		for (size_t i = 0; i < utf8.size() && pos + 1 < sizeof(e.text.text); i++)
			e.text.text[pos++] = utf8[i];
	};
	for (char32_t c : cps)
	{
		if (c < 0x80)
			appendUtf8(std::string(1, static_cast<char>(c)));
		else if (c < 0x800)
			appendUtf8(std::string({static_cast<char>(0xC0 | (c >> 6)), static_cast<char>(0x80 | (c & 0x3F))}));
		else if (c < 0x10000)
			appendUtf8(std::string({
				static_cast<char>(0xE0 | (c >> 12)),
				static_cast<char>(0x80 | ((c >> 6) & 0x3F)),
				static_cast<char>(0x80 | (c & 0x3F))
			}));
		else
			appendUtf8(std::string({
				static_cast<char>(0xF0 | (c >> 18)),
				static_cast<char>(0x80 | ((c >> 12) & 0x3F)),
				static_cast<char>(0x80 | ((c >> 6) & 0x3F)),
				static_cast<char>(0x80 | (c & 0x3F))
			}));
	}
	if (pos > 0)
	{
		e.text.text[pos] = '\0';
		queueEvent(e);
	}
}

// Maps ASCII/latin text to a SDL keycode so the game sees "real" keys.
static int asciiToSDL(unsigned c)
{
	switch (c)
	{
		case ' ': return SDLK_SPACE;
		case '\r': case '\n': return SDLK_RETURN;
		case '\t': return SDLK_TAB;
		case 0x08: case 0x7F: return SDLK_BACKSPACE;
		default: break;
	}
	if (c >= 'a' && c <= 'z') return SDLK_a + (c - 'a');
	if (c >= 'A' && c <= 'Z') return SDLK_a + (c - 'A');
	if (c >= '0' && c <= '9') return SDLK_0 + (c - '0');
	switch (c)
	{
		case '!': return SDLK_EXCLAIM;
		case '"': return SDLK_QUOTEDBL;
		case '#': return SDLK_HASH;
		case '$': case '%': return SDLK_PERCENT;
		case '&': return SDLK_AMPERSAND;
		case '\'': return SDLK_QUOTE;
		case '(': return SDLK_LEFTPAREN;
		case ')': return SDLK_RIGHTPAREN;
		case '*': return SDLK_ASTERISK;
		case '+': return SDLK_PLUS;
		case ',': return SDLK_COMMA;
		case '-': return SDLK_MINUS;
		case '.': return SDLK_PERIOD;
		case '/': return SDLK_SLASH;
		case ':': return SDLK_COLON;
		case ';': return SDLK_SEMICOLON;
		case '<': return SDLK_LESS;
		case '=': return SDLK_EQUALS;
		case '>': return SDLK_GREATER;
		case '?': return SDLK_QUESTION;
		case '@': return SDLK_AT;
		case '[': return SDLK_LEFTBRACKET;
		case '\\': return SDLK_BACKSLASH;
		case ']': return SDLK_RIGHTBRACKET;
		case '^': return SDLK_CARET;
		case '_': return SDLK_UNDERSCORE;
		case '`': return SDLK_BACKQUOTE;
		case '{': return SDLK_LEFTBRACKET;
		case '|': return 0; // no SDLK_PIPE in SDL2
		case '}': return SDLK_RIGHTBRACKET;
		case '~': return 0;
		default: return 0;
	}
}

static void pushPrintableChar(uint32_t cp)
{
	std::u32string cps;
	cps.push_back(cp);
	pushText(cps);

	// Also synthesize the matching key event (like real keyboards do).
	unsigned base = cp;
	if (base >= 'A' && base <= 'Z')
		base += 32;
	int sym = (base < 128) ? asciiToSDL(base) : 0;
	if (sym != 0)
	{
		bool shifted = cp != base;
		if (shifted && !(g_modifiers & MOD_SHIFT))
			pushKeyboard(SDLK_LSHIFT, true, false);
		pushKeyboard(sym, true, false);
		pushKeyboard(sym, false, false);
		if (shifted && !(g_modifiers & MOD_SHIFT))
			pushKeyboard(SDLK_LSHIFT, false, false);
	}
}

static int cellToVirtualX(int cell /*1-based*/, int cellsTotal)
{
	float t = (cellsTotal > 0) ? (static_cast<float>(cell - 1) + 0.5f) / cellsTotal : 0.5f;
	int x = static_cast<int>(t * g_render_w);
	if (x < 0) x = 0;
	if (x >= g_render_w) x = g_render_w - 1;
	return x;
}

static int cellToVirtualY(int cell, int cellsTotal)
{
	float t = (cellsTotal > 0) ? (static_cast<float>(cell - 1) + 0.5f) / cellsTotal : 0.5f;
	int y = static_cast<int>(t * g_render_h);
	if (y < 0) y = 0;
	if (y >= g_render_h) y = g_render_h - 1;
	return y;
}

static void pushMouseMotionAbs(int vx, int vy, int dxUp, int dyUp)
{
	SDL_Event e = {};
	e.type = SDL_MOUSEMOTION;
	e.motion.x = vx;
	e.motion.y = vy;
	e.motion.xrel = dxUp;
	e.motion.yrel = dyUp;
	queueEvent(e);
}

static void pushMouseButton(int lwjglButton, bool down, int vx, int vy)
{
	if (lwjglButton < 0 || lwjglButton >= 8)
		return;
	SDL_Event e = {};
	e.type = down ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
	e.button.button = lwjglButton + 1; // Mouse.cpp converts back to LWJGL order
	e.button.x = vx;
	e.button.y = vy;
	queueEvent(e);
}

static void pushWheel(int delta)
{
	SDL_Event e = {};
	e.type = SDL_MOUSEWHEEL;
	e.wheel.y = delta;
	queueEvent(e);
}

// ---------------------------------------------------------------------------
// Input byte-stream parser
// ---------------------------------------------------------------------------

class InputStream
{
public:
	void feed(const uint8_t *data, size_t n)
	{
		buffer.insert(buffer.end(), data, data + n);
		parse();
	}

	// True while more complete bytes remain buffered.
	bool empty() const { return buffer.empty(); }

private:
	std::vector<uint8_t> buffer;
	size_t pos = 0;

	int peek(size_t ahead) const { return (pos + ahead < buffer.size()) ? buffer[pos + ahead] : -1; }
	bool avail(size_t n) const { return pos + n <= buffer.size(); }
	void consume(size_t n) { pos += n; }

	static bool isFinalByte(int c) { return c >= 0x40 && c <= 0x7E; }
	static bool isParamByte(int c) { return (c >= '0' && c <= '9') || c == ';' || c == ':' || c == '<' || c == '=' || c == '>'; }

	void parse()
	{
		for (;;)
		{
			if (pos >= buffer.size())
				break;

			uint8_t b = buffer[pos];

			if (b != 0x1B)
			{
				parseGround(b);
				pos++;
				continue;
			}

			// Escape sequence family
			int next = peek(1);
			if (next < 0)
			{
				// Lone ESC at buffer end: wait for more bytes briefly handled by
				// caller timeout; treat as ESC press only if nothing follows.
				break;
			}
			if (next == '[')
			{
				consume(2);
				if (!parseCSI())
					break; // incomplete
				continue;
			}
			if (next == 'O')
			{
				int fin = peek(2);
				if (fin < 0)
					break;
				consume(2);
				parseSS3(static_cast<uint8_t>(fin));
				consume(1);
				continue;
			}
			if (next == '_')
			{
				// APC ... ST — swallow (kitty protocol responses land here).
				if (!swallowUntilST())
					break;
				continue;
			}
			if (next == ']' || next == 'P' || next == '^')
			{
				// OSC / DCS / PM — swallow till ST or BEL.
				if (!swallowUntilST())
					break;
				continue;
			}

			// ESC followed by anything else: treat as plain ESC keypress.
			consume(1);
			applyModifiers(0);
			handleKey(SDLK_ESCAPE, false);
			continue;
		}

		if (pos > 0)
			buffer.erase(buffer.begin(), buffer.begin() + static_cast<long>(pos));
		pos = 0;
	}

	// Returns false when the sequence is incomplete and needs more bytes.
	bool parseCSI()
	{
		// Collect parameter + intermediate bytes up to final byte.
		size_t start = pos;
		while (avail(1))
		{
			uint8_t b = buffer[pos];
			if (isFinalByte(b))
			{
				std::vector<uint8_t> seq(buffer.begin() + static_cast<long>(start),
										 buffer.begin() + static_cast<long>(pos));
				consume(1);
				dispatchCSI(seq, static_cast<char>(b));
				return true;
			}
			if (!isParamByte(b) && b != '?')
			{
				// Malformed; drop it.
				consume(1);
				return true;
			}
			pos++;
		}
		pos = start;
		return false;
	}

	bool swallowUntilST()
	{
		// Search for ESC \ (ST) or BEL terminator from current pos.
		for (size_t i = pos; i + 1 < buffer.size(); i++)
		{
			if (buffer[i] == 0x1B && buffer[i + 1] == '\\')
			{
				consume(i + 2 - pos);
				return true;
			}
			if (buffer[i] == 0x07)
			{
				consume(i + 1 - pos);
				return true;
			}
		}
		// Incomplete: keep bytes for later.
		return false;
	}

	void parseSS3(uint8_t fin)
	{
		applyModifiers(0);
		switch (fin)
		{
			case 'A': handleKey(SDLK_UP, false); break;
			case 'B': handleKey(SDLK_DOWN, false); break;
			case 'C': handleKey(SDLK_RIGHT, false); break;
			case 'D': handleKey(SDLK_LEFT, false); break;
			case 'H': handleKey(SDLK_HOME, false); break;
			case 'F': handleKey(SDLK_END, false); break;
			case 'P': handleKey(SDLK_F1, false); break;
			case 'Q': handleKey(SDLK_F2, false); break;
			case 'R': handleKey(SDLK_F3, false); break;
			case 'S': handleKey(SDLK_F4, false); break;
			case 'M': handleKey(SDLK_RETURN, false); break;
			default: break;
		}
	}

	void dispatchCSI(const std::vector<uint8_t> &seq, char finalByte)
	{
		std::string s(reinterpret_cast<const char *>(seq.data()), seq.size());

		if (!s.empty() && s[0] == '<')
		{
			parseSGRMouse(s.substr(1), finalByte);
			return;
		}

		// Split parameters on ';'
		std::vector<std::string> params;
		size_t begin = 0;
		if (!s.empty() && (s[0] == '?' || s[0] == '>' || s[0] == '=' || s[0] == '<'))
			begin = 1;
		for (size_t i = begin;; i++)
		{
			if (i == s.size() || s[i] == ';')
			{
				params.push_back(s.substr(begin, i - begin));
				begin = i + 1;
				if (i == s.size())
					break;
			}
		}

		auto paramInt = [&](size_t idx, int def) {
			if (idx >= params.size() || params[idx].empty())
				return def;
			try { return std::stoi(params[idx]); }
			catch (...) { return def; }
		};

		switch (finalByte)
		{
			case 'A': handleArrow(params, SDLK_UP); return;
			case 'B': handleArrow(params, SDLK_DOWN); return;
			case 'C': handleArrow(params, SDLK_RIGHT); return;
			case 'D': handleArrow(params, SDLK_LEFT); return;
			case 'H': applyModifiers(0); handleKey(SDLK_HOME, false); return;
			case 'F': applyModifiers(0); handleKey(SDLK_END, false); return;
			case 'Z': applyModifiers(MOD_SHIFT); handleKey(SDLK_TAB, false); applyModifiers(0); return;
			case 'u':
				parseKittyKey(params);
				return;
			case '~':
			{
				int code = paramInt(0, 0);
				applyModifiers(modsFromParam(paramInt(1, 1)));
				switch (code)
				{
					case 1: case 7: handleKey(SDLK_HOME, false); break;
					case 2: handleKey(SDLK_INSERT, false); break;
					case 3: handleKey(SDLK_DELETE, false); break;
					case 4: case 8: handleKey(SDLK_END, false); break;
					case 5: handleKey(SDLK_PAGEUP, false); break;
					case 6: handleKey(SDLK_PAGEDOWN, false); break;
					case 11: handleKey(SDLK_F1, false); break;
					case 12: handleKey(SDLK_F2, false); break;
					case 13: handleKey(SDLK_F3, false); break;
					case 14: handleKey(SDLK_F4, false); break;
					case 15: handleKey(SDLK_F5, false); break;
					case 17: handleKey(SDLK_F6, false); break;
					case 18: handleKey(SDLK_F7, false); break;
					case 19: handleKey(SDLK_F8, false); break;
					case 20: handleKey(SDLK_F9, false); break;
					case 21: handleKey(SDLK_F10, false); break;
					case 23: handleKey(SDLK_F11, false); break;
					case 24: handleKey(SDLK_F12, false); break;
					default: break;
				}
				applyModifiers(0);
				return;
			}
			default:
				break;
		}
	}

	static int modsFromParam(int p)
	{
		// Legacy xterm modifier param: value-1 bitfield {1=shift,2=alt,4=ctrl}
		int v = p - 1;
		if (v <= 0)
			return 0;
		int m = 0;
		if (v & 1) m |= MOD_SHIFT;
		if (v & 2) m |= MOD_ALT;
		if (v & 4) m |= MOD_CTRL;
		return m;
	}

	void handleArrow(const std::vector<std::string> &params, int sdlKey)
	{
		int mods = 0;
		if (params.size() >= 2 && !params[1].empty())
			mods = modsFromParam(std::atoi(params[1].c_str()));
		applyModifiers(mods);
		handleKey(sdlKey, false);
		applyModifiers(0);
	}

	// Kitty keyboard protocol: unicode-key-code[:alt];mods[:event-type]
	void parseKittyKey(const std::vector<std::string> &params)
	{
		if (params.empty() || params[0].empty())
			return;

		long keyCode = -1;
		try { keyCode = std::stol(params[0]); }
		catch (...) { return; }

		int mods = 0;
		int eventType = 1; // press
		if (params.size() >= 2 && !params[1].empty())
		{
			std::vector<std::string> parts;
			size_t b = 0;
			for (size_t i = 0;; i++)
			{
				if (i == params[1].size() || params[1][i] == ':')
				{
					parts.push_back(params[1].substr(b, i - b));
					b = i + 1;
					if (i == params[1].size())
						break;
				}
			}
			if (!parts.empty() && !parts[0].empty())
				mods = std::atoi(parts[0].c_str());
			if (parts.size() >= 2 && !parts[1].empty())
				eventType = std::atoi(parts[1].c_str());
		}

		applyModifiers(kittyModsToInternal(mods));

		if (eventType == 3)
			handleKeyUp(kittyKeyToSDL(keyCode));
		else
			handleKey(kittyKeyToSDL(keyCode), eventType == 2);

		if (eventType != 1)
			applyModifiers(0);
	}

	static int kittyModsToInternal(int m)
	{
		int r = 0;
		if (m & 1) r |= MOD_SHIFT;
		if (m & 2) r |= MOD_ALT;
		if (m & 4) r |= MOD_CTRL;
		return r;
	}

	static int kittyKeyToSDL(long code)
	{
		if (code < 128)
			return asciiToSDL(static_cast<unsigned>(code));
		switch (code)
		{
			case 57344: return SDLK_ESCAPE;
			case 57345: return SDLK_RETURN;
			case 57346: return SDLK_TAB;
			case 57347: return SDLK_BACKSPACE;
			case 57348: return SDLK_INSERT;
			case 57349: return SDLK_DELETE;
			case 57350: return SDLK_LEFT;
			case 57351: return SDLK_RIGHT;
			case 57352: return SDLK_UP;
			case 57353: return SDLK_DOWN;
			case 57354: return SDLK_PAGEUP;
			case 57355: return SDLK_PAGEDOWN;
			case 57356: return SDLK_HOME;
			case 57357: return SDLK_END;
			case 57358: return SDLK_CAPSLOCK;
			case 57362: return SDLK_PAUSE;
			case 57363: return SDLK_APPLICATION;
		}
		if (code >= 57364 && code <= 57375)
			return SDLK_F1 + static_cast<int>(code - 57364);
		if (code >= 57399 && code <= 57408) // KP_0..KP_9
			return SDLK_KP_0 + static_cast<int>(code - 57399);
		switch (code)
		{
			case 57409: return SDLK_KP_PERIOD;
			case 57410: return SDLK_KP_DIVIDE;
			case 57411: return SDLK_KP_MULTIPLY;
			case 57412: return SDLK_KP_MINUS;
			case 57413: return SDLK_KP_PLUS;
			case 57414: return SDLK_KP_ENTER;
			case 57415: return SDLK_KP_EQUALS;
			case 57441: return SDLK_LSHIFT;
			case 57442: return SDLK_LCTRL;
			case 57443: return SDLK_LALT;
			case 57444: return SDLK_LGUI;
			case 57445: return SDLK_RSHIFT;
			case 57446: return SDLK_RCTRL;
			case 57447: return SDLK_RALT;
			case 57448: return SDLK_RGUI;
		}
		return 0;
	}

	void parseSGRMouse(const std::string &body, char finalByte)
	{
		int rawBtn = 0, col = 0, row = 0;
		if (::sscanf(body.c_str(), "%d;%d;%d", &rawBtn, &col, &row) != 3)
			return;

		bool release = (finalByte == 'm');
		bool motion = (rawBtn & 32) != 0;
		bool wheel = (rawBtn & 64) != 0;
		int btn = rawBtn & 3;

		updateTermSize();

		int vx, vy;
		{
			std::lock_guard<std::mutex> lock(g_state_mutex);
			vx = cellToVirtualX(col, g_cols);
			vy = cellToVirtualY(row, g_rows);
		}

		if (wheel)
		{
			int dir = (btn == 0) ? 1 : ((btn == 1) ? -1 : 0); // 64=up,65=down
			if (dir != 0)
				pushWheel(dir);
			return;
		}

		// Virtual-cursor movement + grab-delta emulation.
		float nx = static_cast<float>(vx), ny = static_cast<float>(vy);
		int dxScreen = 0, dyScreen = 0;
		bool grabbedNow;
		{
			std::lock_guard<std::mutex> lock(g_state_mutex);
			grabbedNow = g_grabbed;
			if (motion || !release || true) // motion events always update cursor
			{
				if (g_cursor_valid)
				{
					dxScreen = static_cast<int>(nx) - static_cast<int>(g_cursor_x);
					dyScreen = static_cast<int>(ny) - static_cast<int>(g_cursor_y);
				}
				g_cursor_x = nx;
				g_cursor_y = ny;
				g_cursor_valid = true;
			}
		}

		if (grabbedNow)
		{
			// Wrap around edges so relative deltas keep flowing over SSH
			// (the host pointer cannot be warped remotely).
			const int marginX = g_render_w / 16, marginY = g_render_h / 16;
			std::lock_guard<std::mutex> lock(g_state_mutex);
			if (g_cursor_x < marginX || g_cursor_x > g_render_w - marginX ||
				g_cursor_y < marginY || g_cursor_y > g_render_h - marginY)
			{
				g_cursor_x = g_render_w / 2.0f;
				g_cursor_y = g_render_h / 2.0f;
			}
		}

		if (dxScreen != 0 || dyScreen != 0)
		{
			if (!grabbedNow)
			{
				dxScreen = 0;
				dyScreen = 0;
			}
			// Clamp implausible jumps (teleports after wrap).
			const int maxJump = g_render_w / 4;
			if (dxScreen > maxJump || dxScreen < -maxJump) dxScreen = 0;
			if (dyScreen > maxJump || dyScreen < -maxJump) dyScreen = 0;
			pushMouseMotionAbs(vx, vy, dxScreen, dyScreen);
		}

		if (wheel || motion)
			return;

		// Button state
		{
			std::lock_guard<std::mutex> lock(g_state_mutex);
			if (btn >= 0 && btn < 8)
				g_buttons[btn] = !release;
		}
		pushMouseButton(btn, !release, vx, vy);
	}

	// -------------------------------------------------------------------

	void applyModifiers(int mods)
	{
		struct Pair { int flag; int sdl; };
		static const Pair pairs[] = {
			{MOD_SHIFT, SDLK_LSHIFT},
			{MOD_CTRL, SDLK_LCTRL},
			{MOD_ALT, SDLK_LALT},
		};
		int diff = mods ^ g_modifiers;
		for (const Pair &p : pairs)
		{
			if (diff & p.flag)
				handleKey(p.sdl, (mods & p.flag) != 0);
		}
		g_modifiers = mods;
	}

	void handleKey(int sdlKey, bool repeat)
	{
		pushKeyboard(sdlKey, true, repeat);
	}

	void handleKeyUp(int sdlKey)
	{
		pushKeyboard(sdlKey, false, false);
	}

	void parseGround(uint8_t b)
	{
		switch (b)
		{
			case 0x03: // Ctrl+C
				g_close_requested = true;
				return;
			case 0x04: // Ctrl+D
				g_close_requested = true;
				return;
			case '\r': case '\n':
				pushPrintableChar('\r');
				return;
			case '\t':
				pushKeyboard(SDLK_TAB, true, false);
				pushText(U"\t");
				pushKeyboard(SDLK_TAB, false, false);
				return;
			case 0x7F: case 0x08:
				pushKeyboard(SDLK_BACKSPACE, true, false);
				pushKeyboard(SDLK_BACKSPACE, false, false);
				return;
			case 0x01: case 0x02: case 0x05: case 0x06: case 0x07:
			case 0x0B: case 0x0C: case 0x0E: case 0x0F: case 0x10:
			case 0x11: case 0x12: case 0x13: case 0x14: case 0x15:
			case 0x16: case 0x17: case 0x18: case 0x19: case 0x1A:
			{
				// Ctrl+letter
				uint8_t letter = b + ('A' - 1);
				applyModifiers(MOD_CTRL);
				pushPrintableChar(letter);
				applyModifiers(0);
				return;
			}
			case 0x1C: case 0x1D: case 0x1E: case 0x1F:
				return;
			default:
				break;
		}

		if (b < 0x20)
			return;

		// UTF-8 decode (handles multi-byte codepoints)
		uint32_t cp = 0;
		size_t extra = 0;
		if (b < 0x80)
		{
			cp = b;
		}
		else if ((b & 0xE0) == 0xC0) { cp = b & 0x1F; extra = 1; }
		else if ((b & 0xF0) == 0xE0) { cp = b & 0x0F; extra = 2; }
		else if ((b & 0xF8) == 0xF0) { cp = b & 0x07; extra = 3; }
		else
			return; // stray continuation byte

		// Continuation bytes sit ahead of the lead byte (pos points at b).
		for (size_t i = 0; i < extra; i++)
		{
			int cont = peek(1 + i);
			if (cont < 0 || (cont & 0xC0) != 0x80)
				return; // incomplete sequence; retry once more bytes arrive
			cp = (cp << 6) | static_cast<uint32_t>(cont & 0x3F);
		}
		pos += extra;

		pushPrintableChar(cp);
	}
};

static InputStream g_stream;

static void inputThreadFunc()
{
	uint8_t buf[4096];
	while (!g_quit_thread.load())
	{
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(0, &fds);
		timeval tv{};
		tv.tv_sec = 0;
		tv.tv_usec = 100 * 1000;
		int r = ::select(1, &fds, nullptr, nullptr, &tv);
		if (r < 0)
		{
			if (errno == EINTR)
				continue;
			break;
		}
		if (r == 0)
			continue;
		ssize_t n = ::read(0, buf, sizeof(buf));
		if (n <= 0)
		{
			if (n == 0)
				g_close_requested = true; // EOF (ssh session ended)
			else if (errno != EINTR && errno != EAGAIN)
				g_close_requested = true;
			continue;
		}
		g_stream.feed(buf, static_cast<size_t>(n));
	}
}

// ---------------------------------------------------------------------------
// Kitty graphics output
// ---------------------------------------------------------------------------

static const char BASE64_TABLE[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string base64Encode(const uint8_t *data, size_t len)
{
	std::string out;
	out.reserve(((len + 2) / 3) * 4);
	for (size_t i = 0; i < len; i += 3)
	{
		uint32_t v = static_cast<uint32_t>(data[i]) << 16;
		if (i + 1 < len) v |= static_cast<uint32_t>(data[i + 1]) << 8;
		if (i + 2 < len) v |= static_cast<uint32_t>(data[i + 2]);
		out += BASE64_TABLE[(v >> 18) & 63];
		out += BASE64_TABLE[(v >> 12) & 63];
		out += (i + 1 < len) ? BASE64_TABLE[(v >> 6) & 63] : '=';
		out += (i + 2 < len) ? BASE64_TABLE[v & 63] : '=';
	}
	return out;
}

// zlib deflate via runtime-linked zlib (we link zlibstatic directly)

static std::string g_frame_ctrl;   // cached control string pieces
static int g_last_cols = -1, g_last_rows = -1;

static void computeCellRect(int &outCols, int &outRows)
{
	int cols, rows;
	{
		std::lock_guard<std::mutex> lock(g_state_mutex);
		cols = g_cols;
		rows = g_rows;
	}
	if (cols < 2) cols = 2;
	if (rows < 2) rows = 2;

	// Assume ~1:2 cell aspect (width:height). Fit image preserving aspect.
	// Displayed aspect in "cell units" where width=1, height=2 per cell:
	// want (cols*1)/(rows*2) ~= renderW/renderH  =>  cols/rows = 2*W/H
	const double targetRatio = 2.0 * double(g_render_w) / double(g_render_h);
	int bestCols = cols;
	int bestRows = static_cast<int>(cols / targetRatio + 0.5);
	if (bestRows > rows)
	{
		bestRows = rows;
		bestCols = static_cast<int>(rows * targetRatio + 0.5);
	}
	if (bestCols > cols) bestCols = cols;
	if (bestRows > rows) bestRows = rows;
	if (bestRows < 1) bestRows = 1;
	if (bestCols < 1) bestCols = 1;

	outCols = bestCols;
	outRows = bestRows;
}

// Universal fallback: paint the frame as ANSI truecolor half-blocks (▀).
// Works in any 24-bit terminal (Konsole, GNOME Terminal, xterm-24bit, ...)
// even when the kitty graphics protocol isn't available.
static void sendFrameHalfBlock(const std::uint8_t *rgba, int width, int height)
{
	int cols, rows;
	{
		std::lock_guard<std::mutex> lock(g_state_mutex);
		cols = g_cols > 0 ? g_cols : 80;
		rows = g_rows > 0 ? g_rows : 24;
	}
	const int pixRows = rows * 2;
	if (cols < 1 || rows < 1 || width <= 0 || height <= 0)
		return;

	// Average a block of the (bottom-up) framebuffer for output pixel (px,py),
	// where py is top-down over pixRows rows.
	auto sample = [&](int px, int py, int &r, int &g, int &b)
	{
		int x0 = px * width / cols, x1 = (px + 1) * width / cols;
		int yTop = py * height / pixRows, yBot = (py + 1) * height / pixRows;
		if (x1 <= x0) x1 = x0 + 1;
		if (yBot <= yTop) yBot = yTop + 1;
		unsigned long rs = 0, gs = 0, bs = 0, n = 0;
		for (int y = yTop; y < yBot && y < height; ++y)
		{
			int fy = height - 1 - y; // GL framebuffers are bottom-up
			for (int x = x0; x < x1 && x < width; ++x)
			{
				const std::uint8_t *p = rgba + (static_cast<size_t>(fy) * width + x) * 4;
				rs += p[0]; gs += p[1]; bs += p[2]; ++n;
			}
		}
		if (n == 0) n = 1;
		r = static_cast<int>(rs / n); g = static_cast<int>(gs / n); b = static_cast<int>(bs / n);
	};

	std::string out;
	out.reserve(static_cast<size_t>(cols) * rows * 26 + 16);
	out += "\x1b[H";
	char buf[64];
	for (int cy = 0; cy < rows; ++cy)
	{
		for (int cx = 0; cx < cols; ++cx)
		{
			int ur, ug, ub, lr, lg, lb;
			sample(cx, cy * 2, ur, ug, ub);
			sample(cx, cy * 2 + 1, lr, lg, lb);
			::snprintf(buf, sizeof(buf),
			           "\x1b[38;2;%d;%d;%dm\x1b[48;2;%d;%d;%dm\xe2\x96\x80",
			           ur, ug, ub, lr, lg, lb);
			out += buf;
		}
		out += "\x1b[0m";
		if (cy + 1 < rows)
			out += "\r\n";
	}
	out += "\x1b[0m";
	ttyWriteStr(out);
}

void sendFrame(const std::uint8_t *rgba, int width, int height)
{
	if (g_tty < 0)
		return;
	if (g_halfblock)
	{
		sendFrameHalfBlock(rgba, width, height);
		return;
	}

	// RGBA -> RGB strip (bottom-up flip happens during copy since GL framebuffers
	// are bottom-up but kitty expects top-down).
	std::vector<uint8_t> rgb(static_cast<size_t>(width) * height * 3);
	for (int srcY = 0; srcY < height; srcY++)
	{
		int dstY = height - 1 - srcY;
		const uint8_t *srcRow = rgba + (static_cast<size_t>(srcY) * width * 4);
		uint8_t *dstRow = rgb.data() + (static_cast<size_t>(dstY) * width * 3);
		for (int x = 0; x < width; x++)
		{
			dstRow[x * 3 + 0] = srcRow[x * 4 + 0];
			dstRow[x * 3 + 1] = srcRow[x * 4 + 1];
			dstRow[x * 3 + 2] = srcRow[x * 4 + 2];
		}
	}

	int compressLevel = 3;
	{
		static int cached = -2;
		if (cached == -2)
			cached = std::atoi(envOr("MC_COMPRESS", "0").c_str());
		compressLevel = cached;
	}

	std::vector<uint8_t> payload = std::move(rgb);
	std::string compressKey;
	if (compressLevel > 0)
	{
		uLongf bound = ::compressBound(payload.size());
		std::vector<uint8_t> compressed(bound);
		uLongf compressedLen = bound;
		if (::compress2(compressed.data(), &compressedLen, payload.data(), payload.size(), compressLevel) == Z_OK &&
			compressedLen < payload.size())
		{
			payload.assign(compressed.data(), compressed.data() + compressedLen);
			compressKey = ",o=z";
		}
	}

	std::string b64 = base64Encode(payload.data(), payload.size());

	int cols, rows;
	computeCellRect(cols, rows);
	bool needPlacement = (cols != g_last_cols || rows != g_last_rows);
	g_last_cols = cols;
	g_last_rows = rows;

	std::string out;
	out += "\x1b[H"; // home cursor

	// Transmit-and-display in ONE command (a=T) with inline cell placement.
	// Konsole (and several others) implement a=T but not the separate a=p "put"
	// step, which is why a=t + a=p showed nothing there.
	char ctrl[200];
	::snprintf(ctrl, sizeof(ctrl),
			   "\x1b_Ga=T,f=24,i=1,s=%d,h=%d,c=%d,r=%d%s,q=1",
			   width, height, cols, rows, compressKey.c_str());

	// Chunked transmission (max 4096 base64 bytes per chunk)
	const size_t CHUNK = 4096;
	size_t offset = 0;
	bool firstChunk = true;
	while (offset < b64.size())
	{
		size_t n = (b64.size() - offset < CHUNK) ? (b64.size() - offset) : CHUNK;
		bool lastChunk = (offset + n >= b64.size());
		if (firstChunk)
			out += ctrl;
		else
			out += "\x1b_G";
		if (!lastChunk)
			out += firstChunk ? ",m=1;" : "m=1;";
		else
			out += firstChunk ? ";" : "m=0;";
		out.append(b64, offset, n);
		out += "\x1b\\";
		offset += n;
		firstChunk = false;
	}

	ttyWriteStr(out);
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void earlyInit()
{
	// Keep escape sequences clean: game logs go to a file instead of stdout.
	const std::string logPath = envOr("MC_LOG", "/tmp/betapluspp.log");
	if (::freopen(logPath.c_str(), "w", stdout) != nullptr)
		setvbuf(stdout, nullptr, _IOLBF, 1024);
	::freopen(logPath.c_str(), "a", stderr);

	// The terminal backend renders without any window system.
	::setenv("SDL_VIDEODRIVER", "dummy", 1);
}

// Blocking probe read with timeout. Returns bytes read into out (may be 0).
static int probeRead(char *out, size_t cap, int timeoutMs)
{
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(0, &fds);
	timeval tv{};
	tv.tv_sec = timeoutMs / 1000;
	tv.tv_usec = (timeoutMs % 1000) * 1000;
	int r = ::select(1, &fds, nullptr, nullptr, &tv);
	if (r <= 0)
		return 0;
	int n = static_cast<int>(::read(0, out, cap));
	return n > 0 ? n : 0;
}

static bool probeContains(const std::string &acc, const std::string &needle)
{
	return acc.find(needle) != std::string::npos;
}

void init(int renderWidth, int renderHeight)
{
	g_render_w = renderWidth;
	g_render_h = renderHeight;

	{
		const std::string res = envOr("MC_RES", "");
		int w = 0, h = 0;
		if (!res.empty() && ::sscanf(res.c_str(), "%dx%d", &w, &h) == 2 && w > 0 && h > 0)
		{
			g_render_w = w;
			g_render_h = h;
		}
	}

	{
		std::lock_guard<std::mutex> lock(g_state_mutex);
		g_cursor_x = g_render_w / 2.0f;
		g_cursor_y = g_render_h / 2.0f;
	}

	if (!::isatty(0) && !::isatty(1))
		throw std::runtime_error(
			"stdin/stdout are not a terminal. Run BetaPlusPlus inside a "
			"kitty-graphics capable terminal (kitty, WezTerm, Konsole, ghostty).");

	g_tty = ::open("/dev/tty", O_RDWR | O_CLOEXEC);
	if (g_tty < 0)
		throw std::runtime_error("cannot open controlling terminal (/dev/tty)");
	TMARK("init: tty opened");

	// Enter alternate screen etc. BEFORE probing so errors show cleanly.
	ttyWriteStr("\x1b[?1049h\x1b[?25l\x1b[H\x1b[2J");
	TMARK("init: alt screen");

	// Raw mode MUST be active before probing: probe responses contain no
	// newline, so a canonical-mode read() would block on the line discipline
	// even after select() reports data available.
	if (::tcgetattr(0, &g_saved_termios) == 0)
	{
		termios raw = g_saved_termios;
		raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
		raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
		raw.c_oflag &= ~(OPOST);
		raw.c_cc[VMIN] = 1;
		raw.c_cc[VTIME] = 0;
		if (::tcsetattr(0, TCSANOW, &raw) == 0)
			g_raw_mode = true;
	}
	TMARK(g_raw_mode ? "init: raw mode on" : "init: raw mode FAILED");
	::tcflush(0, TCIFLUSH);

	// --- Probe kitty graphics support -------------------------------------
	ttyWriteStr("\x1b_Gi=31,a=q,t=d,f=24,s=1,h=1;AAAA\x1b\\");
	ttyWriteStr("\x1b[c"); // DA1
	TMARK("init: gfx probe sent");

	std::string acc;
	char tmp[256];
	int deadlineMs = 700;
	while (deadlineMs > 0)
	{
		auto t0 = std::chrono::steady_clock::now();
		int n = probeRead(tmp, sizeof(tmp), deadlineMs);
		if (n <= 0)
			break;
		acc.append(tmp, static_cast<size_t>(n));
		auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(
					  std::chrono::steady_clock::now() - t0).count();
		deadlineMs -= static_cast<int>(dt);
		if (probeContains(acc, "i=31") && (probeContains(acc, "OK") || probeContains(acc, "EBAD")))
			break;
	}

	// Many kitty-graphics terminals (notably Konsole) implement transmit/display
	// but never answer the `a=q` probe. Treat a silent probe as "probably
	// supported" and try to render anyway, instead of aborting — otherwise the
	// app exits to a blank alt-screen on a perfectly capable terminal.
	{
		const std::string term = envOr("MC_TERM", "auto");
		if (term == "halfblock")
			g_halfblock = true;
		else if (term == "kitty")
			g_halfblock = false;
		else
			g_halfblock = !probeContains(acc, "i=31"); // silent probe -> half-blocks
		TMARK(g_halfblock ? "init: renderer = half-block ANSI"
		                  : "init: renderer = kitty graphics");
	}

	// --- Probe kitty keyboard protocol (optional, gives us key releases) --
	ttyWriteStr("\x1b[?u");
	acc.clear();
	{
		int ms = 250;
		while (ms > 0)
		{
			auto t0 = std::chrono::steady_clock::now();
			int n = probeRead(tmp, sizeof(tmp), ms);
			if (n <= 0)
				break;
			acc.append(tmp, static_cast<size_t>(n));
			ms -= static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(
									   std::chrono::steady_clock::now() - t0).count());
			if (probeContains(acc, "u"))
				break;
		}
	}
	if (probeContains(acc, "\x1b[?"))
		g_kitty_keyboard = true;
	TMARK(g_kitty_keyboard ? "init: kbd probe: kitty keyboard yes" : "init: kbd probe: legacy only");

	// Enable mouse reporting now (raw mode was applied before the probes).
	std::string enable;
	if (g_kitty_keyboard)
		enable += "\x1b[>3u";      // disambiguate + report event types
	enable += "\x1b[?1000h"          // mouse: click reporting
			  "\x1b[?1003h"          // mouse: all-motion reporting
			  "\x1b[?1006h";         // mouse: SGR encoding
	ttyWriteStr(enable);
	ttyWriteStr("\x1b[2J\x1b[H");
	TMARK(g_kitty_keyboard ? "init: probes done (kitty kbd yes)" : "init: probes done (kitty kbd no)");

	::tcflush(0, TCIFLUSH);
	updateTermSize();

	g_quit_thread = false;
	g_input_thread = std::thread(inputThreadFunc);
	TMARK("init: input thread running");
}

void shutdown()
{
	if (g_input_thread.joinable())
	{
		g_quit_thread = true;
		g_input_thread.join();
	}

	if (g_tty >= 0)
	{
		std::string restore = "\x1b[?1006l\x1b[?1003l\x1b[?1000l";
		if (g_kitty_keyboard)
			restore += "\x1b[<u";
		restore += "\x1b[?25h\x1b[?1049l";
		ttyWriteStr(restore);
	}

	if (g_raw_mode)
		::tcsetattr(0, TCSANOW, &g_saved_termios);

	if (g_tty >= 0)
	{
		::close(g_tty);
		g_tty = -1;
	}
}

void pump()
{
	std::deque<PendingEvent> local;
	{
		std::lock_guard<std::mutex> lock(g_queue_mutex);
		local.swap(g_pending);
	}

	for (const PendingEvent &pe : local)
	{
		const SDL_Event &e = pe.e;
		switch (e.type)
		{
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			case SDL_TEXTINPUT:
				Keyboard::detail::pushEvent(e);
				break;
			case SDL_MOUSEMOTION:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEWHEEL:
				Mouse::detail::pushEvent(e);
				break;
		}
	}

	// Auto-release stale keys (legacy terminals never send key-ups).
	auto now = std::chrono::steady_clock::now();
	std::vector<int> expired;
	{
		std::lock_guard<std::mutex> lock(g_queue_mutex);
		for (auto it = g_held_keys.begin(); it != g_held_keys.end();)
		{
			if (now >= it->second)
			{
				expired.push_back(it->first);
				it = g_held_keys.erase(it);
			}
			else
				++it;
		}
	}
	for (int key : expired)
	{
		SDL_Event e = {};
		e.type = SDL_KEYUP;
		e.key.keysym.sym = key;
		e.key.state = SDL_RELEASED;
		Keyboard::detail::pushEvent(e);
	}

	updateTermSize();
}

bool isCloseRequested()
{
	return g_close_requested.load();
}

// ---------------------------------------------------------------------------
// Mouse state accessors (used by pc/lwjgl/Mouse.cpp under MC_TERMINAL)
// ---------------------------------------------------------------------------

int mouseX()
{
	std::lock_guard<std::mutex> lock(g_state_mutex);
	return static_cast<int>(g_cursor_x);
}

int mouseY()
{
	std::lock_guard<std::mutex> lock(g_state_mutex);
	return static_cast<int>(g_cursor_y);
}

bool mouseButtonDown(int lwjglButton)
{
	std::lock_guard<std::mutex> lock(g_state_mutex);
	return lwjglButton >= 0 && lwjglButton < 8 && g_buttons[lwjglButton];
}

bool isGrabbed()
{
	std::lock_guard<std::mutex> lock(g_state_mutex);
	return g_grabbed;
}

void setGrabbed(bool grabbed)
{
	std::lock_guard<std::mutex> lock(g_state_mutex);
	g_grabbed = grabbed;
	g_cursor_x = g_render_w / 2.0f;
	g_cursor_y = g_render_h / 2.0f;
}

void setCursorPosition(int x, int y)
{
	std::lock_guard<std::mutex> lock(g_state_mutex);
	g_cursor_x = static_cast<float>(x);
	g_cursor_y = static_cast<float>(y);
	g_cursor_valid = true;
}

} // namespace terminal
} // namespace lwjgl
