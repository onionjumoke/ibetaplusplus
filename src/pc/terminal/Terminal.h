#pragma once

// Headless terminal backend: renders frames into any terminal implementing
// the kitty graphics protocol (kitty, WezTerm, Konsole, ghostty, ...) over a
// plain SSH connection. Input (keyboard + SGR mouse) is parsed from stdin in
// raw mode and fed into the same lwjgl event queues the SDL backend uses.

#include <cstdint>

namespace lwjgl
{
namespace terminal
{

// Must run before SDL_Init: pins SDL to the dummy video driver and moves the
// game's stdout/stderr chatter to a log file so escape sequences stay clean.
void earlyInit();

// Prepares the tty, probes kitty-graphics support, spawns the input thread.
// Throws std::runtime_error if the terminal cannot display images.
void init(int renderWidth, int renderHeight);

// Restores the terminal to its original state.
void shutdown();

// Drains queued input into the lwjgl Keyboard/Mouse queues (once per frame).
void pump();

// Streams one RGBA8 frame using the kitty graphics protocol.
void sendFrame(const std::uint8_t *rgba, int width, int height);

bool isCloseRequested();

// --- state consumed by the terminal build of pc/lwjgl/Mouse.cpp ---
int mouseX();
int mouseY();
bool mouseButtonDown(int lwjglButton);
bool isGrabbed();
void setGrabbed(bool grabbed);
void setCursorPosition(int x, int y);

} // namespace terminal
} // namespace lwjgl
