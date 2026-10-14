#include "ModelCow.h"

#include "ModelRenderer.h"

ModelCow::ModelCow() : ModelQuadruped(12, 0.0f)
{
	head = new ModelRenderer(0, 0);
	head->addBox(-4.0f, -4.0f, -6.0f, 8, 8, 6, 0.0f);
	head->setRotationPoint(0.0f, 4.0f, -8.0f);
	horn1 = new ModelRenderer(22, 0);
	horn1->addBox(-4.0f, -5.0f, -4.0f, 1, 3, 1, 0.0f);
	horn1->setRotationPoint(0.0f, 3.0f, -7.0f);
	horn2 = new ModelRenderer(22, 0);
	horn2->addBox(3.0f, -5.0f, -4.0f, 1, 3, 1, 0.0f);
	horn2->setRotationPoint(0.0f, 3.0f, -7.0f);
	udders = new ModelRenderer(52, 0);
	udders->addBox(-2.0f, -3.0f, 0.0f, 4, 6, 2, 0.0f);
	udders->setRotationPoint(0.0f, 14.0f, 6.0f);
	udders->rotateAngleX = 1.570796f;
	body = new ModelRenderer(18, 4);
	body->addBox(-6.0f, -10.0f, -7.0f, 12, 18, 10, 0.0f);
	body->setRotationPoint(0.0f, 5.0f, 2.0f);
	leg1->rotationPointX--;
	leg2->rotationPointX++;
	leg1->rotationPointZ += 0.0f;
	leg2->rotationPointZ += 0.0f;
	leg3->rotationPointX--;
	leg4->rotationPointX++;
	leg3->rotationPointZ--;
	leg4->rotationPointZ--;
}

void ModelCow::render(float f, float f1, float f2, float f3, float f4, float f5)
{
	ModelQuadruped::render(f, f1, f2, f3, f4, f5);
	horn1->render(f5);
	horn2->render(f5);
	udders->render(f5);
}

void ModelCow::setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5)
{
	ModelQuadruped::setRotationAngles(f, f1, f2, f3, f4, f5);
	horn1->rotateAngleY = head->rotateAngleY;
	horn1->rotateAngleX = head->rotateAngleX;
	horn2->rotateAngleY = head->rotateAngleY;
	horn2->rotateAngleX = head->rotateAngleX;
}
