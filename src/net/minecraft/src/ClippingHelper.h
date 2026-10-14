#pragma once

// net.minecraft.src.ClippingHelper
class ClippingHelper
{
public:
	ClippingHelper();

	bool isBoxInFrustum(double d, double d1, double d2, double d3, double d4, double d5);

	float frustum[16][16];
	float projectionMatrix[16];
	float modelviewMatrix[16];
	float clippingMatrix[16];
};
