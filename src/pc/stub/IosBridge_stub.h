// Minimal stub for non-iOS builds — all no-ops.
#pragma once
#include <string>
namespace iosapp {
inline void setOverlayMode(int) {}
inline void publishTextInput(const std::string &) {}
inline void publishInputTarget(bool) {}
inline bool takePendingKeyboardShow() { return false; }
inline bool consumePendingTextInput(std::string &) { return false; }
inline bool consumePendingTextInject(std::string &, int &, bool &) { return false; }
inline void showTextInput(const char *) {}
inline void requestTextInput(const char *, const char *) {}
inline void hideTextInput() {}
inline bool isTextInputActive() { return false; }
inline std::string currentUsername() { return "Player"; }
} // namespace iosapp
