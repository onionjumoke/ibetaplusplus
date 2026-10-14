#include "lwjgl/GLContext.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "terminal/Terminal.h"
#include "terminal/OffscreenGL.h"

namespace lwjgl
{
namespace terminal
{

// ---------------------------------------------------------------------------
// Shared dlopen helpers
// ---------------------------------------------------------------------------

static void *openLib(const char *const *names)
{
	for (const char *const *p = names; *p; p++)
	{
		if (void *h = ::dlopen(*p, RTLD_NOW | RTLD_GLOBAL))
			return h;
	}
	return nullptr;
}

[[noreturn]] static void fail(const std::string &msg)
{
	throw std::runtime_error(msg);
}

// ---------------------------------------------------------------------------
// OSMesa provider — pure software desktop GL, no window system at all.
// The framebuffer is plain user memory we hand to Mesa; frames are copied
// straight out of it without glReadPixels.
// ---------------------------------------------------------------------------

using OSMesaContext = void *;
using OSMesaCreateContextExtFn = OSMesaContext (*)(unsigned format, int depthBits,
												   int stencilBits, int accumBits,
												   OSMesaContext sharelist);
using OSMesaMakeCurrentFn = int (*)(OSMesaContext ctx, void *buffer, unsigned type,
									int width, int height);
using OSMesaDestroyContextFn = void (*)(OSMesaContext ctx);
using OSMesaGetProcAddressFn = void *(*)(const char *);

class OSMesaProvider final : public OffscreenGL
{
public:
	~OSMesaProvider() override
	{
		if (m_context && m_destroy)
			m_destroy(m_context);
	}

	const char *name() const override { return "osmesa"; }

	void init(int width, int height)
	{
		static const char *kNames[] = {"libOSMesa.so.8", "libOSMesa.so.6", "libOSMesa.so", nullptr};
		m_lib = openLib(kNames);
		if (!m_lib)
			fail("libOSMesa not found (install libosmesa6)");

		auto create = reinterpret_cast<OSMesaCreateContextExtFn>(::dlsym(m_lib, "OSMesaCreateContextExt"));
		m_makeCurrent = reinterpret_cast<OSMesaMakeCurrentFn>(::dlsym(m_lib, "OSMesaMakeCurrent"));
		m_destroy = reinterpret_cast<OSMesaDestroyContextFn>(::dlsym(m_lib, "OSMesaDestroyContext"));
		m_getProc = reinterpret_cast<OSMesaGetProcAddressFn>(::dlsym(m_lib, "OSMesaGetProcAddress"));
		if (!create || !m_makeCurrent || !m_destroy)
			fail("libOSMesa is missing required symbols");

		// RGBA8, 24-bit depth, 8-bit stencil, no accumulation buffer.
		m_context = create(0x1908 /*GL_RGBA*/, 24, 8, 0, nullptr);
		if (!m_context)
			fail("OSMesaCreateContextExt failed — the software rasterizer is "
				 "unavailable on this CPU (llvmpipe requires NEON on ARM; try "
				 "MC_GL=egl with the vc4/fkms overlay enabled)");

		m_pixels.resize(static_cast<size_t>(width) * height * 4);

		if (!m_makeCurrent(m_context, m_pixels.data(), 0x1401 /*GL_UNSIGNED_BYTE*/, width, height))
			fail("OSMesaMakeCurrent failed");
	}

	void makeCurrent() override {}

	void *frameBuffer(size_t &strideBytes) override
	{
		strideBytes = 0; // tightly packed RGBA, caller knows the width
		return m_pixels.data();
	}

	void *getProcAddress(const char *name) override
	{
		if (!m_getProc)
			return nullptr;
		return m_getProc(name);
	}

private:
	void *m_lib = nullptr;
	OSMesaContext m_context = nullptr;
	OSMesaMakeCurrentFn m_makeCurrent = nullptr;
	OSMesaDestroyContextFn m_destroy = nullptr;
	OSMesaGetProcAddressFn m_getProc = nullptr;
	std::vector<unsigned char> m_pixels;
};

// ---------------------------------------------------------------------------
// EGL provider — GPU-accelerated when a DRM render node exists (vc4 on the
// Pi Zero with KMS/fkms enabled); falls back to software otherwise.
// Frames must be read back with glReadPixels.
// ---------------------------------------------------------------------------

using EGLDisplay_ = void *;
using EGLSurface_ = void *;
using EGLContext_ = void *;
using EGLConfig_ = void *;
using EGLBoolean_ = unsigned;
using EGLint_ = int;
using EGLenum_ = unsigned;

using PFN_eglGetPlatformDisplayEXT = EGLDisplay_ (*)(EGLenum_, void *, const EGLint_ *);
using PFN_eglGetDisplay = EGLDisplay_ (*)(void *);
using PFN_eglInitialize = EGLBoolean_ (*)(EGLDisplay_, EGLint_ *, EGLint_ *);
using PFN_eglChooseConfig = EGLBoolean_ (*)(EGLDisplay_, const EGLint_ *, EGLConfig_ *, EGLint_, EGLint_ *);
using PFN_eglBindAPI = EGLBoolean_ (*)(EGLenum_);
using PFN_eglCreatePbufferSurface = EGLSurface_ (*)(EGLDisplay_, EGLConfig_, const EGLint_ *);
using PFN_eglCreateContext = EGLContext_ (*)(EGLDisplay_, EGLConfig_, EGLContext_, const EGLint_ *);
using PFN_eglMakeCurrent = EGLBoolean_ (*)(EGLDisplay_, EGLSurface_, EGLSurface_, EGLContext_);
using PFN_eglQueryString = const char *(*)(EGLDisplay_, EGLint_);
using PFN_eglGetProcAddress = void *(*)(const char *);

class EGLProvider final : public OffscreenGL
{
public:
	const char *name() const override { return "egl"; }

	void init(int width, int height)
	{
		static const char *kEGLNames[] = {"libEGL.so.1", "libEGL.so", nullptr};
		m_egl = openLib(kEGLNames);
		if (!m_egl)
			fail("libEGL not found");

		auto getPlatformDisplay = reinterpret_cast<PFN_eglGetPlatformDisplayEXT>(::dlsym(m_egl, "eglGetPlatformDisplayEXT"));
		auto getDisplay = reinterpret_cast<PFN_eglGetDisplay>(::dlsym(m_egl, "eglGetDisplay"));
		auto initialize = reinterpret_cast<PFN_eglInitialize>(::dlsym(m_egl, "eglInitialize"));
		auto chooseConfig = reinterpret_cast<PFN_eglChooseConfig>(::dlsym(m_egl, "eglChooseConfig"));
		auto bindAPI = reinterpret_cast<PFN_eglBindAPI>(::dlsym(m_egl, "eglBindAPI"));
		auto createPbuffer = reinterpret_cast<PFN_eglCreatePbufferSurface>(::dlsym(m_egl, "eglCreatePbufferSurface"));
		auto createContext = reinterpret_cast<PFN_eglCreateContext>(::dlsym(m_egl, "eglCreateContext"));
		auto makeCurrent = reinterpret_cast<PFN_eglMakeCurrent>(::dlsym(m_egl, "eglMakeCurrent"));
		auto queryString = reinterpret_cast<PFN_eglQueryString>(::dlsym(m_egl, "eglQueryString"));
		m_eglGetProc = reinterpret_cast<PFN_eglGetProcAddress>(::dlsym(m_egl, "eglGetProcAddress"));
		if (!getDisplay || !initialize || !chooseConfig || !bindAPI ||
			!createPbuffer || !createContext || !makeCurrent)
			fail("libEGL missing required symbols");

		// Prefer the surfaceless platform explicitly (no X11/Wayland needed).
		bool haveSurfacelessExt = false;
		if (getPlatformDisplay && queryString)
		{
			const char *clientExts = queryString(nullptr /*EGL_NO_DISPLAY*/, 0x30BD /*EGL_EXTENSIONS*/);
			haveSurfacelessExt = clientExts && ::strstr(clientExts, "EGL_MESA_platform_surfaceless") != nullptr;
		}
		if (haveSurfacelessExt)
			m_display = getPlatformDisplay(0x31DD /*EGL_PLATFORM_SURFACELESS_MESA*/, nullptr, nullptr);
		if (!m_display)
			m_display = getDisplay(nullptr /*EGL_DEFAULT_DISPLAY*/);
		if (!m_display)
			fail("eglGetDisplay failed (no headless EGL platform available)");

		EGLint_ major = 0, minor = 0;
		if (!initialize(m_display, &major, &minor))
			fail("eglInitialize failed");

		const EGLint_ configAttribs[] = {
			0x3033 /*SURFACE_TYPE*/,     0x0001 /*PBUFFER_BIT*/,
			0x3024 /*RED_SIZE*/,         8,
			0x3023 /*GREEN_SIZE*/,       8,
			0x3022 /*BLUE_SIZE*/,        8,
			0x3025 /*DEPTH_SIZE*/,       24,
			0x3026 /*STENCIL_SIZE*/,     8,
			0x3040 /*RENDERABLE_TYPE*/,  0x0002 /*OPENGL_BIT*/,
			0x3038 /*NONE*/
		};
		EGLConfig_ config = nullptr;
		EGLint_ count = 0;
		if (!chooseConfig(m_display, configAttribs, &config, 1, &count) || count < 1)
			fail("eglChooseConfig found no desktop-GL pbuffer config "
				 "(driver may be GLES-only)");

		if (!bindAPI(0x30A2 /*EGL_OPENGL_API*/))
			fail("eglBindAPI(EGL_OPENGL_API) unsupported");

		const EGLint_ pbAttribs[] = {0x3057 /*WIDTH*/, width, 0x3056 /*HEIGHT*/, height, 0x3038 /*NONE*/};
		m_surface = createPbuffer(m_display, config, pbAttribs);
		if (!m_surface)
			fail("eglCreatePbufferSurface failed");

		m_context = createContext(m_display, config, nullptr /*EGL_NO_CONTEXT*/, nullptr);
		if (!m_context)
			fail("eglCreateContext(desktop GL) failed — driver may only support GLES");

		if (!makeCurrent(m_display, m_surface, m_surface, m_context))
			fail("eglMakeCurrent failed");
	}

	void makeCurrent() override {}

	void *frameBuffer(size_t &) override { return nullptr; } // use glReadPixels

	void *getProcAddress(const char *name) override
	{
		if (m_eglGetProc)
			if (void *p = m_eglGetProc(name))
				return p;
		return ::dlsym(m_egl, name);
	}

private:
	void *m_egl = nullptr;
	PFN_eglGetProcAddress m_eglGetProc = nullptr;
	EGLDisplay_ m_display = nullptr;
	EGLSurface_ m_surface = nullptr;
	EGLContext_ m_context = nullptr;
};

// ---------------------------------------------------------------------------
// Factory
// ---------------------------------------------------------------------------

std::unique_ptr<OffscreenGL> createOffscreenGL(int width, int height)
{
	std::string choice = "auto";
	if (const char *v = ::getenv("MC_GL"))
		choice = v;

	std::vector<std::string> order;
	if (choice == "osmesa" || choice == "egl")
		order.push_back(choice);
	else
		order = {"egl", "osmesa"};

	std::string attempts;
	for (const std::string &kind : order)
	{
		try
		{
			std::unique_ptr<OffscreenGL> provider;
			if (kind == "osmesa")
				provider.reset(new OSMesaProvider());
			else if (kind == "egl")
				provider.reset(new EGLProvider());
			else
				continue;

			provider->init(width, height);
			return provider;
		}
		catch (const std::exception &e)
		{
			attempts += kind + ": " + e.what() + "; ";
		}
	}
	fail("no usable offscreen GL backend (" + attempts + ")");
}

} // namespace terminal

namespace GLContext
{

namespace detail
{

// Resolution strategy shared by all providers:
//   1. dlsym on libGL (core entry points)
//   2. eglGetProcAddress (extension entry points)
//   3. OSMesaGetProcAddress (software path extensions)
static void *terminalResolve(const char *name)
{
	static void *glLib = []() -> void * {
		const char *names[] = {"libGL.so.1", "libGL.so", nullptr};
		for (const char *const *n = names; *n; n++)
			if (void *h = ::dlopen(*n, RTLD_NOW | RTLD_LAZY | RTLD_GLOBAL))
				return h;
		return nullptr;
	}();

	if (void *p = ::dlsym(glLib, name))
		return p;

	static auto eglGetProc = reinterpret_cast<void *(*)(const char *)>(
		::dlsym(RTLD_DEFAULT, "eglGetProcAddress"));
	if (eglGetProc)
		if (void *p = eglGetProc(name))
			return p;

	static auto osmesaGetProc = []() -> void *(*)(const char *) {
		if (void *h = ::dlopen("libOSMesa.so.8", RTLD_NOW | RTLD_LAZY))
			return reinterpret_cast<void *(*)(const char *)>(::dlsym(h, "OSMesaGetProcAddress"));
		return nullptr;
	}();
	if (osmesaGetProc)
		if (void *p = osmesaGetProc(name))
			return p;

	return nullptr;
}

class ContextSingleton
{
public:
	std::unique_ptr<terminal::OffscreenGL> provider;
	GLCapabilities capabilities;
	const int width = 854, height = 480;

	static inline terminal::OffscreenGL *s_activeProvider = nullptr;

	ContextSingleton()
	{
		std::fprintf(stderr, "[bpp] GLContext: terminal init...\n");
		std::fflush(stderr);
		// Terminal must be alive before anything renders.
		terminal::init(width, height);

		try
		{
			provider = terminal::createOffscreenGL(width, height);
		}
		catch (...)
		{
			terminal::shutdown();
			throw;
		}
		std::fprintf(stderr, "[bpp] GLContext: provider '%s' ready\n", provider->name());
		std::fflush(stderr);
		provider->makeCurrent();

		// Prefer the provider's own resolver: dlsym on glvnd's libGL would
		// dispatch into a context table that never saw our OSMesa/EGL context.
		s_activeProvider = provider.get();
		int gladOk = gladLoadGLLoader([](const char *name) -> void * {
			if (s_activeProvider)
				if (void *p = s_activeProvider->getProcAddress(name))
					return p;
			return terminalResolve(name);
		});
		std::fprintf(stderr, "[bpp] GLContext: glad pass done ok=%d\n", gladOk);
		std::fflush(stderr);
		if (!gladOk)
		{
			terminal::shutdown();
			throw std::runtime_error("failed to load OpenGL entry points");
		}
		const GLubyte *verStr = glGetString(GL_VERSION);
		std::fprintf(stderr, "[bpp] GLContext: glad loaded (%s)\n", verStr ? reinterpret_cast<const char *>(verStr) : "(null)");
		std::fflush(stderr);

		// Parse extension list exactly like the SDL backend.
		const GLubyte *extensions = glGetString(GL_EXTENSIONS);
		if (extensions != nullptr)
		{
			std::string cap;
			const char *p = reinterpret_cast<const char *>(extensions);
			while (*p != '\0')
			{
				if (*p == ' ')
				{
					if (!cap.empty())
					{
						capabilities.add(cap);
						cap.clear();
					}
					while (*p == ' ')
						p++;
					continue;
				}
				cap.push_back(*p++);
			}
			if (!cap.empty())
				capabilities.add(cap);
		}
	}
};

static ContextSingleton &getContext()
{
	static ContextSingleton context;
	return context;
}

SDL_Window *getWindow()
{
	return nullptr; // no window system in terminal mode
}

SDL_GLContext getGLContext()
{
	return reinterpret_cast<SDL_GLContext>(&getContext());
}

} // namespace detail

void instantiate()
{
	detail::getContext();
}

const detail::GLCapabilities &getCapabilities()
{
	return detail::getContext().capabilities;
}

} // namespace GLContext

namespace terminal
{

OffscreenGL &activeOffscreen()
{
	return *GLContext::detail::getContext().provider;
}

int renderWidth() { return GLContext::detail::getContext().width; }
int renderHeight() { return GLContext::detail::getContext().height; }

} // namespace terminal
} // namespace lwjgl
