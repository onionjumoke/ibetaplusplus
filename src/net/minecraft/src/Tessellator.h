#pragma once

#include "java/Type.h"

// net.minecraft.src.Tessellator
class Tessellator
{
public:
	static bool convertQuadsToTriangles;
	static bool tryVBO;

	static Tessellator instance;

	Tessellator(int bufferSize);
	~Tessellator();

	void draw();
	void startDrawingQuads();
	void startDrawing(int mode);
	void setTextureUV(double u, double v);
	void setColorOpaque_F(float r, float g, float b);
	void setColorRGBA_F(float r, float g, float b, float a);
	void setColorOpaque(int r, int g, int b);
	void setColorRGBA(int r, int g, int b, int a);
	void addVertexWithUV(double x, double y, double z, double u, double v);
	void addVertex(double x, double y, double z);
	void setColorOpaque_I(int color);
	void setColorRGBA_I(int color, int alpha);
	void disableColor();
	void setNormal(float x, float y, float z);
	void setTranslationD(double x, double y, double z);
	void setTranslationF(float x, float y, float z);

private:
	void reset();

	int_t* rawBuffer;
	int_t bufferSize;

	int_t vertexCount;
	double textureU;
	double textureV;
	int_t color;
	bool hasColor;
	bool hasTexture;
	bool hasNormals;
	int_t rawBufferIndex;
	int_t addedVertices;
	bool isColorDisabled;
	int_t drawMode;
	double xOffset;
	double yOffset;
	double zOffset;
	int_t normal;
	bool isDrawing;
	bool useVBO;
	uint32_t* vertexBuffers;
	int_t vboIndex;
	int_t vboCount;
};
