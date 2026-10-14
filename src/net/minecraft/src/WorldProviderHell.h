#pragma once

#include "WorldProvider.h"

class WorldChunkManagerHell;
class ChunkProviderHell;

// net.minecraft.src.WorldProviderHell
class WorldProviderHell : public WorldProvider
{
public:
	WorldProviderHell() = default;

	void registerWorldChunkManager() override;
	Vec3D *getFogColor(float f, float f1) override; // func_4096_a — nether fog (dark red)
	IChunkProvider *getChunkProvider() override;
	bool canCoordinateBeSpawn(int_t i, int_t j) override;
	float calculateCelestialAngle(long_t l, float f) override;
	bool canRespawnHere() override;

protected:
	void generateLightBrightnessTable() override;
};
