#include "ModelSquid.h"

#include "ModelRenderer.h"

#include <cmath>

ModelSquid::ModelSquid()
{
	int_t byte0 = -16;
	squidBody = new ModelRenderer(0, 0);
	squidBody->addBox(-6.0f, -8.0f, -6.0f, 12, 16, 12);
	squidBody->rotationPointY += 24 + byte0;
	for (int_t i = 0; i < 8; i++)
	{
		squidTentacles[i] = new ModelRenderer(48, 0);
		double d = ((double)i * 3.141592653589793 * 2.0) / (double)8;
		float f = (float)std::cos(d) * 5.0f;
		float f1 = (float)std::sin(d) * 5.0f;
		squidTentacles[i]->addBox(-1.0f, 0.0f, -1.0f, 2, 18, 2);
		squidTentacles[i]->rotationPointX = f;
		squidTentacles[i]->rotationPointZ = f1;
		squidTentacles[i]->rotationPointY = 31 + byte0;
		d = ((double)i * 3.141592653589793 * -2.0) / (double)8 + 1.5707963267948966;
		squidTentacles[i]->rotateAngleY = (float)d;
	}
}

void ModelSquid::setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5)
{
	for (int_t i = 0; i < 8; i++)
	{
		squidTentacles[i]->rotateAngleX = f2;
	}
}

void ModelSquid::render(float f, float f1, float f2, float f3, float f4, float f5)
{
	setRotationAngles(f, f1, f2, f3, f4, f5);
	squidBody->render(f5);
	for (int_t i = 0; i < 8; i++)
	{
		squidTentacles[i]->render(f5);
	}
}
