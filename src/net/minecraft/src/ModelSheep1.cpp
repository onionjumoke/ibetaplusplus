#include "ModelSheep1.h"

#include "ModelRenderer.h"

ModelSheep1::ModelSheep1() : ModelQuadruped(12, 0.0f)
{
	head = new ModelRenderer(0, 0);
	head->addBox(-3.0f, -4.0f, -4.0f, 6, 6, 6, 0.6f);
	head->setRotationPoint(0.0f, 6.0f, -8.0f);
	body = new ModelRenderer(28, 8);
	body->addBox(-4.0f, -10.0f, -7.0f, 8, 16, 6, 1.75f);
	body->setRotationPoint(0.0f, 5.0f, 2.0f);
	float f = 0.5f;
	leg1 = new ModelRenderer(0, 16);
	leg1->addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, f);
	leg1->setRotationPoint(-3.0f, 12.0f, 7.0f);
	leg2 = new ModelRenderer(0, 16);
	leg2->addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, f);
	leg2->setRotationPoint(3.0f, 12.0f, 7.0f);
	leg3 = new ModelRenderer(0, 16);
	leg3->addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, f);
	leg3->setRotationPoint(-3.0f, 12.0f, -5.0f);
	leg4 = new ModelRenderer(0, 16);
	leg4->addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, f);
	leg4->setRotationPoint(3.0f, 12.0f, -5.0f);
}
