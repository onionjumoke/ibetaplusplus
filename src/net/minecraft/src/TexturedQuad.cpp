#include "TexturedQuad.h"

#include "PositionTextureVertex.h"
#include "Vec3D.h"
#include "Tessellator.h"

TexturedQuad::TexturedQuad(PositionTextureVertex *apositiontexturevertex[]) :
	nVertices(0),
	invertNormal(false)
{
	vertexPositions = apositiontexturevertex;
	nVertices = 4; // array length
}

TexturedQuad::TexturedQuad(PositionTextureVertex *apositiontexturevertex[], int i, int j, int k, int l) :
	TexturedQuad(apositiontexturevertex)
{
	float f = 0.0015625f;
	float f1 = 0.003125f;
	vertexPositions[0] = vertexPositions[0]->setTexturePosition((float)k / 64.0f - f, (float)j / 32.0f + f1);
	vertexPositions[1] = vertexPositions[1]->setTexturePosition((float)i / 64.0f + f, (float)j / 32.0f + f1);
	vertexPositions[2] = vertexPositions[2]->setTexturePosition((float)i / 64.0f + f, (float)l / 32.0f - f1);
	vertexPositions[3] = vertexPositions[3]->setTexturePosition((float)k / 64.0f - f, (float)l / 32.0f - f1);
}

void TexturedQuad::flipFace()
{
	PositionTextureVertex **apositiontexturevertex = new PositionTextureVertex*[nVertices];
	for (int_t i = 0; i < nVertices; i++)
	{
		apositiontexturevertex[i] = vertexPositions[nVertices - i - 1];
	}
	vertexPositions = apositiontexturevertex;
}

void TexturedQuad::draw(Tessellator *tessellator, float f)
{
	Vec3D *vec3d = vertexPositions[1]->vector3D->subtract(vertexPositions[0]->vector3D);
	Vec3D *vec3d1 = vertexPositions[1]->vector3D->subtract(vertexPositions[2]->vector3D);
	Vec3D *vec3d2 = vec3d1->crossProduct(vec3d)->normalize();

	tessellator->startDrawingQuads();
	if (invertNormal)
	{
		tessellator->setNormal(-(float)vec3d2->xCoord, -(float)vec3d2->yCoord, -(float)vec3d2->zCoord);
	}
	else
	{
		tessellator->setNormal((float)vec3d2->xCoord, (float)vec3d2->yCoord, (float)vec3d2->zCoord);
	}
	for (int_t i = 0; i < 4; i++)
	{
		PositionTextureVertex *positiontexturevertex = vertexPositions[i];
		tessellator->addVertexWithUV(
			(float)positiontexturevertex->vector3D->xCoord * f,
			(float)positiontexturevertex->vector3D->yCoord * f,
			(float)positiontexturevertex->vector3D->zCoord * f,
			positiontexturevertex->texturePositionX,
			positiontexturevertex->texturePositionY);
	}
	tessellator->draw();
}
