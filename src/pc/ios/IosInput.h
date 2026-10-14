#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// IosInput — input entry points the EAGLView touch layer calls. Implemented by
// the iOS lwjgl Mouse/Keyboard translation units; coordinates are SCREEN space
// (origin top-left, SDL convention) and keys are already LWJGL-coded.
//
// The shared pc/lwjgl headers keep their SDL_Event-shaped detail::pushEvent
// for the PC builds; these hooks are the iOS-native equivalent.
// ─────────────────────────────────────────────────────────────────────────────

namespace lwjgl
{

namespace Mouse
{
namespace detail
{
void pushScreenMotion(int x, int y, int xrel, int yrel);
void pushScreenButton(int button /* LWJGL order: 0=left 1=right 2=middle */, bool down, int x, int y);
void pushWheelDelta(int dz);
} // namespace detail
} // namespace Mouse

namespace Keyboard
{
namespace detail
{
void pushLwjglKey(int key, bool down);
void pushUnicodeChar(char32_t c);
} // namespace detail
} // namespace Keyboard

} // namespace lwjgl
