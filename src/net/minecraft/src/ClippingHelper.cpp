#include "ClippingHelper.h"

ClippingHelper::ClippingHelper()
{
	for (int i = 0; i < 16; i++)
	{
		projectionMatrix[i] = 0.0f;
		modelviewMatrix[i] = 0.0f;
		clippingMatrix[i] = 0.0f;
		for (int j = 0; j < 16; j++)
			frustum[i][j] = 0.0f;
	}
}

bool ClippingHelper::isBoxInFrustum(double d, double d1, double d2, double d3, double d4, double d5)
{
	for (int i = 0; i < 6; i++)
	{
		if ((double)frustum[i][0] * d + (double)frustum[i][1] * d1 + (double)frustum[i][2] * d2 + (double)frustum[i][3] <= 0.0
			&& (double)frustum[i][0] * d3 + (double)frustum[i][1] * d1 + (double)frustum[i][2] * d2 + (double)frustum[i][3] <= 0.0
			&& (double)frustum[i][0] * d + (double)frustum[i][1] * d4 + (double)frustum[i][2] * d2 + (double)frustum[i][3] <= 0.0
			&& (double)frustum[i][0] * d3 + (double)frustum[i][1] * d4 + (double)frustum[i][2] * d2 + (double)frustum[i][3] <= 0.0
			&& (double)frustum[i][0] * d + (double)frustum[i][1] * d1 + (double)frustum[i][2] * d5 + (double)frustum[i][3] <= 0.0
			&& (double)frustum[i][0] * d3 + (double)frustum[i][1] * d1 + (double)frustum[i][2] * d5 + (double)frustum[i][3] <= 0.0
			&& (double)frustum[i][0] * d + (double)frustum[i][1] * d4 + (double)frustum[i][2] * d5 + (double)frustum[i][3] <= 0.0
			&& (double)frustum[i][0] * d3 + (double)frustum[i][1] * d4 + (double)frustum[i][2] * d5 + (double)frustum[i][3] <= 0.0)
		{
			return false;
		}
	}
	return true;
}
