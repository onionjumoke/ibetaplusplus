#include "Vec3D.h"

#include "MathHelper.h"

std::vector<Vec3D *> Vec3D::vectorList;
int_t Vec3D::nextVector = 0;

Vec3D *Vec3D::createVectorHelper(double d, double d1, double d2)
{
	return new Vec3D(d, d1, d2);
}

void Vec3D::getCraftingManager()
{
	for (Vec3D *vector : vectorList)
		delete vector;
	vectorList.clear();
	nextVector = 0;
}

void Vec3D::initialize()
{
	// Per-frame pool reset. Cap growth for low-RAM targets (see AxisAlignedBB::clearBoundingBoxPool).
	int_t cap = nextVector * 2 + 256;
	if ((int_t)vectorList.size() > cap)
	{
		for (int_t i = cap; i < (int_t)vectorList.size(); i++)
			delete vectorList[i];
		vectorList.resize(cap);
		vectorList.shrink_to_fit();
	}
	nextVector = 0;
}

Vec3D *Vec3D::createVector(double d, double d1, double d2)
{
	if (nextVector >= (int_t)vectorList.size())
		vectorList.push_back(createVectorHelper(0.0, 0.0, 0.0));
	return vectorList[nextVector++]->setComponents(d, d1, d2);
}

Vec3D::Vec3D(double d, double d1, double d2)
{
	if (d == -0.0)
		d = 0.0;
	if (d1 == -0.0)
		d1 = 0.0;
	if (d2 == -0.0)
		d2 = 0.0;
	xCoord = d;
	yCoord = d1;
	zCoord = d2;
}

Vec3D *Vec3D::setComponents(double d, double d1, double d2)
{
	xCoord = d;
	yCoord = d1;
	zCoord = d2;
	return this;
}

Vec3D *Vec3D::subtract(Vec3D *vec3d)
{
	return createVector(vec3d->xCoord - xCoord, vec3d->yCoord - yCoord, vec3d->zCoord - zCoord);
}

Vec3D *Vec3D::normalize()
{
	double d = MathHelper::sqrt_double(xCoord * xCoord + yCoord * yCoord + zCoord * zCoord);
	if (d < 0.0001)
		return createVector(0.0, 0.0, 0.0);
	else
		return createVector(xCoord / d, yCoord / d, zCoord / d);
}

Vec3D *Vec3D::crossProduct(Vec3D *vec3d)
{
	return createVector(yCoord * vec3d->zCoord - zCoord * vec3d->yCoord, zCoord * vec3d->xCoord - xCoord * vec3d->zCoord, xCoord * vec3d->yCoord - yCoord * vec3d->xCoord);
}

Vec3D *Vec3D::addVector(double d, double d1, double d2)
{
	return createVector(xCoord + d, yCoord + d1, zCoord + d2);
}

double Vec3D::distanceTo(Vec3D *vec3d)
{
	double d = vec3d->xCoord - xCoord;
	double d1 = vec3d->yCoord - yCoord;
	double d2 = vec3d->zCoord - zCoord;
	return (double)MathHelper::sqrt_double(d * d + d1 * d1 + d2 * d2);
}

double Vec3D::squareDistanceTo(Vec3D *vec3d)
{
	double d = vec3d->xCoord - xCoord;
	double d1 = vec3d->yCoord - yCoord;
	double d2 = vec3d->zCoord - zCoord;
	return d * d + d1 * d1 + d2 * d2;
}

double Vec3D::squareDistanceTo(double d, double d1, double d2)
{
	double d3 = d - xCoord;
	double d4 = d1 - yCoord;
	double d5 = d2 - zCoord;
	return d3 * d3 + d4 * d4 + d5 * d5;
}

double Vec3D::lengthVector()
{
	return (double)MathHelper::sqrt_double(xCoord * xCoord + yCoord * yCoord + zCoord * zCoord);
}

Vec3D *Vec3D::getIntermediateWithXValue(Vec3D *vec3d, double d)
{
	double d1 = vec3d->xCoord - xCoord;
	double d2 = vec3d->yCoord - yCoord;
	double d3 = vec3d->zCoord - zCoord;
	if (d1 * d1 < 1.0000000116860974E-007)
		return nullptr;
	double d4 = (d - xCoord) / d1;
	if (d4 < 0.0 || d4 > 1.0)
		return nullptr;
	else
		return createVector(xCoord + d1 * d4, yCoord + d2 * d4, zCoord + d3 * d4);
}

Vec3D *Vec3D::getIntermediateWithYValue(Vec3D *vec3d, double d)
{
	double d1 = vec3d->xCoord - xCoord;
	double d2 = vec3d->yCoord - yCoord;
	double d3 = vec3d->zCoord - zCoord;
	if (d2 * d2 < 1.0000000116860974E-007)
		return nullptr;
	double d4 = (d - yCoord) / d2;
	if (d4 < 0.0 || d4 > 1.0)
		return nullptr;
	else
		return createVector(xCoord + d1 * d4, yCoord + d2 * d4, zCoord + d3 * d4);
}

Vec3D *Vec3D::getIntermediateWithZValue(Vec3D *vec3d, double d)
{
	double d1 = vec3d->xCoord - xCoord;
	double d2 = vec3d->yCoord - yCoord;
	double d3 = vec3d->zCoord - zCoord;
	if (d3 * d3 < 1.0000000116860974E-007)
		return nullptr;
	double d4 = (d - zCoord) / d3;
	if (d4 < 0.0 || d4 > 1.0)
		return nullptr;
	else
		return createVector(xCoord + d1 * d4, yCoord + d2 * d4, zCoord + d3 * d4);
}

jstring Vec3D::toString()
{
	return u"(" + String::toString(xCoord) + u", " + String::toString(yCoord) + u", " + String::toString(zCoord) + u")";
}

void Vec3D::rotateAroundX(float f)
{
	float f1 = MathHelper::cos(f);
	float f2 = MathHelper::sin(f);
	double d = xCoord;
	double d1 = yCoord * (double)f1 + zCoord * (double)f2;
	double d2 = zCoord * (double)f1 - yCoord * (double)f2;
	xCoord = d;
	yCoord = d1;
	zCoord = d2;
}

void Vec3D::rotateAroundY(float f)
{
	float f1 = MathHelper::cos(f);
	float f2 = MathHelper::sin(f);
	double d = xCoord * (double)f1 + zCoord * (double)f2;
	double d1 = yCoord;
	double d2 = zCoord * (double)f1 - xCoord * (double)f2;
	xCoord = d;
	yCoord = d1;
	zCoord = d2;
}
