#pragma once

// Internal interface shared between the terminal GLContext (which creates an
// offscreen context) and Display (which reads pixels out of it each frame).

#include <cstddef>
#include <memory>

namespace lwjgl
{
namespace terminal
{

class OffscreenGL
{
public:
	virtual ~OffscreenGL() {}

	virtual const char *name() const = 0;

	// Configure and create the context. Throws std::runtime_error on failure.
	virtual void init(int width, int height) = 0;

	// Must be called before any GL call.
	virtual void makeCurrent() = 0;

	// Resolves GL entry points consistent with this provider's context
	// (glvnd's libGL dispatch must be avoided for non-glvnd contexts).
	virtual void *getProcAddress(const char *name) = 0;

	// Direct framebuffer access (OSMesa renders straight into user memory).
	// Returns nullptr when the provider requires glReadPixels instead (EGL).
	virtual void *frameBuffer(size_t &strideBytes) = 0;
};

// Creates the best available offscreen context. Honors $MC_GL ("osmesa",
// "egl", default: try egl then osmesa). Throws std::runtime_error listing
// what was attempted when nothing works.
std::unique_ptr<OffscreenGL> createOffscreenGL(int width, int height);

// Access to the context created by lwjgl::GLContext::instantiate().
OffscreenGL &activeOffscreen();
int renderWidth();
int renderHeight();

} // namespace terminal
} // namespace lwjgl
