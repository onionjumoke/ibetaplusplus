// ─────────────────────────────────────────────────────────────────────────────
// lwjgl/GLContext.cpp (iOS) — binds the shared header to the EAGL context
// created by IOSApp.mm on the game thread, then initialises the GLES shim.
//
// The shared pc/lwjgl/GLContext.h is used verbatim: SDL.h and glad/glad.h
// resolve to the iOS shims (opaque window handles / GLES11Shim).
// ─────────────────────────────────────────────────────────────────────────────

#include "lwjgl/GLContext.h"

#include <stdexcept>

#include "IosBridge.h"
#include "GLES11Shim.h"

namespace lwjgl
{
namespace GLContext
{

namespace detail
{

static GLCapabilities g_capabilities;

SDL_Window *getWindow()
{
	// Opaque UIKit window handle; nothing in the portable layer dereferences it.
	return nullptr;
}

SDL_GLContext getGLContext()
{
	// Opaque EAGLContext handle; likewise never dereferenced above this layer.
	return nullptr;
}

}

// Context functions
void instantiate()
{
	if (!iosapp::waitForView(30.0))
		throw std::runtime_error("iOS view never became ready");

	if (!iosapp::createGlContext())
		throw std::runtime_error("failed to create EAGL OpenGLES 1.1 context");

	if (!gles11shim::initialize())
		throw std::runtime_error("failed to resolve OpenGLES entry points");

	// Parse the ES extension list the same way the SDL build does.
	if (const GLubyte *extensions = glGetString(GL_EXTENSIONS))
	{
		std::string cap;
		const char *p = reinterpret_cast<const char *>(extensions);
		while (*p != '\0')
		{
			if (*p == ' ')
			{
				if (!cap.empty())
				{
					detail::g_capabilities.add(cap);
					cap.clear();
				}
				while (*p == ' ')
					p++;
				continue;
			}
			cap.push_back(*p++);
		}
		if (!cap.empty())
			detail::g_capabilities.add(cap);
	}
}

const detail::GLCapabilities &getCapabilities()
{
	return detail::g_capabilities;
}

}
}
