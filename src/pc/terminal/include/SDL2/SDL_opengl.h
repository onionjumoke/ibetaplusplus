#pragma once

// Terminal-build shim: route SDL2's bundled OpenGL declarations through glad
// so every gl* call resolves to glad's function pointers (loaded at runtime
// from OSMesa/EGL) instead of requiring libGL at link time.
#include <glad/glad.h>
