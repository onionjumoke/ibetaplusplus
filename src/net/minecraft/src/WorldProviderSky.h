#pragma once

#include "WorldProvider.h"

class WorldChunkManagerHell;
class ChunkProviderSky;

// net.minecraft.src.WorldProviderSky
class WorldProviderSky : public WorldProvider
{
public:
	WorldProviderSky() = default;

	void registerWorldChunkManager() override;
	IChunkProvider *getChunkProvider() override;
	float calculateCelestialAngle(long_t l, float f) override;
	float *calcSunriseSunsetColors(float f, float f1) override;
	Vec3D *getFogColor(float f, float f1) override; // func_4096_a override for The End
	bool hasVoidParticles() override;
	float getCloudHeight() override;
	bool canCoordinateBeSpawn(int_t i, int_t j) override;
};
