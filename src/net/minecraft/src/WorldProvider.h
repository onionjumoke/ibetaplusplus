#pragma once

#include <array>
#include "java/Type.h"
#include "Vec3D.h"
#include "MathHelper.h"
#include "IChunkProvider.h"

class World;
class WorldChunkManager;

// net.minecraft.src.WorldProvider
class WorldProvider
{
public:
	WorldProvider();
	virtual ~WorldProvider();

	void registerWorld(World *world);
	virtual void registerWorldChunkManager();
	virtual IChunkProvider *getChunkProvider();
	virtual bool canCoordinateBeSpawn(int_t i, int_t j);
	virtual float calculateCelestialAngle(long_t l, float f);
	virtual float *calcSunriseSunsetColors(float f, float f1);
	virtual Vec3D *getFogColor(float celestialAngle, float partialTicks);
	virtual bool canRespawnHere();
	virtual float getCloudHeight();
	virtual bool hasVoidParticles();
	// func_28112_c — true if the sky/horizon should have the normal color blend applied
	virtual bool hasSkyColorBlend() { return true; }

	static WorldProvider *getProviderForDimension(int_t i);

protected:
	virtual void generateLightBrightnessTable();

public:
	World *worldObj = nullptr;
	WorldChunkManager *worldChunkMgr = nullptr;
	bool isNether;
	bool isHellWorld;
	bool hasNoSky;
	float lightBrightnessTable[16];
	int_t worldType;

private:
	float colorsSunriseSunset[4];
};
