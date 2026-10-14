#pragma once

#include "ModelBase.h"
#include "java/Type.h"

class ModelRenderer;

// net.minecraft.src.ModelQuadruped
class ModelQuadruped : public ModelBase
{
public:
	ModelQuadruped(int_t i, float f);

	void render(float f, float f1, float f2, float f3, float f4, float f5) override;
	void setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5) override;

	ModelRenderer *head;
	ModelRenderer *body;
	ModelRenderer *leg1;
	ModelRenderer *leg2;
	ModelRenderer *leg3;
	ModelRenderer *leg4;
};
