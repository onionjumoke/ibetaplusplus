#include "PositionTextureVertex.h"

#include "Vec3D.h"

PositionTextureVertex::PositionTextureVertex(float f, float f1, float f2, float f3, float f4) :
	PositionTextureVertex(Vec3D::createVectorHelper(f, f1, f2), f3, f4)
{
}

PositionTextureVertex *PositionTextureVertex::setTexturePosition(float f, float f1)
{
	return new PositionTextureVertex(this, f, f1);
}

PositionTextureVertex::PositionTextureVertex(PositionTextureVertex *positiontexturevertex, float f, float f1)
{
	vector3D = positiontexturevertex->vector3D;
	texturePositionX = f;
	texturePositionY = f1;
}

PositionTextureVertex::PositionTextureVertex(Vec3D *vec3d, float f, float f1)
{
	vector3D = vec3d;
	texturePositionX = f;
	texturePositionY = f1;
}
