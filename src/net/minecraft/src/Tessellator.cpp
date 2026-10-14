#include "Tessellator.h"

#include <cstdio>
#include <cstring>
#include <stdexcept>

#include "pc/OpenGL.h"
#include "pc/lwjgl/GLContext.h"

static int32_t floatToRawIntBits(float f)
{
	int32_t result;
	std::memcpy(&result, &f, sizeof(result));
	return result;
}

static bool isLittleEndian()
{
	union
	{
		uint32_t i;
		uint8_t b[4];
	};
	i = 1;
	return b[0] == 1;
}

bool Tessellator::convertQuadsToTriangles = true;
bool Tessellator::tryVBO = false;
Tessellator Tessellator::instance(0x200000);

Tessellator::Tessellator(int bufferSize) :
	rawBuffer(new int_t[bufferSize]),
	bufferSize(bufferSize),
	vertexCount(0),
	textureU(0.0),
	textureV(0.0),
	color(0),
	hasColor(false),
	hasTexture(false),
	hasNormals(false),
	rawBufferIndex(0),
	addedVertices(0),
	isColorDisabled(false),
	drawMode(0),
	xOffset(0.0),
	yOffset(0.0),
	zOffset(0.0),
	normal(0),
	isDrawing(false),
	useVBO(false),
	vertexBuffers(nullptr),
	vboIndex(0),
	vboCount(10)
{
	useVBO = tryVBO && lwjgl::GLContext::getCapabilities()["GL_ARB_vertex_buffer_object"];
#ifdef MC_IOS
	// The iOS shim replays display lists as recorded client-array snapshots; the
	// VBO ring + per-list glBufferDataARB replay is not faithfully reproduced and
	// produced garbage vertices (shattered chunk meshes). Use client arrays.
	useVBO = false;
#endif
	if (useVBO)
	{
		vertexBuffers = new uint32_t[vboCount];
		glGenBuffersARB(vboCount, reinterpret_cast<GLuint*>(vertexBuffers));
	}
}

Tessellator::~Tessellator()
{
	delete[] rawBuffer;
	delete[] vertexBuffers;
}

void Tessellator::draw()
{
	if (!isDrawing)
	{
		throw std::runtime_error("Not tesselating!");
	}
	isDrawing = false;
	if (vertexCount > 0)
	{
		if (useVBO)
		{
			vboIndex = (vboIndex + 1) % vboCount;
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, reinterpret_cast<GLuint*>(vertexBuffers)[vboIndex]);
			glBufferDataARB(GL_ARRAY_BUFFER_ARB, rawBufferIndex * 4, rawBuffer, GL_STREAM_DRAW_ARB);
		}
		if (hasTexture)
		{
			if (useVBO)
			{
				glTexCoordPointer(2, GL_FLOAT, 32, reinterpret_cast<void*>(12));
			}
			else
			{
				glTexCoordPointer(2, GL_FLOAT, 32, reinterpret_cast<float*>(rawBuffer) + 3);
			}
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		if (hasColor)
		{
			if (useVBO)
			{
				glColorPointer(4, GL_UNSIGNED_BYTE, 32, reinterpret_cast<void*>(20));
			}
			else
			{
				glColorPointer(4, GL_UNSIGNED_BYTE, 32, reinterpret_cast<uint8_t*>(rawBuffer) + 20);
			}
			glEnableClientState(GL_COLOR_ARRAY);
		}
		if (hasNormals)
		{
			if (useVBO)
			{
				glNormalPointer(GL_BYTE, 32, reinterpret_cast<void*>(24));
			}
			else
			{
				glNormalPointer(GL_BYTE, 32, reinterpret_cast<uint8_t*>(rawBuffer) + 24);
			}
			glEnableClientState(GL_NORMAL_ARRAY);
		}
		if (useVBO)
		{
			glVertexPointer(3, GL_FLOAT, 32, reinterpret_cast<void*>(0));
		}
		else
		{
			glVertexPointer(3, GL_FLOAT, 32, reinterpret_cast<float*>(rawBuffer));
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		if (drawMode == 7 && convertQuadsToTriangles)
		{
			glDrawArrays(4, 0, vertexCount);
		}
		else
		{
			glDrawArrays(drawMode, 0, vertexCount);
		}
		glDisableClientState(GL_VERTEX_ARRAY);
		if (hasTexture)
		{
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		if (hasColor)
		{
			glDisableClientState(GL_COLOR_ARRAY);
		}
		if (hasNormals)
		{
			glDisableClientState(GL_NORMAL_ARRAY);
		}
	}
	reset();
}

void Tessellator::reset()
{
	vertexCount = 0;
	rawBufferIndex = 0;
	addedVertices = 0;
}

void Tessellator::startDrawingQuads()
{
	startDrawing(7);
}

void Tessellator::startDrawing(int i)
{
	if (isDrawing)
	{
		throw std::runtime_error("Already tesselating!");
	}
	else
	{
		isDrawing = true;
		reset();
		drawMode = i;
		hasNormals = false;
		hasColor = false;
		hasTexture = false;
		isColorDisabled = false;
	}
}

void Tessellator::setTextureUV(double d, double d1)
{
	hasTexture = true;
	textureU = d;
	textureV = d1;
}

void Tessellator::setColorOpaque_F(float f, float f1, float f2)
{
	setColorOpaque((int)(f * 255.0f), (int)(f1 * 255.0f), (int)(f2 * 255.0f));
}

void Tessellator::setColorRGBA_F(float f, float f1, float f2, float f3)
{
	setColorRGBA((int)(f * 255.0f), (int)(f1 * 255.0f), (int)(f2 * 255.0f), (int)(f3 * 255.0f));
}

void Tessellator::setColorOpaque(int i, int j, int k)
{
	setColorRGBA(i, j, k, 255);
}

void Tessellator::setColorRGBA(int i, int j, int k, int l)
{
	if (isColorDisabled)
	{
		return;
	}
	if (i > 255)
	{
		i = 255;
	}
	if (j > 255)
	{
		j = 255;
	}
	if (k > 255)
	{
		k = 255;
	}
	if (l > 255)
	{
		l = 255;
	}
	if (i < 0)
	{
		i = 0;
	}
	if (j < 0)
	{
		j = 0;
	}
	if (k < 0)
	{
		k = 0;
	}
	if (l < 0)
	{
		l = 0;
	}
	hasColor = true;
	if (isLittleEndian())
	{
		color = l << 24 | k << 16 | j << 8 | i;
	}
	else
	{
		color = i << 24 | j << 16 | k << 8 | l;
	}
}

void Tessellator::addVertexWithUV(double d, double d1, double d2, double d3, double d4)
{
	setTextureUV(d3, d4);
	addVertex(d, d1, d2);
}

void Tessellator::addVertex(double d, double d1, double d2)
{
	addedVertices++;
	if (drawMode == 7 && convertQuadsToTriangles && addedVertices % 4 == 0)
	{
		for (int iter = 0; iter < 2; iter++)
		{
			int j = 8 * (3 - iter);
			if (hasTexture)
			{
				rawBuffer[rawBufferIndex + 3] = rawBuffer[(rawBufferIndex - j) + 3];
				rawBuffer[rawBufferIndex + 4] = rawBuffer[(rawBufferIndex - j) + 4];
			}
			if (hasColor)
			{
				rawBuffer[rawBufferIndex + 5] = rawBuffer[(rawBufferIndex - j) + 5];
			}
			rawBuffer[rawBufferIndex + 0] = rawBuffer[(rawBufferIndex - j) + 0];
			rawBuffer[rawBufferIndex + 1] = rawBuffer[(rawBufferIndex - j) + 1];
			rawBuffer[rawBufferIndex + 2] = rawBuffer[(rawBufferIndex - j) + 2];
			vertexCount++;
			rawBufferIndex += 8;
		}
	}
	if (hasTexture)
	{
		rawBuffer[rawBufferIndex + 3] = floatToRawIntBits((float)textureU);
		rawBuffer[rawBufferIndex + 4] = floatToRawIntBits((float)textureV);
	}
	if (hasColor)
	{
		rawBuffer[rawBufferIndex + 5] = color;
	}
	if (hasNormals)
	{
		rawBuffer[rawBufferIndex + 6] = normal;
	}
	rawBuffer[rawBufferIndex + 0] = floatToRawIntBits((float)(d + xOffset));
	rawBuffer[rawBufferIndex + 1] = floatToRawIntBits((float)(d1 + yOffset));
	rawBuffer[rawBufferIndex + 2] = floatToRawIntBits((float)(d2 + zOffset));
	rawBufferIndex += 8;
	vertexCount++;
	if (vertexCount % 4 == 0 && rawBufferIndex >= bufferSize - 32)
	{
		draw();
		isDrawing = true;
	}
}

void Tessellator::setColorOpaque_I(int i)
{
	int j = i >> 16 & 0xFF;
	int k = i >> 8 & 0xFF;
	int l = i & 0xFF;
	setColorOpaque(j, k, l);
}

void Tessellator::setColorRGBA_I(int i, int j)
{
	int k = i >> 16 & 0xFF;
	int l = i >> 8 & 0xFF;
	int i1 = i & 0xFF;
	setColorRGBA(k, l, i1, j);
}

void Tessellator::disableColor()
{
	isColorDisabled = true;
}

void Tessellator::setNormal(float f, float f1, float f2)
{
	if (!isDrawing)
	{
		printf("But..\n");
	}
	hasNormals = true;
	byte_t byte0 = (byte_t)(int_t)(f * 128.0f);
	byte_t byte1 = (byte_t)(int_t)(f1 * 127.0f);
	byte_t byte2 = (byte_t)(int_t)(f2 * 127.0f);
	normal = byte0 | byte1 << 8 | byte2 << 16;
}

void Tessellator::setTranslationD(double d, double d1, double d2)
{
	xOffset = d;
	yOffset = d1;
	zOffset = d2;
}

void Tessellator::setTranslationF(float f, float f1, float f2)
{
	xOffset += f;
	yOffset += f1;
	zOffset += f2;
}
