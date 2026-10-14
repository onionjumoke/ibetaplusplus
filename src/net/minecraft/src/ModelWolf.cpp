#include "ModelWolf.h"

#include "EntityWolf.h"
#include "MathHelper.h"
#include "ModelRenderer.h"
#include "pc/OpenGL.h"

ModelWolf::ModelWolf()
{
	float f = 0.0f;
	float f1 = 13.5f;
	wolfHeadMain = new ModelRenderer(0, 0);
	wolfHeadMain->addBox(-3.0f, -3.0f, -2.0f, 6, 6, 4, f);
	wolfHeadMain->setRotationPoint(-1.0f, f1, -7.0f);
	wolfBody = new ModelRenderer(18, 14);
	wolfBody->addBox(-4.0f, -2.0f, -3.0f, 6, 9, 6, f);
	wolfBody->setRotationPoint(0.0f, 14.0f, 2.0f);
	wolfMane = new ModelRenderer(21, 0);
	wolfMane->addBox(-4.0f, -3.0f, -3.0f, 8, 6, 7, f);
	wolfMane->setRotationPoint(-1.0f, 14.0f, 2.0f);
	wolfLeg1 = new ModelRenderer(0, 18);
	wolfLeg1->addBox(-1.0f, 0.0f, -1.0f, 2, 8, 2, f);
	wolfLeg1->setRotationPoint(-2.5f, 16.0f, 7.0f);
	wolfLeg2 = new ModelRenderer(0, 18);
	wolfLeg2->addBox(-1.0f, 0.0f, -1.0f, 2, 8, 2, f);
	wolfLeg2->setRotationPoint(0.5f, 16.0f, 7.0f);
	wolfLeg3 = new ModelRenderer(0, 18);
	wolfLeg3->addBox(-1.0f, 0.0f, -1.0f, 2, 8, 2, f);
	wolfLeg3->setRotationPoint(-2.5f, 16.0f, -4.0f);
	wolfLeg4 = new ModelRenderer(0, 18);
	wolfLeg4->addBox(-1.0f, 0.0f, -1.0f, 2, 8, 2, f);
	wolfLeg4->setRotationPoint(0.5f, 16.0f, -4.0f);
	wolfTail = new ModelRenderer(9, 18);
	wolfTail->addBox(-1.0f, 0.0f, -1.0f, 2, 8, 2, f);
	wolfTail->setRotationPoint(-1.0f, 12.0f, 8.0f);
	wolfRightEar = new ModelRenderer(16, 14);
	wolfRightEar->addBox(-3.0f, -5.0f, 0.0f, 2, 2, 1, f);
	wolfRightEar->setRotationPoint(-1.0f, f1, -7.0f);
	wolfLeftEar = new ModelRenderer(16, 14);
	wolfLeftEar->addBox(1.0f, -5.0f, 0.0f, 2, 2, 1, f);
	wolfLeftEar->setRotationPoint(-1.0f, f1, -7.0f);
	wolfSnout = new ModelRenderer(0, 10);
	wolfSnout->addBox(-2.0f, 0.0f, -5.0f, 3, 3, 4, f);
	wolfSnout->setRotationPoint(-0.5f, f1, -7.0f);
}

void ModelWolf::render(float f, float f1, float f2, float f3, float f4, float f5)
{
	ModelBase::render(f, f1, f2, f3, f4, f5);
	setRotationAngles(f, f1, f2, f3, f4, f5);
	wolfHeadMain->renderWithRotation(f5);
	wolfBody->render(f5);
	wolfLeg1->render(f5);
	wolfLeg2->render(f5);
	wolfLeg3->render(f5);
	wolfLeg4->render(f5);
	wolfRightEar->renderWithRotation(f5);
	wolfLeftEar->renderWithRotation(f5);
	wolfSnout->renderWithRotation(f5);
	wolfTail->renderWithRotation(f5);
	wolfMane->render(f5);
}

void ModelWolf::setLivingAnimations(EntityLiving *entityliving, float f, float f1, float f2)
{
	EntityWolf *entitywolf = (EntityWolf *)entityliving;
	if (entitywolf->isWolfAngry())
	{
		wolfTail->rotateAngleY = 0.0f;
	}
	else
	{
		wolfTail->rotateAngleY = MathHelper::cos(f * 0.6662f) * 1.4f * f1;
	}
	if (entitywolf->isWolfSitting())
	{
		wolfMane->setRotationPoint(-1.0f, 16.0f, -3.0f);
		wolfMane->rotateAngleX = 1.256637f;
		wolfMane->rotateAngleY = 0.0f;
		wolfBody->setRotationPoint(0.0f, 18.0f, 0.0f);
		wolfBody->rotateAngleX = 0.7853982f;
		wolfTail->setRotationPoint(-1.0f, 21.0f, 6.0f);
		wolfLeg1->setRotationPoint(-2.5f, 22.0f, 2.0f);
		wolfLeg1->rotateAngleX = 4.712389f;
		wolfLeg2->setRotationPoint(0.5f, 22.0f, 2.0f);
		wolfLeg2->rotateAngleX = 4.712389f;
		wolfLeg3->rotateAngleX = 5.811947f;
		wolfLeg3->setRotationPoint(-2.49f, 17.0f, -4.0f);
		wolfLeg4->rotateAngleX = 5.811947f;
		wolfLeg4->setRotationPoint(0.51f, 17.0f, -4.0f);
	}
	else
	{
		wolfBody->setRotationPoint(0.0f, 14.0f, 2.0f);
		wolfBody->rotateAngleX = 1.570796f;
		wolfMane->setRotationPoint(-1.0f, 14.0f, -3.0f);
		wolfMane->rotateAngleX = wolfBody->rotateAngleX;
		wolfTail->setRotationPoint(-1.0f, 12.0f, 8.0f);
		wolfLeg1->setRotationPoint(-2.5f, 16.0f, 7.0f);
		wolfLeg2->setRotationPoint(0.5f, 16.0f, 7.0f);
		wolfLeg3->setRotationPoint(-2.5f, 16.0f, -4.0f);
		wolfLeg4->setRotationPoint(0.5f, 16.0f, -4.0f);
		wolfLeg1->rotateAngleX = MathHelper::cos(f * 0.6662f) * 1.4f * f1;
		wolfLeg2->rotateAngleX = MathHelper::cos(f * 0.6662f + 3.141593f) * 1.4f * f1;
		wolfLeg3->rotateAngleX = MathHelper::cos(f * 0.6662f + 3.141593f) * 1.4f * f1;
		wolfLeg4->rotateAngleX = MathHelper::cos(f * 0.6662f) * 1.4f * f1;
	}
	float f3 = entitywolf->getInterestedAngle(f2) + entitywolf->getShakeAngle(f2, 0.0f);
	wolfHeadMain->rotateAngleZ = f3;
	wolfRightEar->rotateAngleZ = f3;
	wolfLeftEar->rotateAngleZ = f3;
	wolfSnout->rotateAngleZ = f3;
	wolfMane->rotateAngleZ = entitywolf->getShakeAngle(f2, -0.08f);
	wolfBody->rotateAngleZ = entitywolf->getShakeAngle(f2, -0.16f);
	wolfTail->rotateAngleZ = entitywolf->getShakeAngle(f2, -0.2f);
	if (entitywolf->getWolfShaking())
	{
		float f4 = entitywolf->getEntityBrightness(f2) * entitywolf->getShadingWhileShaking(f2);
		glColor3f(f4, f4, f4);
	}
}

void ModelWolf::setRotationAngles(float f, float f1, float f2, float f3, float f4, float f5)
{
	ModelBase::setRotationAngles(f, f1, f2, f3, f4, f5);
	wolfHeadMain->rotateAngleX = f4 / 57.29578f;
	wolfHeadMain->rotateAngleY = f3 / 57.29578f;
	wolfRightEar->rotateAngleY = wolfHeadMain->rotateAngleY;
	wolfRightEar->rotateAngleX = wolfHeadMain->rotateAngleX;
	wolfLeftEar->rotateAngleY = wolfHeadMain->rotateAngleY;
	wolfLeftEar->rotateAngleX = wolfHeadMain->rotateAngleX;
	wolfSnout->rotateAngleY = wolfHeadMain->rotateAngleY;
	wolfSnout->rotateAngleX = wolfHeadMain->rotateAngleX;
	wolfTail->rotateAngleX = f2;
}
