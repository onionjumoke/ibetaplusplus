#pragma once

#include <vector>

#include "java/Type.h"
#include "java/String.h"

class Vec3D;
class MovingObjectPosition;

// net.minecraft.src.AxisAlignedBB
class AxisAlignedBB
{
public:
	static AxisAlignedBB *getBoundingBox(double d, double d1, double d2, double d3, double d4, double d5);
	static void getStatMap();
	static void clearBoundingBoxPool();
	static void func_28196_a() { clearBoundingBoxPool(); } // reset pool
	static AxisAlignedBB *getBoundingBoxFromPool(double d, double d1, double d2, double d3, double d4, double d5);

private:
	AxisAlignedBB(double d, double d1, double d2, double d3, double d4, double d5);

public:
	AxisAlignedBB *setBounds(double d, double d1, double d2, double d3, double d4, double d5);
	AxisAlignedBB *addCoord(double d, double d1, double d2);
	AxisAlignedBB *expand(double d, double d1, double d2);
	AxisAlignedBB *contract(double d, double d1, double d2); // Java func_28195_e
	AxisAlignedBB *getOffsetBoundingBox(double d, double d1, double d2);
	double calculateXOffset(AxisAlignedBB *axisalignedbb, double d);
	double calculateYOffset(AxisAlignedBB *axisalignedbb, double d);
	double calculateZOffset(AxisAlignedBB *axisalignedbb, double d);
	bool intersectsWith(AxisAlignedBB *axisalignedbb);
	AxisAlignedBB *offset(double d, double d1, double d2);
	bool isVecInside(Vec3D *vec3d);
	double getAverageEdgeLength();
	AxisAlignedBB *getStatList(double d, double d1, double d2);
	AxisAlignedBB *copy();
	MovingObjectPosition *calculateIntercept(Vec3D *vec3d, Vec3D *vec3d1);

private:
	bool isVecInYZ(Vec3D *vec3d);
	bool isVecInXZ(Vec3D *vec3d);
	bool isVecInXY(Vec3D *vec3d);

public:
	void setBB(AxisAlignedBB *axisalignedbb);
	jstring toString();

private:
	static std::vector<AxisAlignedBB *> boundingBoxes;
	static int_t numBoundingBoxesInUse;

public:
	double minX;
	double minY;
	double minZ;
	double maxX;
	double maxY;
	double maxZ;
};
