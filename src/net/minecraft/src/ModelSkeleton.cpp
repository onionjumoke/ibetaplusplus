#include "ModelSkeleton.h"

#include "ModelRenderer.h"

ModelSkeleton::ModelSkeleton()
{
	float f = 0.0f;
	bipedRightArm = new ModelRenderer(40, 16);
	bipedRightArm->addBox(-1.0f, -2.0f, -1.0f, 2, 12, 2, f);
	bipedRightArm->setRotationPoint(-5.0f, 2.0f, 0.0f);
	bipedLeftArm = new ModelRenderer(40, 16);
	bipedLeftArm->mirror = true;
	bipedLeftArm->addBox(-1.0f, -2.0f, -1.0f, 2, 12, 2, f);
	bipedLeftArm->setRotationPoint(5.0f, 2.0f, 0.0f);
	bipedRightLeg = new ModelRenderer(0, 16);
	bipedRightLeg->addBox(-1.0f, 0.0f, -1.0f, 2, 12, 2, f);
	bipedRightLeg->setRotationPoint(-2.0f, 12.0f, 0.0f);
	bipedLeftLeg = new ModelRenderer(0, 16);
	bipedLeftLeg->mirror = true;
	bipedLeftLeg->addBox(-1.0f, 0.0f, -1.0f, 2, 12, 2, f);
	bipedLeftLeg->setRotationPoint(2.0f, 12.0f, 0.0f);
}
