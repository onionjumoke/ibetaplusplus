#pragma once

#include <string>

// ─────────────────────────────────────────────────────────────────────────────
// IosBridge — the narrow C++ seam between the portable platform layer and the
// Objective-C UIKit world (IOSApp.mm / EAGLView.mm).
//
// Threading model: main thread runs UIApplicationMain and all UIKit work; the
// game loop runs on its own std::thread and owns the EAGL context.
// ─────────────────────────────────────────────────────────────────────────────

namespace iosapp
{

// ── UIKit side → engine (main thread) ────────────────────────────────────────
void notifySurfaceResized(int width, int height);
void notifyAppActive(bool active);
void notifyQuitRequested();

// ── Engine side ← UIKit (game thread) ────────────────────────────────────────
// Blocks until the window/view pair exists (delegate finished launching).
bool waitForView(double timeoutSeconds);

bool isAppActive();
bool isQuitRequested();

// Creates the EAGL ES1 context + framebuffers bound to this thread and makes
// it current. Must be called once from the thread that will render.
bool createGlContext();
void destroyGlContext();

// Presents the renderbuffer (swapBuffers).
void presentFrame();

int surfaceWidth();
int surfaceHeight();

// Virtual-keyboard support for chat screens: shows/hides a UITextField whose
// output is forwarded into lwjgl::Keyboard as character events.
void showTextInput(const char *initial);
void showFieldInput(const char *initial);
void hideTextInput();
bool isTextInputActive();
// Overlay KB button: shows the field if hidden, hides it otherwise.
void toggleKeyboard();

// Generic alert text input (username editing etc.). The typed string is NOT
// injected as key events; the game thread picks it up via
// consumePendingTextInput().
void requestTextInput(const char *title, const char *initial);
bool consumePendingTextInput(std::string &out);
std::string currentUsername();

// Full-replacement text injection from the iOS keyboard alert: on Send the
// game thread replays BACKSPACEs for the field's snapshot length plus the new
// characters through the normal key queue. withReturn appends a RETURN tap.
bool consumePendingTextInject(std::string &out, int &backspaces, bool &withReturn);

// Game thread publishes the currently focused text source each frame (chat
// line, options username field, or empty when none). The KB button prefills
// the alert from it so existing text can be edited/resent.
void publishTextInput(const std::string &text);
void publishInputTarget(bool exists);
bool inputTargetExists();
std::string activeInputText();
// Deferred-show handshake: KB tap with no live target defers the alert until
// the game thread has opened the input source and published fresh text.
bool takePendingKeyboardShow();
bool takePendingFieldBlur();

// Overlay visibility mode, written by the game thread each frame:
// 0 = main menu, 1 = in world no screen, 2 = in world with a GUI screen open.
void setOverlayMode(int mode);
int overlayMode();
inline bool overlayInWorld() { return overlayMode() == 1 || overlayMode() == 2; }

// Compact HUD (coordinates / fps / chunk info): the game thread publishes a
// preformatted line each frame; the UIKit layer renders it in a label.
void publishHud(const std::string &text);
std::string hudText();

// ── Entry point (implemented in Main_ios.mm) ─────────────────────────────────
// Runs the whole portable game bootstrap (GLContext::instantiate →
// Minecraft::start) on the calling thread. The app delegate spawns it.
int runGameOnThisThread();

} // namespace iosapp
