#pragma once

#include <vector>

#include "java/Type.h"
#include "java/String.h"

// net.minecraft.src.Vec3D
class Vec3D
{
public:
	static Vec3D *createVectorHelper(double d, double d1, double d2);
	static void getCraftingManager();
	static void initialize();
	static void func_28215_a() { getCraftingManager(); } // reset pool
	static Vec3D *createVector(double d, double d1, double d2);

private:
	Vec3D(double d, double d1, double d2);
	Vec3D *setComponents(double d, double d1, double d2);

public:
	Vec3D *subtract(Vec3D *vec3d);
	Vec3D *normalize();
	Vec3D *crossProduct(Vec3D *vec3d);
	Vec3D *addVector(double d, double d1, double d2);
	double distanceTo(Vec3D *vec3d);
	double squareDistanceTo(Vec3D *vec3d);
	double squareDistanceTo(double d, double d1, double d2);
	double lengthVector();
	Vec3D *getIntermediateWithXValue(Vec3D *vec3d, double d);
	Vec3D *getIntermediateWithYValue(Vec3D *vec3d, double d);
	Vec3D *getIntermediateWithZValue(Vec3D *vec3d, double d);
	jstring toString();
	void rotateAroundX(float f);
	void rotateAroundY(float f);

private:
	static std::vector<Vec3D *> vectorList;
	static int_t nextVector;

public:
	double xCoord;
	double yCoord;
	double zCoord;
};
