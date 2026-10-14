#include "ModelRenderer.h"

#include "GLAllocation.h"
#include "PositionTextureVertex.h"
#include "Tessellator.h"
#include "TexturedQuad.h"
#include "pc/OpenGL.h"

ModelRenderer::ModelRenderer(int_t i, int_t j)
{
	compiled = false;
	displayList = 0;
	mirror = false;
	showModel = true;
	field_1402_i = false;
	textureOffsetX = i;
	textureOffsetY = j;
	corners = nullptr;
	faces = nullptr;
	rotationPointX = 0.0f;
	rotationPointY = 0.0f;
	rotationPointZ = 0.0f;
	rotateAngleX = 0.0f;
	rotateAngleY = 0.0f;
	rotateAngleZ = 0.0f;
}

void ModelRenderer::addBox(float f, float f1, float f2, int_t i, int_t j, int_t k)
{
	addBox(f, f1, f2, i, j, k, 0.0f);
}

void ModelRenderer::addBox(float f, float f1, float f2, int_t i, int_t j, int_t k, float f3)
{
	corners = new PositionTextureVertex *[8];
	faces = new TexturedQuad *[6];
	float f4 = f + (float)i;
	float f5 = f1 + (float)j;
	float f6 = f2 + (float)k;
	f -= f3;
	f1 -= f3;
	f2 -= f3;
	f4 += f3;
	f5 += f3;
	f6 += f3;
	if (mirror)
	{
		float f7 = f4;
		f4 = f;
		f = f7;
	}
	PositionTextureVertex *positiontexturevertex = new PositionTextureVertex(f, f1, f2, 0.0f, 0.0f);
	PositionTextureVertex *positiontexturevertex1 = new PositionTextureVertex(f4, f1, f2, 0.0f, 8.0f);
	PositionTextureVertex *positiontexturevertex2 = new PositionTextureVertex(f4, f5, f2, 8.0f, 8.0f);
	PositionTextureVertex *positiontexturevertex3 = new PositionTextureVertex(f, f5, f2, 8.0f, 0.0f);
	PositionTextureVertex *positiontexturevertex4 = new PositionTextureVertex(f, f1, f6, 0.0f, 0.0f);
	PositionTextureVertex *positiontexturevertex5 = new PositionTextureVertex(f4, f1, f6, 0.0f, 8.0f);
	PositionTextureVertex *positiontexturevertex6 = new PositionTextureVertex(f4, f5, f6, 8.0f, 8.0f);
	PositionTextureVertex *positiontexturevertex7 = new PositionTextureVertex(f, f5, f6, 8.0f, 0.0f);
	corners[0] = positiontexturevertex;
	corners[1] = positiontexturevertex1;
	corners[2] = positiontexturevertex2;
	corners[3] = positiontexturevertex3;
	corners[4] = positiontexturevertex4;
	corners[5] = positiontexturevertex5;
	corners[6] = positiontexturevertex6;
	corners[7] = positiontexturevertex7;

	PositionTextureVertex **face0 = new PositionTextureVertex *[4]{ positiontexturevertex5, positiontexturevertex1, positiontexturevertex2, positiontexturevertex6 };
	PositionTextureVertex **face1 = new PositionTextureVertex *[4]{ positiontexturevertex, positiontexturevertex4, positiontexturevertex7, positiontexturevertex3 };
	PositionTextureVertex **face2 = new PositionTextureVertex *[4]{ positiontexturevertex5, positiontexturevertex4, positiontexturevertex, positiontexturevertex1 };
	PositionTextureVertex **face3 = new PositionTextureVertex *[4]{ positiontexturevertex2, positiontexturevertex3, positiontexturevertex7, positiontexturevertex6 };
	PositionTextureVertex **face4 = new PositionTextureVertex *[4]{ positiontexturevertex1, positiontexturevertex, positiontexturevertex3, positiontexturevertex2 };
	PositionTextureVertex **face5 = new PositionTextureVertex *[4]{ positiontexturevertex4, positiontexturevertex5, positiontexturevertex6, positiontexturevertex7 };

	faces[0] = new TexturedQuad(face0, textureOffsetX + k + i, textureOffsetY + k, textureOffsetX + k + i + k, textureOffsetY + k + j);
	faces[1] = new TexturedQuad(face1, textureOffsetX + 0, textureOffsetY + k, textureOffsetX + k, textureOffsetY + k + j);
	faces[2] = new TexturedQuad(face2, textureOffsetX + k, textureOffsetY + 0, textureOffsetX + k + i, textureOffsetY + k);
	faces[3] = new TexturedQuad(face3, textureOffsetX + k + i, textureOffsetY + 0, textureOffsetX + k + i + i, textureOffsetY + k);
	faces[4] = new TexturedQuad(face4, textureOffsetX + k, textureOffsetY + k, textureOffsetX + k + i, textureOffsetY + k + j);
	faces[5] = new TexturedQuad(face5, textureOffsetX + k + i + k, textureOffsetY + k, textureOffsetX + k + i + k + i, textureOffsetY + k + j);

	if (mirror)
	{
		for (int_t l = 0; l < 6; l++)
			faces[l]->flipFace();
	}
}

void ModelRenderer::setRotationPoint(float f, float f1, float f2)
{
	rotationPointX = f;
	rotationPointY = f1;
	rotationPointZ = f2;
}

void ModelRenderer::render(float f)
{
	if (field_1402_i)
		return;
	if (!showModel)
		return;
	if (!compiled)
		compileDisplayList(f);
	if (rotateAngleX != 0.0f || rotateAngleY != 0.0f || rotateAngleZ != 0.0f)
	{
		glPushMatrix();
		glTranslatef(rotationPointX * f, rotationPointY * f, rotationPointZ * f);
		if (rotateAngleZ != 0.0f)
			glRotatef(rotateAngleZ * 57.29578f, 0.0f, 0.0f, 1.0f);
		if (rotateAngleY != 0.0f)
			glRotatef(rotateAngleY * 57.29578f, 0.0f, 1.0f, 0.0f);
		if (rotateAngleX != 0.0f)
			glRotatef(rotateAngleX * 57.29578f, 1.0f, 0.0f, 0.0f);
		glCallList(displayList);
		glPopMatrix();
	}
	else if (rotationPointX != 0.0f || rotationPointY != 0.0f || rotationPointZ != 0.0f)
	{
		glTranslatef(rotationPointX * f, rotationPointY * f, rotationPointZ * f);
		glCallList(displayList);
		glTranslatef(-rotationPointX * f, -rotationPointY * f, -rotationPointZ * f);
	}
	else
	{
		glCallList(displayList);
	}
}

void ModelRenderer::renderWithRotation(float f)
{
	if (field_1402_i)
		return;
	if (!showModel)
		return;
	if (!compiled)
		compileDisplayList(f);
	glPushMatrix();
	glTranslatef(rotationPointX * f, rotationPointY * f, rotationPointZ * f);
	if (rotateAngleY != 0.0f)
		glRotatef(rotateAngleY * 57.29578f, 0.0f, 1.0f, 0.0f);
	if (rotateAngleX != 0.0f)
		glRotatef(rotateAngleX * 57.29578f, 1.0f, 0.0f, 0.0f);
	if (rotateAngleZ != 0.0f)
		glRotatef(rotateAngleZ * 57.29578f, 0.0f, 0.0f, 1.0f);
	glCallList(displayList);
	glPopMatrix();
}

void ModelRenderer::postRender(float f)
{
	if (field_1402_i)
		return;
	if (!showModel)
		return;
	if (!compiled)
		compileDisplayList(f);
	if (rotateAngleX != 0.0f || rotateAngleY != 0.0f || rotateAngleZ != 0.0f)
	{
		glTranslatef(rotationPointX * f, rotationPointY * f, rotationPointZ * f);
		if (rotateAngleZ != 0.0f)
			glRotatef(rotateAngleZ * 57.29578f, 0.0f, 0.0f, 1.0f);
		if (rotateAngleY != 0.0f)
			glRotatef(rotateAngleY * 57.29578f, 0.0f, 1.0f, 0.0f);
		if (rotateAngleX != 0.0f)
			glRotatef(rotateAngleX * 57.29578f, 1.0f, 0.0f, 0.0f);
	}
	else if (rotationPointX != 0.0f || rotationPointY != 0.0f || rotationPointZ != 0.0f)
	{
		glTranslatef(rotationPointX * f, rotationPointY * f, rotationPointZ * f);
	}
}

void ModelRenderer::compileDisplayList(float f)
{
	displayList = GLAllocation::generateDisplayLists(1);
	glNewList(displayList, 4864);
	Tessellator *tessellator = &Tessellator::instance;
	for (int_t i = 0; i < 6; i++)
		faces[i]->draw(tessellator, f);
	glEndList();
	compiled = true;
}
