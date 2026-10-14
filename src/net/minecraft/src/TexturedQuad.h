#pragma once

#include "java/Type.h"

class PositionTextureVertex;
class Tessellator;
class Vec3D;

// net.minecraft.src.TexturedQuad
class TexturedQuad
{
public:
	TexturedQuad(PositionTextureVertex *apositiontexturevertex[]);
	TexturedQuad(PositionTextureVertex *apositiontexturevertex[], int i, int j, int k, int l);

	void flipFace();
	void draw(Tessellator *tessellator, float f);

	PositionTextureVertex **vertexPositions;
	int_t nVertices;
	bool invertNormal;
};
