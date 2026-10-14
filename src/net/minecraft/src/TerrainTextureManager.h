#pragma once

#include <vector>

#include "java/Type.h"

class IsoImageBuffer;

// net.minecraft.src.TerrainTextureManager
class TerrainTextureManager
{
public:
	TerrainTextureManager();

	void func_799_a(IsoImageBuffer *isoimagebuffer);

private:
	void func_800_a();
	bool loadTerrainPixels(std::vector<int_t> &pixels);

	float field_1181_a[768];
	int_t field_1180_b[5120];
	int_t field_1186_c[5120];
	int_t field_1185_d[5120];
	int_t field_1184_e[5120];
	int_t field_1183_f[34];
	int_t field_1182_g[768];
};
