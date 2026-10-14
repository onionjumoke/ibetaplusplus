// ─────────────────────────────────────────────────────────────────────────────
// GLES11Shim.cpp — desktop GL 1.x emulation over OpenGL ES 1.1.
// See GLES11Shim.h for the design notes.
//
// Three mechanisms:
//   1. Record-or-forward wrappers: every call that may appear inside a display
//      list goes through a tiny interpreter. Outside lists the call forwards to
//      the real ES entry point (and mirrors matrices into software stacks);
//      inside glNewList/glEndList it is packed into the list's command stream.
//   2. Display lists: recorded command streams, replayed on glCallList(s).
//      glDrawArrays during a compile DEEP-COPIES the client-side vertex data,
//      because the game's Tessellator reuses its staging buffer immediately
//      after the list is closed (a raw pointer copy would render garbage).
//      Pointer/client-state commands are deliberately NOT recorded — each
//      snapshot carries everything its draw needs, which also keeps stale CPU
//      addresses out of the stream.
//   3. Immediate mode: glBegin..glEnd accumulates vertices and draws once on
//      glEnd, converting quads to triangles like the Tessellator does.
//
// The real ES entry points are resolved with dlsym(RTLD_DEFAULT). Our shadow
// functions carry hidden visibility so dyld never resolves those lookups back
// at us (no recursion), while other translation units of the executable still
// link against the shadows instead of the framework exports.
//
// Threading: every GL call in this port happens on the single game thread;
// no locking here on purpose.
// ─────────────────────────────────────────────────────────────────────────────

#include "GLES11Shim.h"

#include <dlfcn.h>
#include <execinfo.h>

#include <cmath>
#include <cstdint>
#include <cstring>
#include <atomic>
#include <cstdio>
#include <map>
#include <set>
#include <vector>

#define SHIM_HIDDEN __attribute__((visibility("hidden")))

// Baked-playback counters (sampled by the system profiler).
std::atomic<uint32_t> g_playDraws{0};
std::atomic<uint32_t> g_playLists{0};

// Debug telemetry shared by wrappers and gles11shim::debugStats().
namespace
{
struct DbgCounters
{
	unsigned long clears = 0;
	unsigned long immFlushes = 0;
	unsigned long immVerts = 0;
	unsigned long arrVerts = 0;
	unsigned long listCalls = 0;
	unsigned long drawArrays = 0;
	unsigned long drawElements = 0;
	unsigned long listCompiles = 0;
	unsigned long texUploadBytes = 0;
	unsigned long texUploadCalls = 0;
	int lastGlError = 0;
};
DbgCounters g_dbg;
} // namespace

// ═════════════════════════════════════════════════════════════════════════════
// Infrastructure
// ═════════════════════════════════════════════════════════════════════════════

namespace
{

template <typename T>
T resolve(const char *name)
{
	return reinterpret_cast<T>(dlsym(RTLD_DEFAULT, name));
}

struct RealGL
{
	void (*Enable)(GLenum);
	void (*Disable)(GLenum);
	void (*BindTexture)(GLenum, GLuint);
	void (*DepthMask)(GLboolean);
	void (*BlendFunc)(GLenum, GLenum);
	void (*AlphaFunc)(GLenum, GLfloat);
	void (*ShadeModel)(GLenum);
	void (*TexParameteri)(GLenum, GLenum, GLint);
	void (*Fogf)(GLenum, GLfloat);
	void (*Fogfv)(GLenum, const GLfloat *);
	void (*Lightfv)(GLenum, GLenum, const GLfloat *);
	void (*LightModelfv)(GLenum, const GLfloat *);
	void (*LineWidth)(GLfloat);
	void (*PolygonOffset)(GLfloat, GLfloat);
	void (*ColorMask)(GLboolean, GLboolean, GLboolean, GLboolean);
	void (*ClearColor)(GLfloat, GLfloat, GLfloat, GLfloat);
	void (*CullFace)(GLenum);
	void (*Clear)(GLbitfield);
	void (*LoadMatrixf)(const GLfloat *);
	void (*Color4f)(GLfloat, GLfloat, GLfloat, GLfloat);
	void (*Normal3f)(GLfloat, GLfloat, GLfloat);

	void (*EnableClientState)(GLenum);
	void (*DisableClientState)(GLenum);
	void (*VertexPointer)(GLint, GLenum, GLsizei, const void *);
	void (*TexCoordPointer)(GLint, GLenum, GLsizei, const void *);
	void (*ColorPointer)(GLint, GLenum, GLsizei, const void *);
	void (*NormalPointer)(GLenum, GLsizei, const void *);
	void (*DrawArrays)(GLenum, GLint, GLsizei);

	void (*MatrixMode)(GLenum);
	void (*LoadIdentity)(void);
	void (*PushMatrix)(void);
	void (*PopMatrix)(void);
	void (*Translatef)(GLfloat, GLfloat, GLfloat);
	void (*Rotatef)(GLfloat, GLfloat, GLfloat, GLfloat);
	void (*Scalef)(GLfloat, GLfloat, GLfloat);
	void (*Orthof)(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
	void (*Frustumf)(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);

	void (*GetFloatv)(GLenum, GLfloat *);

	void (*TexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *);
	void (*TexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void *);

	void (*GenBuffers)(GLsizei, GLuint *);
	void (*DeleteBuffers)(GLsizei, const GLuint *);
	void (*BindBuffer)(GLenum, GLuint);
	void (*BufferData)(GLenum, GLsizeiptr, const void *, GLenum);

	bool ok;
};

RealGL &real()
{
	static RealGL r = [] {
		RealGL x{};
		x.Enable = resolve<void (*)(GLenum)>("glEnable");
		x.Disable = resolve<void (*)(GLenum)>("glDisable");
		x.BindTexture = resolve<void (*)(GLenum, GLuint)>("glBindTexture");
		x.DepthMask = resolve<void (*)(GLboolean)>("glDepthMask");
		x.BlendFunc = resolve<void (*)(GLenum, GLenum)>("glBlendFunc");
		x.AlphaFunc = resolve<void (*)(GLenum, GLfloat)>("glAlphaFunc");
		x.ShadeModel = resolve<void (*)(GLenum)>("glShadeModel");
		x.TexParameteri = resolve<void (*)(GLenum, GLenum, GLint)>("glTexParameteri");
		x.Fogf = resolve<void (*)(GLenum, GLfloat)>("glFogf");
		x.Fogfv = resolve<void (*)(GLenum, const GLfloat *)>("glFogfv");
		x.Lightfv = resolve<void (*)(GLenum, GLenum, const GLfloat *)>("glLightfv");
		x.LightModelfv = resolve<void (*)(GLenum, const GLfloat *)>("glLightModelfv");
		x.LineWidth = resolve<void (*)(GLfloat)>("glLineWidth");
		x.PolygonOffset = resolve<void (*)(GLfloat, GLfloat)>("glPolygonOffset");
		x.ColorMask = resolve<void (*)(GLboolean, GLboolean, GLboolean, GLboolean)>("glColorMask");
		x.ClearColor = resolve<void (*)(GLfloat, GLfloat, GLfloat, GLfloat)>("glClearColor");
		x.CullFace = resolve<void (*)(GLenum)>("glCullFace");
		x.Clear = resolve<void (*)(GLbitfield)>("glClear");
		x.TexImage2D = resolve<void (*)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *)>("glTexImage2D");
		x.TexSubImage2D = resolve<void (*)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void *)>("glTexSubImage2D");
		x.Color4f = resolve<void (*)(GLfloat, GLfloat, GLfloat, GLfloat)>("glColor4f");
		x.Normal3f = resolve<void (*)(GLfloat, GLfloat, GLfloat)>("glNormal3f");

		x.EnableClientState = resolve<void (*)(GLenum)>("glEnableClientState");
		x.DisableClientState = resolve<void (*)(GLenum)>("glDisableClientState");
		x.VertexPointer = resolve<void (*)(GLint, GLenum, GLsizei, const void *)>("glVertexPointer");
		x.TexCoordPointer = resolve<void (*)(GLint, GLenum, GLsizei, const void *)>("glTexCoordPointer");
		x.ColorPointer = resolve<void (*)(GLint, GLenum, GLsizei, const void *)>("glColorPointer");
		x.NormalPointer = resolve<void (*)(GLenum, GLsizei, const void *)>("glNormalPointer");
		x.DrawArrays = resolve<void (*)(GLenum, GLint, GLsizei)>("glDrawArrays");

		x.MatrixMode = resolve<void (*)(GLenum)>("glMatrixMode");
		x.LoadIdentity = resolve<void (*)(void)>("glLoadIdentity");
		x.PushMatrix = resolve<void (*)(void)>("glPushMatrix");
		x.PopMatrix = resolve<void (*)(void)>("glPopMatrix");
		x.Translatef = resolve<void (*)(GLfloat, GLfloat, GLfloat)>("glTranslatef");
		x.Rotatef = resolve<void (*)(GLfloat, GLfloat, GLfloat, GLfloat)>("glRotatef");
		x.Scalef = resolve<void (*)(GLfloat, GLfloat, GLfloat)>("glScalef");
		x.Orthof = resolve<void (*)(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat)>("glOrthof");
		x.Frustumf = resolve<void (*)(GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat)>("glFrustumf");

		x.GetFloatv = resolve<void (*)(GLenum, GLfloat *)>("glGetFloatv");
		x.LoadMatrixf = resolve<void (*)(const GLfloat *)>("glLoadMatrixf");

		x.GenBuffers = resolve<void (*)(GLsizei, GLuint *)>("glGenBuffers");
		x.DeleteBuffers = resolve<void (*)(GLsizei, const GLuint *)>("glDeleteBuffers");
		x.BindBuffer = resolve<void (*)(GLenum, GLuint)>("glBindBuffer");
		x.BufferData = resolve<void (*)(GLenum, GLsizeiptr, const void *, GLenum)>("glBufferData");

		x.ok = x.DrawArrays && x.MatrixMode && x.GetFloatv && x.Enable;
		return x;
	}();
	return r;
}

// ─── Word packing ─────────────────────────────────────────────────────────────

using Words = std::vector<uint32_t>;

uint32_t packFloat(float f)
{
	uint32_t u = 0;
	std::memcpy(&u, &f, sizeof(u));
	return u;
}

float unpackFloat(uint32_t u)
{
	float f = 0.0f;
	std::memcpy(&f, &u, sizeof(f));
	return f;
}

void pushFloat(Words &w, double d) { w.push_back(packFloat(static_cast<float>(d))); }
void pushInt(Words &w, int32_t i) { w.push_back(static_cast<uint32_t>(i)); }
void pushEnum(Words &w, GLenum e) { w.push_back(static_cast<uint32_t>(e)); }

// Append raw bytes as little-endian words, zero-padded. Returns padded length.
size_t pushBytes(Words &w, const void *data, size_t len)
{
	const uint8_t *p = static_cast<const uint8_t *>(data);
	size_t i = 0;
	for (; i + sizeof(uint32_t) <= len; i += sizeof(uint32_t))
	{
		uint32_t u;
		std::memcpy(&u, p + i, sizeof(u));
		w.push_back(u);
	}
	if (i < len)
	{
		uint32_t u = 0;
		std::memcpy(&u, p + i, len - i);
		w.push_back(u);
	}
	return (len + sizeof(uint32_t) - 1) & ~(sizeof(uint32_t) - 1);
}

enum Op : uint32_t
{
	OP_END_STREAM = 0,

	OP_ENABLE,
	OP_DISABLE,
	OP_BIND_TEXTURE,
	OP_DEPTH_MASK,
	OP_BLEND_FUNC,
	OP_ALPHA_FUNC,
	OP_SHADE_MODEL,
	OP_TEX_PARAMETERI,
	OP_FOGF,
	OP_FOGFV,
	OP_LIGHTFV,
	OP_LIGHT_MODELFV,
	OP_COLOR4F,
	OP_LINE_WIDTH,
	OP_POLYGON_OFFSET,
	OP_COLOR_MASK,
	OP_CLEAR_COLOR,
	OP_CULL_FACE,
	OP_CLEAR,

	OP_MATRIX_MODE,
	OP_LOAD_IDENTITY,
	OP_PUSH_MATRIX,
	OP_POP_MATRIX,
	OP_TRANSLATEF,
	OP_ROTATEF,
	OP_SCALEF,
	OP_ORTHO,
	OP_FRUSTUM,

	OP_DRAWARRAYS_SNAPSHOT,
	OP_BEGIN_BLOCK,
	OP_CALL_LIST,
	OP_CALL_LISTS,

	OP_COUNT
};

// Header packing: 8 bits op + 24 bits argc. The old 16/16 split truncated
// argc for chunk draw snapshots (>65535 words), desyncing the interpreter so
// it swallowed POP_MATRIX/END_STREAM and drifted the matrix stack.
inline uint32_t opHeader(Op op, size_t argc)
{
	return static_cast<uint32_t>(op) | (static_cast<uint32_t>(argc) << 8);
}

// ═════════════════════════════════════════════════════════════════════════════
// Display list storage
// ═════════════════════════════════════════════════════════════════════════════

std::map<GLuint, Words> &lists()
{
	static std::map<GLuint, Words> m;
	return m;
}

// ── Baked display lists: replay without re-interpreting the command stream.
// The interpreter walk + per-draw std::vector/memcpy of vertex snapshots cost
// more than the actual GL work on SGX543. Bake once into stable buffers;
// replays become pure VertexPointer/DrawArrays calls.
struct BakedAttr
{
	uint8_t idx;
	GLint size;
	GLenum type;
	GLsizei stride;
	size_t offset; // into pool
};

struct BakedCmd
{
	enum Kind : uint8_t
	{
		K_BINDTEX,
		K_ENABLE,
		K_DISABLE,
		K_DRAW,
		K_RAW,   // any other state op, replayed verbatim
		K_NESTED // child display list, played recursively
	} kind;
	GLenum a = 0;   // target / cap / draw mode / nested list id / nested list id
	GLuint b = 0;   // texture id
	GLsizei count = 0;
	uint32_t header = 0;
	std::vector<uint32_t> rawWords; // K_RAW payload (header included)
	uint8_t attrMask = 0;
	BakedAttr attrs[4];
};

struct BakedList
{
	bool ready = false;
	bool unbakeable = false; // contains nested calls or unknown ops
	std::vector<uint8_t> pool;
	std::vector<BakedCmd> cmds;
};

std::map<GLuint, BakedList> &bakedLists()
{
	static std::map<GLuint, BakedList> m;
	return m;
}

void invalidateBaked(GLuint list) { bakedLists().erase(list); }

GLuint g_nextListId = 1;
GLuint g_recordingList = 0;

bool recording()
{
	return g_recordingList != 0;
}

Words *recordingStream()
{
	auto it = lists().find(g_recordingList);
	return it != lists().end() ? &it->second : nullptr;
}

// ═════════════════════════════════════════════════════════════════════════════
// Software matrix mirror (answers glGetFloatv, tracks stacks beyond what the
// hardware may guarantee)
// ═════════════════════════════════════════════════════════════════════════════

struct Mat4
{
	float m[16];
};

Mat4 matIdentity()
{
	Mat4 r{{}};
	r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
	return r;
}

Mat4 matMultiply(const Mat4 &a, const Mat4 &b)
{
	Mat4 r{{}};
	for (int col = 0; col < 4; ++col)
		for (int row = 0; row < 4; ++row)
			for (int k = 0; k < 4; ++k)
				r.m[col * 4 + row] += a.m[k * 4 + row] * b.m[col * 4 + k];
	return r;
}

const int kMaxMatrixDepth = 16; // was 64 — real ES overflows at 2-16, mirror desync -> frustum wrong

struct MatrixState
{
	int modeIdx = 0;
	Mat4 cur[4];
	std::vector<Mat4> stack[4];
	// Pushes the real ES stack refused (depth limit). The matching pops must
	// also be ignored, or mirror and real GL drift apart.
	int refused[4] = {0};
	int maxDepth[4] = {0};

	// Real OpenGL ES 1.1 stack depths: modelview 16, projection/texture 2.
	static int depthLimit(int idx) { return idx == 0 ? 16 : 2; }

	static int index(GLenum mode)
	{
		switch (mode)
		{
			case GL_PROJECTION: return 1;
			case GL_TEXTURE: return 2;
			case GL_COLOR: return 3;
			default: return 0;
		}
	}
};

MatrixState g_mat;

// ES 1.1 real matrix stacks are shallow (modelview 16, others 2) while the
// game nests far deeper; refusing pushes broke everything drawn afterwards.
// Keep the software mirror authoritative and force it into the real GL after
// every change. The hardware stack is no longer used.
void syncRealMatrix()
{
	if (real().LoadMatrixf)
		real().LoadMatrixf(g_mat.cur[g_mat.modeIdx].m);
}

void noteDeepPush(const void *ra)
{
	static std::atomic<int> dumps{0};
	if (dumps.fetch_add(1) < 2)
	{
		void *bt[40];
		int n = backtrace(bt, 40);
		fprintf(stderr, "Beta++: DEEPPUSH ra=%p depth=%d bt=%d\n", ra, (int)g_mat.stack[g_mat.modeIdx].size(), n);
		backtrace_symbols_fd(bt, n, 2);
	}
}

void resetMatrices()
{
	g_mat.modeIdx = 0;
	for (int i = 0; i < 4; ++i)
	{
		g_mat.cur[i] = matIdentity();
		g_mat.stack[i].clear();
	}
}

bool isMatrixPname(GLenum pname)
{
	return pname == GL_MODELVIEW_MATRIX || pname == GL_PROJECTION_MATRIX || pname == GL_TEXTURE_MATRIX;
}

// ═════════════════════════════════════════════════════════════════════════════
// Client array tracking
// ═════════════════════════════════════════════════════════════════════════════

enum ArrIdx
{
	ARR_VERTEX = 0,
	ARR_TEXCOORD,
	ARR_COLOR,
	ARR_NORMAL,
	ARR_COUNT
};

struct ArraySpec
{
	GLint size = 0;
	GLenum type = 0;
	GLsizei stride = 0;
	const void *pointer = nullptr;
	bool enabled = false;
};

ArraySpec g_arrays[ARR_COUNT];

void resetClientState()
{
	for (int i = 0; i < ARR_COUNT; ++i)
		g_arrays[i] = ArraySpec{};
}

int arrIndex(GLenum array)
{
	switch (array)
	{
		case GL_VERTEX_ARRAY: return ARR_VERTEX;
		case GL_TEXTURE_COORD_ARRAY: return ARR_TEXCOORD;
		case GL_COLOR_ARRAY: return ARR_COLOR;
		case GL_NORMAL_ARRAY: return ARR_NORMAL;
		default: return -1;
	}
}

size_t arrayTypeSize(GLenum type)
{
	switch (type)
	{
		case GL_BYTE:
		case GL_UNSIGNED_BYTE: return 1;
		case GL_SHORT:
		case GL_UNSIGNED_SHORT: return 2;
		default: return 4;
	}
}

size_t arrayStride(const ArraySpec &a)
{
	if (a.stride != 0)
		return static_cast<size_t>(a.stride);
	return static_cast<size_t>(a.size) * arrayTypeSize(a.type);
}

uint32_t enabledMask()
{
	uint32_t m = 0;
	for (int i = 0; i < ARR_COUNT; ++i)
		if (g_arrays[i].enabled)
			m |= (1u << i);
	return m;
}

void applyEnabledMask(uint32_t want)
{
	static const GLenum names[ARR_COUNT] = {
		GL_VERTEX_ARRAY, GL_TEXTURE_COORD_ARRAY, GL_COLOR_ARRAY, GL_NORMAL_ARRAY};
	uint32_t have = enabledMask();
	uint32_t diff = have ^ want;
	for (int i = 0; i < ARR_COUNT; ++i)
	{
		if (!(diff & (1u << i)))
			continue;
		if (want & (1u << i))
		{
			g_arrays[i].enabled = true;
			if (real().EnableClientState)
				real().EnableClientState(names[i]);
		}
		else
		{
			g_arrays[i].enabled = false;
			if (real().DisableClientState)
				real().DisableClientState(names[i]);
		}
	}
}

// ═════════════════════════════════════════════════════════════════════════════
// Immediate mode
// ═════════════════════════════════════════════════════════════════════════════

struct ImmVertex
{
	float x, y, z;
	float s, t;
	uint32_t color;
	float nx, ny, nz;
	bool hasTex, hasNrm;
};

std::vector<ImmVertex> g_immVerts;
GLenum g_immMode = 0;
bool g_immActive = false;

// Stream-level attribute flags: set when glTexCoord2f/glNormal3f are seen
// inside an active Begin block (they may precede the first vertex).
bool g_streamHasTex = false;
bool g_streamHasNrm = false;

float g_curColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
float g_curTexCoord[2] = {0.0f, 0.0f};
float g_curNormal[3] = {0.0f, 0.0f, 1.0f};

void resetImmediate()
{
	g_immVerts.clear();
	g_immMode = 0;
	g_immActive = false;
	g_streamHasTex = false;
	g_streamHasNrm = false;
	g_curColor[0] = g_curColor[1] = g_curColor[2] = g_curColor[3] = 1.0f;
	g_curTexCoord[0] = g_curTexCoord[1] = 0.0f;
	g_curNormal[0] = 0.0f;
	g_curNormal[1] = 0.0f;
	g_curNormal[2] = 1.0f;
}

uint32_t colorToU32(const float c[4])
{
	auto q = [](float f) -> uint32_t {
		if (f <= 0.0f) return 0;
		if (f >= 1.0f) return 255;
		return static_cast<uint32_t>(f * 255.0f + 0.5f);
	};
	return q(c[0]) | (q(c[1]) << 8) | (q(c[2]) << 16) | (q(c[3]) << 24); // RGBA bytes
}

// Shared vertex-array draw used by live immediate mode and BEGIN_BLOCK replay.
// Builds CPU arrays from vertices, converts quads to triangles, saves/restores
// ambient client state around the draw.
void drawVertices(GLenum mode, const ImmVertex *v, GLsizei count)
{
	if (!real().ok || v == nullptr || count <= 0)
		return;
	g_dbg.immFlushes++;
	g_dbg.immVerts += static_cast<unsigned long>(count);

	// Expand quads to triangles (same winding the Tessellator produces).
	std::vector<ImmVertex> expanded;
	if (mode == GL_QUADS)
	{
		expanded.reserve(count / 4 * 6);
		for (GLsizei i = 0; i + 3 < count; i += 4)
		{
			expanded.push_back(v[i + 0]);
			expanded.push_back(v[i + 1]);
			expanded.push_back(v[i + 2]);
			expanded.push_back(v[i + 0]);
			expanded.push_back(v[i + 2]);
			expanded.push_back(v[i + 3]);
		}
		v = expanded.data();
		count = static_cast<GLsizei>(expanded.size());
		mode = GL_TRIANGLES;
		if (count == 0)
			return;
	}

	bool hasTex = false, hasNrm = false;
	for (GLsizei i = 0; i < count; ++i)
	{
		hasTex |= v[i].hasTex;
		hasNrm |= v[i].hasNrm;
	}

	std::vector<float> pos(count * 3);
	std::vector<float> tex(hasTex ? count * 2 : 0);
	std::vector<uint8_t> col(count * 4); // colour always present (defaults white)
	std::vector<float> nrm(hasNrm ? count * 3 : 0);

	for (GLsizei i = 0; i < count; ++i)
	{
		pos[i * 3 + 0] = v[i].x;
		pos[i * 3 + 1] = v[i].y;
		pos[i * 3 + 2] = v[i].z;
		if (hasTex)
		{
			tex[i * 2 + 0] = v[i].s;
			tex[i * 2 + 1] = v[i].t;
		}
		col[i * 4 + 0] = static_cast<uint8_t>((v[i].color >> 0) & 0xff);
		col[i * 4 + 1] = static_cast<uint8_t>((v[i].color >> 8) & 0xff);
		col[i * 4 + 2] = static_cast<uint8_t>((v[i].color >> 16) & 0xff);
		col[i * 4 + 3] = static_cast<uint8_t>((v[i].color >> 24) & 0xff);
		if (hasNrm)
		{
			nrm[i * 3 + 0] = v[i].nx;
			nrm[i * 3 + 1] = v[i].ny;
			nrm[i * 3 + 2] = v[i].nz;
		}
	}

	uint32_t savedMask = enabledMask();

	real().VertexPointer(3, GL_FLOAT, 0, pos.data());
	applyEnabledMask(savedMask | (1u << ARR_VERTEX));
	if (hasTex)
	{
		real().TexCoordPointer(2, GL_FLOAT, 0, tex.data());
		applyEnabledMask(enabledMask() | (1u << ARR_TEXCOORD));
	}
	real().ColorPointer(4, GL_UNSIGNED_BYTE, 0, col.data());
	applyEnabledMask(enabledMask() | (1u << ARR_COLOR));
	if (hasNrm)
	{
		real().NormalPointer(GL_FLOAT, 0, nrm.data());
		applyEnabledMask(enabledMask() | (1u << ARR_NORMAL));
	}

	real().DrawArrays(mode, 0, count);
	applyEnabledMask(savedMask);
}

// ═════════════════════════════════════════════════════════════════════════════
// Recording emitters
// ═════════════════════════════════════════════════════════════════════════════

Words &emit(Op op, size_t argc)
{
	Words &w = *recordingStream();
	w.push_back(opHeader(op, argc));
	return w;
}

void emitSimple(Op op, std::initializer_list<uint32_t> args)
{
	Words &w = emit(op, args.size());
	for (uint32_t a : args)
		w.push_back(a);
}

void emitDrawArraysSnapshot(GLenum mode, GLint first, GLsizei count)
{
	Words &w = *recordingStream();

	// The Tessellator's arrays are interleaved slices of ONE vertex buffer
	// (stride 32, different base offsets). Copying count*stride bytes per
	// array stored the same records four times; copy the union span once and
	// keep per-array offsets. ~4x less geometry RAM per chunk.
	struct Att
	{
		int idx;
		GLint size;
		GLenum type;
		GLsizei stride;
		const uint8_t *ptr;
		size_t len;
	};
	Att atts[4];
	int natt = 0;
	const uint8_t *lo = nullptr;
	const uint8_t *hi = nullptr;

	for (int i = 0; i < ARR_COUNT; ++i)
	{
		const ArraySpec &a = g_arrays[i];
		if (!a.enabled || a.pointer == nullptr || a.size == 0 || count <= 0)
			continue;

		size_t stride = arrayStride(a);
		size_t offset = static_cast<size_t>(first) * stride;
		size_t len = static_cast<size_t>(count) * stride;
		const uint8_t *p = static_cast<const uint8_t *>(a.pointer) + offset;

		atts[natt].idx = i;
		atts[natt].size = a.size;
		atts[natt].type = a.type;
		atts[natt].stride = static_cast<GLsizei>(stride);
		atts[natt].ptr = p;
		atts[natt].len = len;
		natt++;

		if (lo == nullptr || p < lo)
			lo = p;
		if (hi == nullptr || p + len > hi)
			hi = p + len;
	}
	if (natt == 0)
		return;

	// The union-span copy is only valid when all arrays are interleaved slices
	// of ONE buffer (the Tessellator: same stride, small offsets). If they come
	// from different buffers, [lo,hi) would splice unrelated memory between them
	// and replay garbage vertices (jumbled triangles / bogus colours). Detect
	// that case and pack each array's bytes contiguously instead.
	size_t maxStride = 0;
	for (int k = 0; k < natt; ++k)
		if ((size_t)atts[k].stride > maxStride)
			maxStride = (size_t)atts[k].stride;

	bool interleaved = true;
	for (int k = 0; k < natt; ++k)
		if ((size_t)(atts[k].ptr - lo) >= maxStride)
		{
			interleaved = false;
			break;
		}

	size_t argcPos = w.size();
	w.push_back(0); // header placeholder

	size_t start = w.size();
	w.push_back(mode);
	pushInt(w, first);
	pushInt(w, count);
	pushInt(w, natt);

	if (interleaved)
	{
		size_t span = static_cast<size_t>(hi - lo);
		pushInt(w, static_cast<int32_t>(span));
		pushBytes(w, lo, span);
		for (int k = 0; k < natt; ++k)
		{
			pushInt(w, atts[k].idx);
			pushInt(w, atts[k].size);
			pushEnum(w, atts[k].type);
			pushInt(w, atts[k].stride);
			pushInt(w, static_cast<int32_t>(atts[k].ptr - lo));
		}
	}
	else
	{
		std::vector<uint8_t> blob;
		int32_t offs[4] = {0, 0, 0, 0};
		for (int k = 0; k < natt; ++k)
		{
			offs[k] = static_cast<int32_t>(blob.size());
			blob.insert(blob.end(), atts[k].ptr, atts[k].ptr + atts[k].len);
		}
		pushInt(w, static_cast<int32_t>(blob.size()));
		if (!blob.empty())
			pushBytes(w, blob.data(), blob.size());
		for (int k = 0; k < natt; ++k)
		{
			pushInt(w, atts[k].idx);
			pushInt(w, atts[k].size);
			pushEnum(w, atts[k].type);
			pushInt(w, atts[k].stride);
			pushInt(w, offs[k]);
		}
	}

	w[argcPos] = opHeader(OP_DRAWARRAYS_SNAPSHOT, w.size() - start);
}

void emitBeginBlock(GLenum mode, const std::vector<ImmVertex> &verts)
{
	Words &w = *recordingStream();
	size_t argcPos = w.size();
	w.push_back(0);

	size_t start = w.size();
	w.push_back(mode);
	pushInt(w, static_cast<int32_t>(verts.size()));
	for (const ImmVertex &vv : verts)
	{
		pushFloat(w, vv.x);
		pushFloat(w, vv.y);
		pushFloat(w, vv.z);
		pushFloat(w, vv.s);
		pushFloat(w, vv.t);
		w.push_back(vv.color);
		pushFloat(w, vv.nx);
		pushFloat(w, vv.ny);
		pushFloat(w, vv.nz);
		w.push_back((vv.hasTex ? 1u : 0u) | (vv.hasNrm ? 2u : 0u));
	}

	w[argcPos] = opHeader(OP_BEGIN_BLOCK, w.size() - start);
}

// ═════════════════════════════════════════════════════════════════════════════
// Replay engine
// ═════════════════════════════════════════════════════════════════════════════

struct Reader
{
	const uint32_t *p;
	const uint32_t *end;
	int depth;
	bool bad = false;

	uint32_t u32()
	{
		if (p >= end)
		{
			bad = true;
			return 0;
		}
		return *p++;
	}
	float f32() { return unpackFloat(u32()); }
	int32_t i32() { return static_cast<int32_t>(u32()); }
	GLenum e32() { return static_cast<GLenum>(u32()); }
	bool b32() { return u32() != 0; }
};

void replayList(GLuint list, int depth);

void executeOp(uint32_t header, Reader &r)
{
	Op op = static_cast<Op>(header & 0xFFu);
	switch (op)
	{
		case OP_END_STREAM:
			r.bad = true;
			break;

		case OP_ENABLE:
			if (real().Enable) real().Enable(r.e32());
			break;
		case OP_DISABLE:
			if (real().Disable) real().Disable(r.e32());
			break;
		case OP_BIND_TEXTURE:
		{
			GLenum t = r.e32();
			GLuint id = r.u32();
			if (real().BindTexture) real().BindTexture(t, id);
			break;
		}
		case OP_DEPTH_MASK:
			if (real().DepthMask) real().DepthMask(r.b32() ? GL_TRUE : GL_FALSE);
			break;
		case OP_BLEND_FUNC:
		{
			GLenum s = r.e32(), d = r.e32();
			if (real().BlendFunc) real().BlendFunc(s, d);
			break;
		}
		case OP_ALPHA_FUNC:
		{
			GLenum fn = r.e32();
			GLfloat ref = r.f32();
			if (real().AlphaFunc) real().AlphaFunc(fn, ref);
			break;
		}
		case OP_SHADE_MODEL:
			if (real().ShadeModel) real().ShadeModel(r.e32());
			break;
		case OP_TEX_PARAMETERI:
		{
			GLenum t = r.e32(), pn = r.e32();
			GLint v = r.i32();
			if (real().TexParameteri) real().TexParameteri(t, pn, v);
			break;
		}
		case OP_FOGF:
		{
			GLenum pn = r.e32();
			GLfloat v = r.f32();
			if (real().Fogf) real().Fogf(pn, v);
			break;
		}
		case OP_FOGFV:
		{
			GLenum pn = r.e32();
			GLfloat vals[4] = {r.f32(), r.f32(), r.f32(), r.f32()};
			if (real().Fogfv) real().Fogfv(pn, vals);
			break;
		}
		case OP_LIGHTFV:
		{
			GLenum light = r.e32(), pname = r.e32();
			GLfloat vals[4] = {r.f32(), r.f32(), r.f32(), r.f32()};
			if (real().Lightfv) real().Lightfv(light, pname, vals);
			break;
		}
		case OP_LIGHT_MODELFV:
		{
			GLenum pname = r.e32();
			GLfloat vals[4] = {r.f32(), r.f32(), r.f32(), r.f32()};
			if (real().LightModelfv) real().LightModelfv(pname, vals);
			break;
		}
		case OP_COLOR4F:
		{
			float cr = r.f32();
			float cg = r.f32();
			float cb = r.f32();
			float ca = r.f32();
			if (real().Color4f) real().Color4f(cr, cg, cb, ca);
			break;
		}
		case OP_LINE_WIDTH:
		{
			GLfloat wd = r.f32();
			if (real().LineWidth) real().LineWidth(wd);
			break;
		}
		case OP_POLYGON_OFFSET:
		{
			GLfloat fa = r.f32(), fb = r.f32();
			if (real().PolygonOffset) real().PolygonOffset(fa, fb);
			break;
		}
		case OP_COLOR_MASK:
		{
			GLboolean a = r.b32() ? GL_TRUE : GL_FALSE;
			GLboolean b = r.b32() ? GL_TRUE : GL_FALSE;
			GLboolean c = r.b32() ? GL_TRUE : GL_FALSE;
			GLboolean d = r.b32() ? GL_TRUE : GL_FALSE;
			if (real().ColorMask) real().ColorMask(a, b, c, d);
			break;
		}
		case OP_CLEAR_COLOR:
		{
			GLfloat a = r.f32(), b = r.f32(), c = r.f32(), d = r.f32();
			if (real().ClearColor) real().ClearColor(a, b, c, d);
			break;
		}
		case OP_CULL_FACE:
			if (real().CullFace) real().CullFace(r.e32());
			break;
		case OP_CLEAR:
			if (real().Clear) real().Clear(r.u32());
			break;

		case OP_MATRIX_MODE:
		{
			GLenum m = r.e32();
			g_mat.modeIdx = MatrixState::index(m);
			if (real().MatrixMode) real().MatrixMode(m);
			break;
		}
		case OP_LOAD_IDENTITY:
			g_mat.cur[g_mat.modeIdx] = matIdentity();
			syncRealMatrix();
			break;
		case OP_PUSH_MATRIX:
		{
			int i = g_mat.modeIdx;
			g_mat.stack[i].push_back(g_mat.cur[i]);
			if ((int)g_mat.stack[i].size() > g_mat.maxDepth[i])
				g_mat.maxDepth[i] = (int)g_mat.stack[i].size();
			if ((int)g_mat.stack[i].size() > 64)
				noteDeepPush(__builtin_return_address(0));
			break;
		}
		case OP_POP_MATRIX:
		{
			int i = g_mat.modeIdx;
			if (!g_mat.stack[i].empty())
			{
				g_mat.cur[i] = g_mat.stack[i].back();
				g_mat.stack[i].pop_back();
				syncRealMatrix();
			}
			break;
		}
		case OP_TRANSLATEF:
		{
			float x = r.f32(), y = r.f32(), z = r.f32();
			Mat4 t = matIdentity();
			t.m[12] = x;
			t.m[13] = y;
			t.m[14] = z;
			int i = g_mat.modeIdx;
			g_mat.cur[i] = matMultiply(g_mat.cur[i], t);
			syncRealMatrix();
			break;
		}
		case OP_ROTATEF:
		{
			float ang = r.f32(), x = r.f32(), y = r.f32(), z = r.f32();
			float len = std::sqrt(x * x + y * y + z * z);
			if (len > 0.0f)
			{
				x /= len; y /= len; z /= len;
				float rad = ang * 3.14159265358979323846f / 180.0f;
				float c = std::cos(rad), s = std::sin(rad), ic = 1.0f - c;
				Mat4 m = matIdentity();
				m.m[0] = x * x * ic + c;
				m.m[1] = y * x * ic + z * s;
				m.m[2] = x * z * ic - y * s;
				m.m[4] = x * y * ic - z * s;
				m.m[5] = y * y * ic + c;
				m.m[6] = y * z * ic + x * s;
				m.m[8] = x * z * ic + y * s;
				m.m[9] = y * z * ic - x * s;
				m.m[10] = z * z * ic + c;
				int i = g_mat.modeIdx;
				g_mat.cur[i] = matMultiply(g_mat.cur[i], m);
			}
			syncRealMatrix();
			break;
		}
		case OP_SCALEF:
		{
			float x = r.f32(), y = r.f32(), z = r.f32();
			Mat4 m = matIdentity();
			m.m[0] = x;
			m.m[5] = y;
			m.m[10] = z;
			int i = g_mat.modeIdx;
			g_mat.cur[i] = matMultiply(g_mat.cur[i], m);
			syncRealMatrix();
			break;
		}
		case OP_ORTHO:
		{
			double l = r.f32(), rr = r.f32(), b = r.f32(), t = r.f32(), n = r.f32(), f = r.f32();
			Mat4 o{{}};
			o.m[0] = static_cast<float>(2.0 / (rr - l));
			o.m[5] = static_cast<float>(2.0 / (t - b));
			o.m[10] = static_cast<float>(-2.0 / (f - n));
			o.m[12] = static_cast<float>(-(rr + l) / (rr - l));
			o.m[13] = static_cast<float>(-(t + b) / (t - b));
			o.m[14] = static_cast<float>(-(f + n) / (f - n));
			o.m[15] = 1.0f;
			int i = g_mat.modeIdx;
			g_mat.cur[i] = matMultiply(g_mat.cur[i], o);
			syncRealMatrix();
			break;
		}
		case OP_FRUSTUM:
		{
			double l = r.f32(), rr = r.f32(), b = r.f32(), t = r.f32(), n = r.f32(), f = r.f32();
			Mat4 fr{{}};
			fr.m[0] = static_cast<float>(2.0 * n / (rr - l));
			fr.m[5] = static_cast<float>(2.0 * n / (t - b));
			fr.m[8] = static_cast<float>((rr + l) / (rr - l));
			fr.m[9] = static_cast<float>((t + b) / (t - b));
			fr.m[10] = static_cast<float>(-(f + n) / (f - n));
			fr.m[11] = -1.0f;
			fr.m[14] = static_cast<float>(-2.0 * f * n / (f - n));
			int i = g_mat.modeIdx;
			g_mat.cur[i] = matMultiply(g_mat.cur[i], fr);
			syncRealMatrix();
			break;
		}

		case OP_DRAWARRAYS_SNAPSHOT:
		{
			// Arrays are offsets into ONE union blob (interleaved Tessellator
			// buffer), so vertex data is stored once instead of four times.
			GLenum mode = r.e32();
			GLint first = r.i32();
			GLsizei count = r.i32();
			int32_t natt = r.i32();
			int32_t span = r.i32();
			(void)first;
			if (span < 0 || natt <= 0)
				break;
			const uint8_t *unionBase = reinterpret_cast<const uint8_t *>(r.p);
			size_t spanWords = ((size_t)span + sizeof(uint32_t) - 1) / sizeof(uint32_t);
			if (r.p + spanWords > r.end)
				break;
			r.p += spanWords;

			const uint8_t *ptrs[4] = {nullptr, nullptr, nullptr, nullptr};
			GLint sizes[4] = {0, 0, 0, 0};
			GLenum types[4] = {0, 0, 0, 0};
			GLsizei strides[4] = {0, 0, 0, 0};

			for (int k = 0; k < natt && k < 8 && !r.bad; ++k)
			{
				int idx = r.i32();
				GLint size = r.i32();
				GLenum type = r.e32();
				GLsizei stride = r.i32();
				int32_t off = r.i32();
				if (idx < 0 || idx >= 4 || off < 0)
					continue;
				ptrs[idx] = unionBase + off;
				sizes[idx] = size;
				types[idx] = type;
				strides[idx] = stride;
			}
			if (r.bad || !real().ok)
				break;

			uint32_t savedMask = enabledMask();
			for (int i = 0; i < 4; ++i)
			{
				if (!ptrs[i])
					continue;
				switch (i)
				{
					case ARR_VERTEX: real().VertexPointer(sizes[i], types[i], strides[i], ptrs[i]); break;
					case ARR_TEXCOORD: real().TexCoordPointer(sizes[i], types[i], strides[i], ptrs[i]); break;
					case ARR_COLOR: real().ColorPointer(sizes[i], types[i], strides[i], ptrs[i]); break;
					case ARR_NORMAL: real().NormalPointer(types[i], strides[i], ptrs[i]); break;
				}
				applyEnabledMask(enabledMask() | (1u << i));
			}

			// Batch large draws: some ES1 drivers silently clamp very large
			// DrawArrays counts, which showed up as partially drawn chunks.
			{
				GLsizei remaining = count;
				GLint base = 0;
				const GLsizei kBatch = 16384;
				while (remaining > 0 && !r.bad)
				{
					GLsizei n = remaining > kBatch ? kBatch : remaining;
					real().DrawArrays(mode, base, n);
					base += n;
					remaining -= n;
				}
			}
			applyEnabledMask(savedMask);
			break;
		}

		case OP_BEGIN_BLOCK:
		{
			GLenum mode = r.e32();
			int32_t count = r.i32();
			if (count <= 0)
				break;
			static std::vector<ImmVertex> block;
			block.clear();
			block.reserve(static_cast<size_t>(count));
			for (int32_t i = 0; i < count && !r.bad; ++i)
			{
				ImmVertex vv{};
				vv.x = r.f32();
				vv.y = r.f32();
				vv.z = r.f32();
				vv.s = r.f32();
				vv.t = r.f32();
				vv.color = r.u32();
				vv.nx = r.f32();
				vv.ny = r.f32();
				vv.nz = r.f32();
				uint32_t flags = r.u32();
				vv.hasTex = (flags & 1u) != 0;
				vv.hasNrm = (flags & 2u) != 0;
				block.push_back(vv);
			}
			drawVertices(mode, block.data(), static_cast<GLsizei>(block.size()));
			break;
		}

		case OP_CALL_LIST:
		{
			GLuint id = r.u32();
			replayList(id, r.depth + 1);
			break;
		}
		case OP_CALL_LISTS:
		{
			int32_t n = r.i32();
			for (int32_t i = 0; i < n && !r.bad; ++i)
				replayList(r.u32(), r.depth + 1);
			break;
		}

		default:
			r.bad = true;
			break;
	}
}

// Parse one OP_DRAWARRAYS_SNAPSHOT at rd, append vertex bytes to pool and a
// K_DRAW cmd. Mirrors the interpreter case block's field order exactly.
bool bakeDrawSnapshot(Reader &r, std::vector<uint8_t> &pool, BakedCmd &out)
{
	out.kind = BakedCmd::K_DRAW;
	out.a = r.e32();      // mode
	(void)r.i32();        // first — snapshots always start at 0
	out.count = r.i32();
	int32_t natt = r.i32();
	int32_t span = r.i32();
	if (span < 0 || natt <= 0)
		return false;
	size_t spanWords = ((size_t)span + sizeof(uint32_t) - 1) / sizeof(uint32_t);
	if (r.p + spanWords > r.end)
		return false;

	// One union blob per draw; arrays address it by offset.
	size_t unionOffset = pool.size();
	pool.insert(pool.end(), reinterpret_cast<const uint8_t *>(r.p),
	            reinterpret_cast<const uint8_t *>(r.p) + (size_t)span);
	r.p += spanWords;

	out.attrMask = 0;
	for (int k = 0; k < natt && k < 4 && !r.bad; ++k)
	{
		BakedAttr at{};
		at.idx = (uint8_t)r.i32();
		at.size = r.i32();
		at.type = r.e32();
		at.stride = r.i32();
		int32_t off = r.i32();
		if (at.idx >= 4 || off < 0)
			continue;
		at.offset = unionOffset + (size_t)off;
		out.attrs[out.attrMask & 3] = at;
		out.attrMask++;
	}
	return !r.bad;
}

const uint8_t *g_bakePoolBase = nullptr;

void playBaked(const BakedList &bk, int depth);
void bakeAndReplay(GLuint list, Words &w, int depth);

// Resolve a child list reference: bake lazily, then play.
void playListRef(GLuint id, int depth)
{
	if (depth > 32)
		return;
	auto itL = lists().find(id);
	if (itL == lists().end())
		return;
	BakedList &child = bakedLists()[id];
	if (child.ready)
	{
		const uint8_t *saveBase = g_bakePoolBase;
		g_bakePoolBase = child.pool.data();
		playBaked(child, depth + 1);
		g_bakePoolBase = saveBase;
		return;
	}
	bakeAndReplay(id, itL->second, depth + 1);
}

void playBaked(const BakedList &bk, int depth)
{
	g_playLists.fetch_add(1, std::memory_order_relaxed);
	for (const BakedCmd &c : bk.cmds)
	{
		if (c.kind == BakedCmd::K_DRAW)
			g_playDraws.fetch_add(1, std::memory_order_relaxed);
		switch (c.kind)
		{
			case BakedCmd::K_BINDTEX:
				if (real().BindTexture) real().BindTexture(c.a, c.b);
				break;
			case BakedCmd::K_ENABLE:
				if (real().Enable) real().Enable(c.a);
				break;
			case BakedCmd::K_DISABLE:
				if (real().Disable) real().Disable(c.a);
				break;
			case BakedCmd::K_RAW:
			{
				Words tmp = c.rawWords;
				Reader rr{tmp.data(), tmp.data() + tmp.size(), 0};
				executeOp(c.header, rr);
				break;
			}
			case BakedCmd::K_NESTED:
				playListRef(c.a, depth + 1);
				break;
			case BakedCmd::K_DRAW:
			{
				if (!real().ok || c.attrMask == 0)
					break;
				uint32_t saved = enabledMask();
				const uint8_t *base = g_bakePoolBase;
				uint32_t m = 0;
				for (int i = 0; i < c.attrMask; ++i)
				{
					const BakedAttr &at = c.attrs[i];
					const void *ptr = base + at.offset;
					switch (at.idx)
					{
						case ARR_VERTEX: real().VertexPointer(at.size, at.type, at.stride, ptr); break;
						case ARR_TEXCOORD: real().TexCoordPointer(at.size, at.type, at.stride, ptr); break;
						case ARR_COLOR: real().ColorPointer(at.size, at.type, at.stride, ptr); break;
						case ARR_NORMAL: real().NormalPointer(at.type, at.stride, ptr); break;
					}
					m |= (1u << at.idx);
				}
				applyEnabledMask(saved | m);
				real().DrawArrays(c.a, 0, c.count);
				applyEnabledMask(saved);
				break;
			}
		}
	}
}

// Live interpretation (original path) — also the fallback for lists we
// decline to bake (nested calls / parse trouble).
void interpretStream(Words &w, int depth)
{
	Reader rd{w.data(), w.data() + w.size(), depth};
	while (!rd.bad && rd.p < rd.end)
	{
		uint32_t header = rd.u32();
		uint32_t argc = header >> 8;
		if ((header & 0xFFu) == OP_END_STREAM)
			break;
		const uint32_t *argStart = rd.p;
		executeOp(header, rd);
		if (rd.p != argStart + argc)
		{
			static std::atomic<int> mm{0};
			if (mm.fetch_add(1) < 12)
				fprintf(stderr, "Beta++: OPMISMATCH op=%u argc=%u consumed=%d\n",
				        (unsigned)(header & 0xFFu), (unsigned)argc, (int)(rd.p - argStart));
		}
		if (rd.p != argStart + argc)
			rd.p = argStart + argc;
		if (rd.p > rd.end)
			rd.p = rd.end;
	}
}

void bakeAndReplay(GLuint list, Words &w, int depth)
{
	BakedList &bk = bakedLists()[list];
	bk.pool.clear();
	bk.cmds.clear();
	bk.ready = false;

	if (depth > 32)
		return; // cycle guard — drop rather than recurse forever

	Reader rd{w.data(), w.data() + w.size(), 0};
	while (!rd.bad && rd.p < rd.end)
	{
		uint32_t header = rd.u32();
		uint32_t argc = header >> 8;
		uint16_t op16 = header & 0xFFu;
		if (op16 == OP_END_STREAM)
			break;
		if (op16 == OP_CALL_LIST || op16 == OP_CALL_LISTS)
		{
			// Nested child list(s): record references and resolve them at
			// playback so chunk/font hierarchies stay fully baked.
			GLuint first = 0;
			GLsizei nids = 0;
			if (op16 == OP_CALL_LIST)
			{
				first = rd.u32();
				nids = 1;
			}
			else
			{
				nids = rd.i32();
				if (nids < 0 || nids > 4096)
					rd.bad = true;
			}
			for (GLsizei k = 0; k < nids && !rd.bad; ++k)
			{
				BakedCmd c{};
				c.kind = BakedCmd::K_NESTED;
				c.a = (op16 == OP_CALL_LIST) ? (GLuint)first : (GLuint)rd.u32();
				bk.cmds.push_back(std::move(c));
			}
			continue;
		}

		const uint32_t *argStart = rd.p;
		switch (op16)
		{
			case OP_BIND_TEXTURE:
			{
				BakedCmd c{};
				c.kind = BakedCmd::K_BINDTEX;
				c.a = rd.e32();
				c.b = rd.u32();
				bk.cmds.push_back(std::move(c));
				break;
			}
			case OP_ENABLE:
			case OP_DISABLE:
			{
				BakedCmd c{};
				c.kind = (op16 == OP_ENABLE) ? BakedCmd::K_ENABLE : BakedCmd::K_DISABLE;
				c.a = rd.e32();
				bk.cmds.push_back(std::move(c));
				break;
			}
			case OP_DRAWARRAYS_SNAPSHOT:
			{
				BakedCmd c{};
				if (!bakeDrawSnapshot(rd, bk.pool, c))
					rd.bad = true;
				else
					bk.cmds.push_back(std::move(c));
				break;
			}
			default:
			{
				// Preserve every other state op verbatim (color, matrices,
				// fog, ...) — dropping any corrupted rendering.
				BakedCmd c{};
				c.kind = BakedCmd::K_RAW;
				c.header = header;
				size_t n = argc;
				if ((int32_t)n < 0 || rd.p + n > rd.end)
					rd.bad = true;
				else
				{
					c.rawWords.push_back(header);
					c.rawWords.insert(c.rawWords.end(), rd.p, rd.p + n);
					rd.p += n;
					bk.cmds.push_back(std::move(c));
				}
				break;
			}
		}
		if (rd.p != argStart + argc)
			rd.p = argStart + argc;
		if (rd.p > rd.end)
			rd.p = rd.end;
	}

	if (rd.bad || depth > 32)
	{
		bk.ready = false;
		bakedLists().erase(list);
		interpretStream(w, 0); // finish this frame live
		return;
	}

	bk.ready = true;
	static std::atomic<uint32_t> bakeDiag{0};
	if (bakeDiag.fetch_add(1, std::memory_order_relaxed) < 60)
	{
		size_t ndraw = 0, nnest = 0, nraw = 0;
		for (const BakedCmd &c : bk.cmds)
		{
			if (c.kind == BakedCmd::K_DRAW) ndraw++;
			else if (c.kind == BakedCmd::K_NESTED) nnest++;
			else if (c.kind == BakedCmd::K_RAW) nraw++;
		}
		fprintf(stderr,
		        "Beta++: bake id=%u words=%zu cmds=%zu draw=%zu nest=%zu "
		        "raw=%zu pool=%zu\n",
		        list, w.size(), bk.cmds.size(), ndraw, nnest, nraw,
		        bk.pool.size());
	}
	g_bakePoolBase = bk.pool.data();
	playBaked(bk, depth);
}

void replayList(GLuint list, int depth)
{
	if (depth > 32)
		return;
	auto itL = lists().find(list);
	if (itL == lists().end())
		return;

	// One-shot op-stream dump: shows exactly what a recorded chunk list
	// contains (matrix ops + draw) so position bugs are visible, not guessed.
	{
		static std::atomic<uint32_t> dbgDump{0};
		if (depth == 0 && itL->second.size() >= 2000 && dbgDump.fetch_add(1) < 8)
		{
			const Words &w = itL->second;
			fprintf(stderr, "Beta++: RLBIG %u n=%zu head:", list, w.size());
			for (size_t q = 0; q < w.size() && q < 8; q++)
				fprintf(stderr, " %08x", w[q]);
			fprintf(stderr, " tail:");
			for (size_t q = (w.size() > 8 ? w.size() - 8 : 0); q < w.size(); q++)
				fprintf(stderr, " %08x", w[q]);
			fprintf(stderr, "\n");
		}
	}

bakeAndReplayDisabled:
	{
		size_t before = g_mat.stack[0].size();
		interpretStream(itL->second, depth);
		size_t after = g_mat.stack[0].size();
		if (depth == 0 && after != before)
		{
			static std::atomic<int> d {0};
			if (d.fetch_add(1) < 10)
				fprintf(stderr, "Beta++: LISTDEPTH id=%u n=%zu before=%zu after=%zu delta=%d\n",
				        list, itL->second.size(), before, after, (int)(after - before));
		}
	}
	(void)itL;
}

} // namespace

extern "C" {

// ── Display lists ────────────────────────────────────────────────────────────

SHIM_HIDDEN GLuint APIENTRY glGenLists(GLsizei range)
{
	if (range <= 0)
		return 0;
	GLuint first = g_nextListId;
	g_nextListId += static_cast<GLuint>(range);
	for (GLuint id = first; id < first + static_cast<GLuint>(range); ++id)
		lists()[id]; // reserve empty stream
	return first;
}

SHIM_HIDDEN void APIENTRY glDeleteLists(GLuint list, GLsizei range)
{
	for (GLuint id = list; id < list + static_cast<GLuint>(range); ++id)
	{
		lists().erase(id);
		bakedLists().erase(id);
	}
}

SHIM_HIDDEN GLboolean APIENTRY glIsList(GLuint list)
{
	return lists().find(list) != lists().end() ? GL_TRUE : GL_FALSE;
}

SHIM_HIDDEN void APIENTRY glNewList(GLuint list, GLenum /*mode*/)
{
	g_dbg.listCompiles++;
	if (list == 0 || recording())
		return; // no nested compiles — the game never does this
	lists()[list].clear();
	invalidateBaked(list);
	g_recordingList = list;
}

SHIM_HIDDEN void APIENTRY glEndList(void)
{
	if (!recording())
		return;
	GLuint finished = g_recordingList;
	recordingStream()->push_back(opHeader(OP_END_STREAM, 0));
	{
		static std::atomic<int> dd{0};
		const Words &w = lists()[finished];
		bool startsPush = !w.empty() && (w[0] & 0xFFu) == 22u; // OP_PUSH_MATRIX
		if (startsPush && w.size() >= 50 && dd.fetch_add(1) < 3)
		{
			fprintf(stderr, "Beta++: RAW id=%u n=%zu:", finished, w.size());
			for (size_t i = 0; i < w.size() && i < 48; i++)
				fprintf(stderr, " %08x", w[i]);
			fprintf(stderr, "\n");
		}
	}
	g_recordingList = 0;
	invalidateBaked(finished);
}

SHIM_HIDDEN void APIENTRY glCallList(GLuint list)
{
	g_dbg.listCalls++;
	if (recording())
		emitSimple(OP_CALL_LIST, {list});
	else
		replayList(list, 0);
}

SHIM_HIDDEN void APIENTRY glCallLists(GLsizei n, GLenum /*type*/, const void *listsIn)
{
	if (listsIn == nullptr || n <= 0)
		return;
	// The game passes plain int32 display list ids (FontRenderer, RenderList).
	const auto *ids = static_cast<const int32_t *>(listsIn);
	if (recording())
	{
		Words &w = emit(OP_CALL_LISTS, 1 + static_cast<size_t>(n));
		w.push_back(static_cast<uint32_t>(n));
		for (GLsizei i = 0; i < n; ++i)
			w.push_back(static_cast<uint32_t>(ids[i]));
	}
	else
	{
		for (GLsizei i = 0; i < n; ++i)
			replayList(static_cast<GLuint>(ids[i]), 0);
	}
}

// ── State (record-or-forward) ────────────────────────────────────────────────

SHIM_HIDDEN void APIENTRY glEnable(GLenum cap)
{
	if (recording())
		emitSimple(OP_ENABLE, {cap});
	else if (real().Enable)
		real().Enable(cap);
}

SHIM_HIDDEN void APIENTRY glDisable(GLenum cap)
{
	if (recording())
		emitSimple(OP_DISABLE, {cap});
	else if (real().Disable)
		real().Disable(cap);
}

SHIM_HIDDEN void APIENTRY glBindTexture(GLenum target, GLuint texture)
{
	if (recording())
	{
		Words &w = emit(OP_BIND_TEXTURE, 2);
		w.push_back(target);
		w.push_back(texture);
	}
	else if (real().BindTexture)
		real().BindTexture(target, texture);
}

SHIM_HIDDEN void APIENTRY glDepthMask(GLboolean flag)
{
	if (recording())
		emitSimple(OP_DEPTH_MASK, {flag ? 1u : 0u});
	else if (real().DepthMask)
		real().DepthMask(flag);
}

SHIM_HIDDEN void APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor)
{
	if (recording())
	{
		Words &w = emit(OP_BLEND_FUNC, 2);
		w.push_back(sfactor);
		w.push_back(dfactor);
	}
	else if (real().BlendFunc)
		real().BlendFunc(sfactor, dfactor);
}

SHIM_HIDDEN void APIENTRY glAlphaFunc(GLenum func, GLfloat ref)
{
	if (recording())
	{
		Words &w = emit(OP_ALPHA_FUNC, 2);
		w.push_back(func);
		w.push_back(packFloat(ref));
	}
	else if (real().AlphaFunc)
		real().AlphaFunc(func, ref);
}

SHIM_HIDDEN void APIENTRY glShadeModel(GLenum mode)
{
	if (recording())
		emitSimple(OP_SHADE_MODEL, {mode});
	else if (real().ShadeModel)
		real().ShadeModel(mode);
}

SHIM_HIDDEN void APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param)
{
	if (recording())
	{
		Words &w = emit(OP_TEX_PARAMETERI, 3);
		w.push_back(target);
		w.push_back(pname);
		w.push_back(static_cast<uint32_t>(param));
	}
	else if (real().TexParameteri)
		real().TexParameteri(target, pname, param);
}

SHIM_HIDDEN void APIENTRY glFogi(GLenum pname, GLint param)
{
	glFogf(pname, static_cast<GLfloat>(param));
}

SHIM_HIDDEN void APIENTRY glFogf(GLenum pname, GLfloat param)
{
	if (recording())
	{
		Words &w = emit(OP_FOGF, 2);
		w.push_back(pname);
		w.push_back(packFloat(param));
	}
	else if (real().Fogf)
		real().Fogf(pname, param);
}

SHIM_HIDDEN void APIENTRY glFogfv(GLenum pname, const GLfloat *params)
{
	if (params == nullptr)
		return;
	int count = (pname == GL_FOG_COLOR) ? 4 : 1;
	if (recording())
	{
		Words &w = emit(OP_FOGFV, 1 + count);
		w.push_back(pname);
		for (int i = 0; i < count; ++i)
			w.push_back(packFloat(params[i]));
	}
	else if (real().Fogfv)
		real().Fogfv(pname, params);
}

SHIM_HIDDEN void APIENTRY glLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
	if (params == nullptr)
		return;
	if (recording())
	{
		Words &w = emit(OP_LIGHTFV, 6);
		w.push_back(light);
		w.push_back(pname);
		for (int i = 0; i < 4; ++i)
			w.push_back(packFloat(params[i]));
	}
	else if (real().Lightfv)
		real().Lightfv(light, pname, params);
}

SHIM_HIDDEN void APIENTRY glLightModelfv(GLenum pname, const GLfloat *params)
{
	if (params == nullptr)
		return;
	if (recording())
	{
		Words &w = emit(OP_LIGHT_MODELFV, 5);
		w.push_back(pname);
		for (int i = 0; i < 4; ++i)
			w.push_back(packFloat(params[i]));
	}
	else if (real().LightModelfv)
		real().LightModelfv(pname, params);
}

SHIM_HIDDEN void APIENTRY glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
	glColor4f(red, green, blue, 1.0f);
}

SHIM_HIDDEN void APIENTRY glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	if (g_immActive)
	{
		g_curColor[0] = red;
		g_curColor[1] = green;
		g_curColor[2] = blue;
		g_curColor[3] = alpha;
		return;
	}
	if (recording())
	{
		Words &w = emit(OP_COLOR4F, 4);
		w.push_back(packFloat(red));
		w.push_back(packFloat(green));
		w.push_back(packFloat(blue));
		w.push_back(packFloat(alpha));
		return;
	}
	if (real().Color4f)
		real().Color4f(red, green, blue, alpha);
}

// Not in ES 1.1 at all: material colour tracking. Entity lighting relies on
// vertex colours + lightmap textures in this renderer, so a no-op is fine.
SHIM_HIDDEN void APIENTRY glColorMaterial(GLenum /*face*/, GLenum /*mode*/)
{
}

SHIM_HIDDEN void APIENTRY glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
	if (g_immActive)
	{
		g_curNormal[0] = nx;
		g_curNormal[1] = ny;
		g_curNormal[2] = nz;
		g_streamHasNrm = true;
		return;
	}
	if (!recording() && real().Normal3f)
		real().Normal3f(nx, ny, nz);
}

SHIM_HIDDEN void APIENTRY glLineWidth(GLfloat width)
{
	if (recording())
	{
		Words &w = emit(OP_LINE_WIDTH, 1);
		w.push_back(packFloat(width));
	}
	else if (real().LineWidth)
		real().LineWidth(width);
}

SHIM_HIDDEN void APIENTRY glPolygonOffset(GLfloat factor, GLfloat units)
{
	if (recording())
	{
		Words &w = emit(OP_POLYGON_OFFSET, 2);
		w.push_back(packFloat(factor));
		w.push_back(packFloat(units));
	}
	else if (real().PolygonOffset)
		real().PolygonOffset(factor, units);
}

SHIM_HIDDEN void APIENTRY glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	if (recording())
	{
		Words &w = emit(OP_COLOR_MASK, 4);
		w.push_back(red ? 1u : 0u);
		w.push_back(green ? 1u : 0u);
		w.push_back(blue ? 1u : 0u);
		w.push_back(alpha ? 1u : 0u);
	}
	else if (real().ColorMask)
		real().ColorMask(red, green, blue, alpha);
}

SHIM_HIDDEN void APIENTRY glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	if (recording())
	{
		Words &w = emit(OP_CLEAR_COLOR, 4);
		w.push_back(packFloat(red));
		w.push_back(packFloat(green));
		w.push_back(packFloat(blue));
		w.push_back(packFloat(alpha));
	}
	else if (real().ClearColor)
		real().ClearColor(red, green, blue, alpha);
}

SHIM_HIDDEN void APIENTRY glCullFace(GLenum mode)
{
	if (recording())
		emitSimple(OP_CULL_FACE, {mode});
	else if (real().CullFace)
		real().CullFace(mode);
}

// ── Texture upload counters (pass-through; ES1 has these natively) ──────────

SHIM_HIDDEN void APIENTRY glTexImage2D(GLenum target, GLint level, GLint internalformat,
                                       GLsizei width, GLsizei height, GLint border,
                                       GLenum format, GLenum type, const GLvoid *pixels)
{
	g_dbg.texUploadCalls++;
	g_dbg.texUploadBytes += static_cast<unsigned long>(width) * height * 4;
	if (real().TexImage2D)
		real().TexImage2D(target, level, internalformat, width, height, border,
		                  format, type, pixels);
}

SHIM_HIDDEN void APIENTRY glTexSubImage2D(GLenum target, GLint level, GLint xoffset,
                                          GLint yoffset, GLsizei width, GLsizei height,
                                          GLenum format, GLenum type, const GLvoid *pixels)
{
	g_dbg.texUploadCalls++;
	g_dbg.texUploadBytes += static_cast<unsigned long>(width) * height * 4;
	if (real().TexSubImage2D)
		real().TexSubImage2D(target, level, xoffset, yoffset, width, height,
		                     format, type, pixels);
}

SHIM_HIDDEN void APIENTRY glClear(GLbitfield mask)
{
	g_dbg.clears++;
	if (recording())
		emitSimple(OP_CLEAR, {mask});
	else if (real().Clear)
		real().Clear(mask);
}

// ── Matrices ─────────────────────────────────────────────────────────────────

int gles11MatrixMaxDepth(int idx) { return g_mat.maxDepth[idx & 3]; }
int gles11MatrixCurDepth(int idx) { return (int)g_mat.stack[idx & 3].size(); }
int gles11MatrixRefused(int idx) { return g_mat.refused[idx & 3]; }

SHIM_HIDDEN void APIENTRY glMatrixMode(GLenum mode)
{
	if (recording())
		emitSimple(OP_MATRIX_MODE, {mode});
	else
	{
		g_mat.modeIdx = MatrixState::index(mode);
		if (real().MatrixMode)
			real().MatrixMode(mode);
	}
}

SHIM_HIDDEN void APIENTRY glLoadIdentity(void)
{
	if (recording())
		emitSimple(OP_LOAD_IDENTITY, {});
	else
	{
		g_mat.cur[g_mat.modeIdx] = matIdentity();
		syncRealMatrix();
	}
}

SHIM_HIDDEN void APIENTRY glPushMatrix(void)
{
	if (recording())
		emitSimple(OP_PUSH_MATRIX, {});
	else
	{
		int i = g_mat.modeIdx;
		g_mat.stack[i].push_back(g_mat.cur[i]);
		if ((int)g_mat.stack[i].size() > g_mat.maxDepth[i])
			g_mat.maxDepth[i] = (int)g_mat.stack[i].size();
		if ((int)g_mat.stack[i].size() > 64)
			noteDeepPush(__builtin_return_address(0));
	}
}

SHIM_HIDDEN void APIENTRY glPopMatrix(void)
{
	if (recording())
		emitSimple(OP_POP_MATRIX, {});
	else
	{
		int i = g_mat.modeIdx;
		if (!g_mat.stack[i].empty())
		{
			g_mat.cur[i] = g_mat.stack[i].back();
			g_mat.stack[i].pop_back();
			syncRealMatrix();
		}
	}
}

SHIM_HIDDEN void APIENTRY glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
	if (recording())
	{
		Words &w = emit(OP_TRANSLATEF, 3);
		w.push_back(packFloat(x));
		w.push_back(packFloat(y));
		w.push_back(packFloat(z));
	}
	else
	{
		Mat4 t = matIdentity();
		t.m[12] = x;
		t.m[13] = y;
		t.m[14] = z;
		int i = g_mat.modeIdx;
		g_mat.cur[i] = matMultiply(g_mat.cur[i], t);
		syncRealMatrix();
	}
}

SHIM_HIDDEN void APIENTRY glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	if (recording())
	{
		Words &w = emit(OP_ROTATEF, 4);
		w.push_back(packFloat(angle));
		w.push_back(packFloat(x));
		w.push_back(packFloat(y));
		w.push_back(packFloat(z));
	}
	else
	{
		float len = std::sqrt(x * x + y * y + z * z);
		if (len > 0.0f)
		{
			float xn = x / len, yn = y / len, zn = z / len;
			float rad = angle * 3.14159265358979323846f / 180.0f;
			float c = std::cos(rad), s = std::sin(rad), ic = 1.0f - c;
			Mat4 m = matIdentity();
			m.m[0] = xn * xn * ic + c;
			m.m[1] = yn * xn * ic + zn * s;
			m.m[2] = xn * zn * ic - yn * s;
			m.m[4] = xn * yn * ic - zn * s;
			m.m[5] = yn * yn * ic + c;
			m.m[6] = yn * zn * ic + xn * s;
			m.m[8] = xn * zn * ic + yn * s;
			m.m[9] = yn * zn * ic - xn * s;
			m.m[10] = zn * zn * ic + c;
			int i = g_mat.modeIdx;
			g_mat.cur[i] = matMultiply(g_mat.cur[i], m);
		}
		syncRealMatrix();
	}
}

SHIM_HIDDEN void APIENTRY glScalef(GLfloat x, GLfloat y, GLfloat z)
{
	if (recording())
	{
		Words &w = emit(OP_SCALEF, 3);
		w.push_back(packFloat(x));
		w.push_back(packFloat(y));
		w.push_back(packFloat(z));
	}
	else
	{
		Mat4 m = matIdentity();
		m.m[0] = x;
		m.m[5] = y;
		m.m[10] = z;
		int i = g_mat.modeIdx;
		g_mat.cur[i] = matMultiply(g_mat.cur[i], m);
		syncRealMatrix();
	}
}

SHIM_HIDDEN void APIENTRY glScaled(GLdouble x, GLdouble y, GLdouble z)
{
	glScalef(static_cast<GLfloat>(x), static_cast<GLfloat>(y), static_cast<GLfloat>(z));
}

SHIM_HIDDEN void APIENTRY glOrthof(GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f)
{
	if (recording())
	{
		Words &w = emit(OP_ORTHO, 6);
		w.push_back(packFloat(l));
		w.push_back(packFloat(r));
		w.push_back(packFloat(b));
		w.push_back(packFloat(t));
		w.push_back(packFloat(n));
		w.push_back(packFloat(f));
	}
	else
	{
		Mat4 o{{}};
		o.m[0] = 2.0f / (r - l);
		o.m[5] = 2.0f / (t - b);
		o.m[10] = -2.0f / (f - n);
		o.m[12] = -(r + l) / (r - l);
		o.m[13] = -(t + b) / (t - b);
		o.m[14] = -(f + n) / (f - n);
		o.m[15] = 1.0f;
		int i = g_mat.modeIdx;
		g_mat.cur[i] = matMultiply(g_mat.cur[i], o);
		syncRealMatrix();
	}
}

SHIM_HIDDEN void APIENTRY glOrtho(GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f)
{
	glOrthof(static_cast<GLfloat>(l), static_cast<GLfloat>(r),
	         static_cast<GLfloat>(b), static_cast<GLfloat>(t),
	         static_cast<GLfloat>(n), static_cast<GLfloat>(f));
}

SHIM_HIDDEN void APIENTRY glFrustumf(GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f)
{
	if (recording())
	{
		Words &w = emit(OP_FRUSTUM, 6);
		w.push_back(packFloat(l));
		w.push_back(packFloat(r));
		w.push_back(packFloat(b));
		w.push_back(packFloat(t));
		w.push_back(packFloat(n));
		w.push_back(packFloat(f));
	}
	else
	{
		Mat4 fr{{}};
		fr.m[0] = 2.0f * n / (r - l);
		fr.m[5] = 2.0f * n / (t - b);
		fr.m[8] = (r + l) / (r - l);
		fr.m[9] = (t + b) / (t - b);
		fr.m[10] = -(f + n) / (f - n);
		fr.m[11] = -1.0f;
		fr.m[14] = -2.0f * f * n / (f - n);
		int i = g_mat.modeIdx;
		g_mat.cur[i] = matMultiply(g_mat.cur[i], fr);
		syncRealMatrix();
	}
}

SHIM_HIDDEN void APIENTRY glFrustum(GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f)
{
	glFrustumf(static_cast<GLfloat>(l), static_cast<GLfloat>(r),
	           static_cast<GLfloat>(b), static_cast<GLfloat>(t),
	           static_cast<GLfloat>(n), static_cast<GLfloat>(f));
}

// ES only exposes the float variants of clear-depth and depth-range.
namespace
{
void (*realClearDepthf)(GLclampf) = resolve<void (*)(GLclampf)>("glClearDepthf");
void (*realDepthRangef)(GLclampf, GLclampf) = resolve<void (*)(GLclampf, GLclampf)>("glDepthRangef");
} // namespace

SHIM_HIDDEN void APIENTRY glClearDepth(GLdouble depth)
{
	if (realClearDepthf)
		realClearDepthf(static_cast<GLclampf>(depth));
}

SHIM_HIDDEN void APIENTRY glDepthRange(GLdouble nearVal, GLdouble farVal)
{
	if (realDepthRangef)
		realDepthRangef(static_cast<GLclampf>(nearVal), static_cast<GLclampf>(farVal));
}

// ── Client arrays & draws ────────────────────────────────────────────────────

SHIM_HIDDEN void APIENTRY glEnableClientState(GLenum array)
{
	int idx = arrIndex(array);
	if (idx >= 0)
		g_arrays[idx].enabled = true;
	if (!recording() && real().EnableClientState)
		real().EnableClientState(array);
}

SHIM_HIDDEN void APIENTRY glDisableClientState(GLenum array)
{
	int idx = arrIndex(array);
	if (idx >= 0)
		g_arrays[idx].enabled = false;
	if (!recording() && real().DisableClientState)
		real().DisableClientState(array);
}

SHIM_HIDDEN void APIENTRY glVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
	ArraySpec &a = g_arrays[ARR_VERTEX];
	a.size = size;
	a.type = type;
	a.stride = stride;
	a.pointer = pointer;
	if (!recording() && real().VertexPointer)
		real().VertexPointer(size, type, stride, pointer);
}

SHIM_HIDDEN void APIENTRY glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
	ArraySpec &a = g_arrays[ARR_TEXCOORD];
	a.size = size;
	a.type = type;
	a.stride = stride;
	a.pointer = pointer;
	if (!recording() && real().TexCoordPointer)
		real().TexCoordPointer(size, type, stride, pointer);
}

SHIM_HIDDEN void APIENTRY glColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
	ArraySpec &a = g_arrays[ARR_COLOR];
	a.size = size;
	a.type = type;
	a.stride = stride;
	a.pointer = pointer;
	if (!recording() && real().ColorPointer)
		real().ColorPointer(size, type, stride, pointer);
}

SHIM_HIDDEN void APIENTRY glNormalPointer(GLenum type, GLsizei stride, const void *pointer)
{
	ArraySpec &a = g_arrays[ARR_NORMAL];
	a.size = 3;
	a.type = type;
	a.stride = stride;
	a.pointer = pointer;
	if (!recording() && real().NormalPointer)
		real().NormalPointer(type, stride, pointer);
}

SHIM_HIDDEN void APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
	if (recording())
	{
		emitDrawArraysSnapshot(mode, first, count);
		return;
	}
	g_dbg.drawArrays++;
	g_dbg.arrVerts += static_cast<unsigned long>(count);
	if (real().ok)
		real().DrawArrays(mode, first, count);
}

// ── Immediate mode ───────────────────────────────────────────────────────────

SHIM_HIDDEN void APIENTRY glBegin(GLenum mode)
{
	if (g_immActive)
		return;
	g_immMode = mode;
	g_immActive = true;
	g_immVerts.clear();
	g_streamHasTex = false;
	g_streamHasNrm = false;
}

SHIM_HIDDEN void APIENTRY glEnd(void)
{
	if (!g_immActive)
		return;
	g_immActive = false;
	if (recording())
		emitBeginBlock(g_immMode, g_immVerts);
	else
		drawVertices(g_immMode, g_immVerts.data(), static_cast<GLsizei>(g_immVerts.size()));
	g_immVerts.clear();
}

SHIM_HIDDEN void APIENTRY glVertex2i(GLint x, GLint y)
{
	glVertex3f(static_cast<GLfloat>(x), static_cast<GLfloat>(y), 0.0f);
}

SHIM_HIDDEN void APIENTRY glVertex2f(GLfloat x, GLfloat y)
{
	glVertex3f(x, y, 0.0f);
}

SHIM_HIDDEN void APIENTRY glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
	if (!g_immActive)
		return;
	ImmVertex v{};
	v.x = x;
	v.y = y;
	v.z = z;
	v.s = g_curTexCoord[0];
	v.t = g_curTexCoord[1];
	v.color = colorToU32(g_curColor);
	v.nx = g_curNormal[0];
	v.ny = g_curNormal[1];
	v.nz = g_curNormal[2];
	v.hasTex = g_streamHasTex;
	v.hasNrm = g_streamHasNrm;
	g_immVerts.push_back(v);
}

SHIM_HIDDEN void APIENTRY glTexCoord2f(GLfloat s, GLfloat t)
{
	if (g_immActive)
	{
		g_curTexCoord[0] = s;
		g_curTexCoord[1] = t;
		g_streamHasTex = true;
	}
	// Outside Begin there is no current-texture-coord state worth keeping.
}

// ── glGetFloatv: answer matrix queries from the mirror ───────────────────────

SHIM_HIDDEN void APIENTRY glGetFloatv(GLenum pname, GLfloat *params)
{
	if (params == nullptr)
		return;
	if (isMatrixPname(pname))
	{
		int idx = (pname == GL_PROJECTION_MATRIX) ? 1 :
		          (pname == GL_TEXTURE_MATRIX) ? 2 :
		          (pname == GL_COLOR_MATRIX) ? 3 : 0;
		std::memcpy(params, g_mat.cur[idx].m, sizeof(float) * 16);
		// DIAG: the frustum is built from this mirror, so compare it to what
		// the real GL actually holds. Any mismatch culls the wrong chunks.
		if (real().GetFloatv && idx != 3)
		{
			static std::atomic<int> cmp{0};
			int c = cmp.fetch_add(1);
			if (c < 6)
			{
				GLfloat rv[16] = {0};
				real().GetFloatv(pname, rv);
				int diffs = 0;
				for (int i = 0; i < 16; i++)
					if (std::fabs(rv[i] - g_mat.cur[idx].m[i]) > 1e-3f) diffs++;
				fprintf(stderr, "Beta++: GMATCH pname=%x idx=%d diffs=%d\n", (unsigned)pname, idx, diffs);
				if (diffs)
				{
					fprintf(stderr, "  mirror:");
					for (int i = 0; i < 16; i++) fprintf(stderr, " %.3f", g_mat.cur[idx].m[i]);
					fprintf(stderr, "\n  real  :");
					for (int i = 0; i < 16; i++) fprintf(stderr, " %.3f", rv[i]);
					fprintf(stderr, "\n");
				}
			}
		}
		return;
	}
	if (real().GetFloatv)
		real().GetFloatv(pname, params);
}

// ── ARB buffer objects → ES 1.1 core ─────────────────────────────────────────

SHIM_HIDDEN void APIENTRY glGenBuffersARB(GLsizei n, GLuint *buffers)
{
	if (real().GenBuffers)
		real().GenBuffers(n, buffers);
	else
		for (GLsizei i = 0; i < n; ++i)
			buffers[i] = 0;
}

SHIM_HIDDEN void APIENTRY glDeleteBuffersARB(GLsizei n, const GLuint *buffers)
{
	if (real().DeleteBuffers)
		real().DeleteBuffers(n, buffers);
}

SHIM_HIDDEN void APIENTRY glBindBufferARB(GLenum target, GLuint buffer)
{
	if (real().BindBuffer)
		real().BindBuffer(target, buffer);
}

SHIM_HIDDEN void APIENTRY glBufferDataARB(GLenum target, GLsizeiptr size, const void *data, GLenum usage)
{
	if (real().BufferData)
		real().BufferData(target, size, data, usage);
}

// ── ARB occlusion queries → fail open (everything visible) ──────────────────

SHIM_HIDDEN void APIENTRY glGenQueriesARB(GLsizei n, GLuint *ids)
{
	for (GLsizei i = 0; i < n; ++i)
		ids[i] = 1u + static_cast<GLuint>(i); // non-zero dummy ids
}

SHIM_HIDDEN void APIENTRY glDeleteQueriesARB(GLsizei /*n*/, const GLuint * /*ids*/)
{
}

SHIM_HIDDEN void APIENTRY glBeginQueryARB(GLenum /*target*/, GLuint /*id*/)
{
}

SHIM_HIDDEN void APIENTRY glEndQueryARB(GLenum /*target*/)
{
}

SHIM_HIDDEN void APIENTRY glGetQueryObjectuivARB(GLuint /*id*/, GLenum pname, GLuint *params)
{
	if (pname == GL_QUERY_RESULT_AVAILABLE_ARB)
		*params = 1u; // result ready…
	else if (pname == GL_QUERY_RESULT_ARB)
		*params = 1u; // …and one sample passed: never cull
}

} // extern "C"

namespace gles11shim
{

bool initialize()
{
	if (!real().ok)
		return false;
	// Never reuse display-list ids: if this runs again mid-session (e.g. the GL
	// context was recreated under memory pressure) the game still holds its old
	// ids (font, chunk render lists). Restarting at 1 would hand a NEW list the
	// same id as an old one, so a chunk could draw another list's geometry
	// (shattered triangles / wrong texture). Just invalidate every list.
	fprintf(stderr, "Beta++: SHIM initialize (reset, nextListId was %u)\n", g_nextListId);
	lists().clear();
	bakedLists().clear();
	g_nextListId += 1024; // skip ahead so no live id is ever reissued
	g_recordingList = 0;
	resetMatrices();
	resetClientState();
	resetImmediate();
	return true;
}

bool insideRecordOrBegin()
{
	return recording() || g_immActive;
}

DebugStats debugStats()
{
	DebugStats s;
	s.clears         = g_dbg.clears;
	s.immFlushes     = g_dbg.immFlushes;
	s.immVerts       = g_dbg.immVerts;
	s.arrVerts       = g_dbg.arrVerts;
	s.listCalls      = g_dbg.listCalls;
	s.drawArrays     = g_dbg.drawArrays;
	s.drawElements   = g_dbg.drawElements;
	s.listCompiles   = g_dbg.listCompiles;
	s.texUploadBytes = g_dbg.texUploadBytes;
	s.texUploadCalls = g_dbg.texUploadCalls;
	s.lastGlError    = g_dbg.lastGlError;
	return s;
}

} // namespace gles11shim
