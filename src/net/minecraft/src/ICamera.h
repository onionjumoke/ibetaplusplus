#pragma once

#include "AxisAlignedBB.h"

// net.minecraft.src.ICamera
class ICamera
{
public:
	virtual ~ICamera() = default;
	virtual bool isBoundingBoxInFrustum(AxisAlignedBB *axisalignedbb) = 0;
	virtual void setPosition(double d, double d1, double d2) = 0;
};
