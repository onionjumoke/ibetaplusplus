#pragma once

class Vec3D;

// net.minecraft.src.PositionTextureVertex
class PositionTextureVertex
{
public:
	PositionTextureVertex(float f, float f1, float f2, float f3, float f4);
	PositionTextureVertex(PositionTextureVertex *positiontexturevertex, float f, float f1);
	PositionTextureVertex(Vec3D *vec3d, float f, float f1);

	PositionTextureVertex *setTexturePosition(float f, float f1);

	Vec3D *vector3D;
	float texturePositionX;
	float texturePositionY;
};
