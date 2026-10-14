#pragma once

#include "java/Type.h"

class PositionTextureVertex;
class TexturedQuad;

// net.minecraft.src.ModelRenderer
class ModelRenderer
{
public:
	ModelRenderer(int_t i, int_t j);

	void addBox(float f, float f1, float f2, int_t i, int_t j, int_t k);
	void addBox(float f, float f1, float f2, int_t i, int_t j, int_t k, float f3);
	void setRotationPoint(float f, float f1, float f2);
	void render(float f);
	void renderWithRotation(float f);
	void postRender(float f);

private:
	void compileDisplayList(float f);

	PositionTextureVertex **corners;
	TexturedQuad **faces;
	int_t textureOffsetX;
	int_t textureOffsetY;

public:
	float rotationPointX;
	float rotationPointY;
	float rotationPointZ;
	float rotateAngleX;
	float rotateAngleY;
	float rotateAngleZ;

private:
	bool compiled;
	int_t displayList;

public:
	bool mirror;
	bool showModel;
	bool field_1402_i;
};
