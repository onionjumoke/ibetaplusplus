#pragma once

#include <glad/glad.h>
#include <SDL.h>

#ifndef GL_RESCALE_NORMAL
#define GL_RESCALE_NORMAL 0x803A
#endif

#ifndef GL_SAMPLES_PASSED_ARB
#define GL_SAMPLES_PASSED_ARB 0x8914
#define GL_QUERY_RESULT_ARB 0x8866
#define GL_QUERY_RESULT_AVAILABLE_ARB 0x8867

// Los drivers modernos a menudo NO exportan los alias "...ARB" aunque soporten la
// extension; exponen los nombres CORE (sin sufijo, OpenGL 1.5). Probamos ambos.
// Si nada carga, las queries fallan-ABIERTO (todo visible) para no borrar el mundo.
inline void *getProcEither(const char *arbName, const char *coreName)
{
	void *f = SDL_GL_GetProcAddress(arbName);
	if (!f) f = SDL_GL_GetProcAddress(coreName);
	return f;
}

inline void glGenQueriesARB(GLsizei count, GLuint *queries)
{
	using Function = void (APIENTRYP)(GLsizei, GLuint *);
	static Function function = reinterpret_cast<Function>(getProcEither("glGenQueriesARB", "glGenQueries"));
	if (function) function(count, queries);
	else for (GLsizei i = 0; i < count; ++i) queries[i] = 0;
}

inline void glDeleteQueriesARB(GLsizei count, const GLuint *queries)
{
	using Function = void (APIENTRYP)(GLsizei, const GLuint *);
	static Function function = reinterpret_cast<Function>(getProcEither("glDeleteQueriesARB", "glDeleteQueries"));
	if (function) function(count, queries);
}

inline void glBeginQueryARB(GLenum target, GLuint query)
{
	using Function = void (APIENTRYP)(GLenum, GLuint);
	static Function function = reinterpret_cast<Function>(getProcEither("glBeginQueryARB", "glBeginQuery"));
	if (function) function(target, query);
}

inline void glEndQueryARB(GLenum target)
{
	using Function = void (APIENTRYP)(GLenum);
	static Function function = reinterpret_cast<Function>(getProcEither("glEndQueryARB", "glEndQuery"));
	if (function) function(target);
}

inline void glGetQueryObjectuivARB(GLuint query, GLenum parameter, GLuint *value)
{
	using Function = void (APIENTRYP)(GLuint, GLenum, GLuint *);
	static Function function = reinterpret_cast<Function>(getProcEither("glGetQueryObjectuivARB", "glGetQueryObjectuiv"));
	if (function) function(query, parameter, value);
	// Fail-OPEN: sin soporte de query -> disponible y "paso 1 sample" (visible),
	// nunca 0 (que marcaria todo como ocluido y borraria el mundo).
	else *value = 1u;
}
#endif
