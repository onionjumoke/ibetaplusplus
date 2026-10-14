#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// GLES11Shim — desktop OpenGL 1.x compatibility surface over OpenGL ES 1.1.
//
// The game code (src/net/minecraft, src/util, src/java) is written against
// desktop GL: immediate mode, display lists and unsuffixed matrix commands.
// iOS only exposes ES 1.1, which dropped glBegin/glEnd and display lists but
// kept the fixed-function pipeline. This header + GLES11Shim.cpp provide the
// missing entry points:
//
//   * glBegin/glEnd/glVertex2i/glTexCoord2f   → CPU vertex buffer + glDrawArrays
//   * glGenLists/glNewList/.../glDeleteLists  → recorded command stream replay
//     (client-side vertex arrays are DEEP-COPIED at record time)
//   * glOrtho/glFrustum/glScaled/glClearDepth → float-suffixed ES equivalents
//   * glColor3f / glFogi / glColorMaterial    → ES-compatible stand-ins
//   * ARB vertex-buffer aliases               → ES 1.1 core buffer objects
//   * ARB occlusion queries                   → fail-open stubs (everything
//                                                 counts as visible), matching
//                                                 pc/OpenGL.h behaviour on PC
//
// Matrix commands are shadowed so display lists capture them correctly; a
// software mirror of the stacks answers glGetFloatv() matrix queries.
// Include this INSTEAD OF <glad/glad.h> on iOS — see src/pc/OpenGL.h and
// src/pc/ios/include/glad/glad.h.
// ─────────────────────────────────────────────────────────────────────────────

#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif

// ES 1.x has no double-precision GL types; desktop signatures need it.
#ifndef GLdouble
typedef double GLdouble;
#endif

// ── Primitive types that exist in desktop GL but not in ES 1.x ───────────────
// The game never feeds these to glDrawArrays directly (Tessellator converts
// quads to triangles itself); they only appear as arguments to glBegin().
#ifndef GL_QUADS
#define GL_QUADS 0x0007
#endif
#ifndef GL_QUAD_STRIP
#define GL_QUAD_STRIP 0x0008
#endif
#ifndef GL_POLYGON
#define GL_POLYGON 0x0009
#endif
#ifndef GL_LINE_LOOP
#define GL_LINE_LOOP 0x0002
#endif

// ── Material enums used by glColorMaterial() calls (no-op'd below) ───────────
#ifndef GL_FRONT_AND_BACK
#define GL_FRONT_AND_BACK 0x0408
#endif
#ifndef GL_AMBIENT_AND_DIFFUSE
#define GL_AMBIENT_AND_DIFFUSE 0x1602
#endif

// ── Tokens dropped from ES that the display-list / glCallLists API needs ────
#ifndef GL_COMPILE
#define GL_COMPILE 0x1300
#define GL_COMPILE_AND_EXECUTE 0x1301
#endif
#ifndef GL_INT
#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_SHORT 0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT 0x1404
#define GL_UNSIGNED_INT 0x1405
#define GL_FLOAT 0x1406
#define GL_DOUBLE 0x140A
#endif

// GL_CLAMP (0x2900) exists in desktop GL; ES only has CLAMP_TO_EDGE. The
// game's textures are all power-of-two so REPEAT/CLAMP behave identically —
// map to the ES token.
#ifndef GL_CLAMP
#define GL_CLAMP 0x2900
#endif

// NV fog-distance extension tokens referenced by Config.cpp: unsupported on
// SGX hardware, SDL_GL_ExtensionSupported() reports false for them anyway.
#ifndef GL_FOG_DISTANCE_MODE_NV
#define GL_FOG_DISTANCE_MODE_NV 0x855A
#define GL_EYE_RADIAL_NV 0x855B
#endif

#ifndef GL_RESCALE_NORMAL_EXT
#define GL_RESCALE_NORMAL_EXT 0x803A
#endif

// Colour-matrix stack tokens (desktop-only); the shim tracks the stack anyway.
#ifndef GL_COLOR
#define GL_COLOR 0x1800
#define GL_COLOR_MATRIX 0x1801
#endif

// ── ARB occlusion query tokens: defined here so pc/OpenGL.h skips its own
//    SDL_GL_GetProcAddress-based inline stubs on iOS. ─────────────────────────
#define GL_SAMPLES_PASSED_ARB 0x8914
#define GL_QUERY_RESULT_ARB 0x8866
#define GL_QUERY_RESULT_AVAILABLE_ARB 0x8867

// ── ARB vertex buffer object aliases → ES 1.1 core buffer objects ────────────
#ifndef GL_ARRAY_BUFFER_ARB
#define GL_ARRAY_BUFFER_ARB 0x8892
#define GL_STREAM_DRAW_ARB 0x88E0
#endif

extern "C" {

// Display lists (emulated; see GLES11Shim.cpp)
GLuint APIENTRY glGenLists(GLsizei range);
void APIENTRY glNewList(GLuint list, GLenum mode);
void APIENTRY glEndList(void);
void APIENTRY glCallList(GLuint list);
void APIENTRY glCallLists(GLsizei n, GLenum type, const void *lists);
void APIENTRY glDeleteLists(GLuint list, GLsizei range);
GLboolean APIENTRY glIsList(GLuint list);

// Immediate mode (emulated)
void APIENTRY glBegin(GLenum mode);
void APIENTRY glEnd(void);
void APIENTRY glVertex2i(GLint x, GLint y);
void APIENTRY glVertex2f(GLfloat x, GLfloat y);
void APIENTRY glVertex3f(GLfloat x, GLfloat y, GLfloat z);
void APIENTRY glTexCoord2f(GLfloat s, GLfloat t);

// Missing small-fry: colour without alpha, integer fog parameters,
// no-op material colour tracking.
void APIENTRY glColor3f(GLfloat r, GLfloat g, GLfloat b);
void APIENTRY glFogi(GLenum pname, GLint param);
void APIENTRY glColorMaterial(GLenum face, GLenum mode);

// Desktop names for float-suffixed ES matrix functions.
void APIENTRY glClearDepth(GLdouble depth);
void APIENTRY glDepthRange(GLdouble nearVal, GLdouble farVal);
void APIENTRY glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
void APIENTRY glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
void APIENTRY glScaled(GLdouble x, GLdouble y, GLdouble z);

// ARB buffer object aliases (Tessellator's optional VBO path).
void APIENTRY glGenBuffersARB(GLsizei n, GLuint *buffers);
void APIENTRY glDeleteBuffersARB(GLsizei n, const GLuint *buffers);
void APIENTRY glBindBufferARB(GLenum target, GLuint buffer);
void APIENTRY glBufferDataARB(GLenum target, GLsizeiptr size, const void *data, GLenum usage);

// ARB occlusion queries: fail-open (always "visible"), same as the PC
// fallback when a driver lacks GL_ARB_occlusion_query.
void APIENTRY glGenQueriesARB(GLsizei n, GLuint *ids);
void APIENTRY glDeleteQueriesARB(GLsizei n, const GLuint *ids);
void APIENTRY glBeginQueryARB(GLenum target, GLuint id);
void APIENTRY glEndQueryARB(GLenum target);
void APIENTRY glGetQueryObjectuivARB(GLuint id, GLenum pname, GLuint *params);

} // extern "C"

namespace gles11shim
{

// Called once by lwjgl::GLContext::instantiate() after the EAGL context is
// current. Resolves the real ES entry points via dlsym and resets state.
bool initialize();

// True while a display list compile or an immediate-mode block is open.
bool insideRecordOrBegin();

// Debug counters for remote diagnosis (cheap atomics; always updated).
struct DebugStats
{
	unsigned long clears;
	unsigned long immFlushes;
	unsigned long immVerts;
	unsigned long arrVerts;
	unsigned long listCalls;
	unsigned long drawArrays;
	unsigned long drawElements;
	unsigned long listCompiles;
	unsigned long texUploadBytes;
	unsigned long texUploadCalls;
	int lastGlError;
};
DebugStats debugStats();

}
