#pragma once

#include <vector>

#include "IChunkProvider.h"
#include "java/Type.h"
#include "java/Random.h"
#include "java/String.h"
#include "NoiseGeneratorOctaves.h"

class World;
class Chunk;
class BiomeGenBase;
class MapGenBase;
class IProgressUpdate;

// net.minecraft.src.ChunkProviderSky
class ChunkProviderSky : public IChunkProvider
{
public:
	ChunkProviderSky(World *world, long_t l);
	~ChunkProviderSky() override;

	void generateSkyTerrain(int_t i, int_t j, byte_t *abyte0,
	                        BiomeGenBase **abiomegenbase, double *ad);  // func_28071_a
	void replaceSkyBlocks(int_t i, int_t j, byte_t *abyte0,
	                      BiomeGenBase **abiomegenbase);                 // func_28072_a

	Chunk *prepareChunk(int_t i, int_t j) override;
	Chunk *provideChunk(int_t i, int_t j) override;

	bool    chunkExists(int_t i, int_t j) override;
	void    populate(IChunkProvider *ichunkprovider, int_t i, int_t j) override;
	bool    saveChunks(bool flag, IProgressUpdate *iprogressupdate) override;
	bool    unload100OldestChunks() override;
	bool    canSave() override;
	jstring makeString() override;

	// Must precede the NoiseGeneratorOctaves members (see ChunkProviderGenerate.h).
	Random field_28087_j;
	NoiseGeneratorOctaves field_28096_a;
	NoiseGeneratorOctaves field_28095_b;
	NoiseGeneratorOctaves field_28094_c;

private:
	std::vector<double> &generateNoiseField(std::vector<double> &ad, int_t i, int_t j, int_t k,
	                                        int_t l, int_t i1, int_t j1); // func_28073_a

	NoiseGeneratorOctaves field_28086_k;
	NoiseGeneratorOctaves field_28085_l;
	NoiseGeneratorOctaves field_28084_m;
	NoiseGeneratorOctaves field_28083_n;
	NoiseGeneratorOctaves field_28082_o;

	World *field_28081_p;

	std::vector<double> field_28080_q;
	std::vector<double> field_28079_r;
	std::vector<double> field_28078_s;
	std::vector<double> field_28077_t;

	MapGenBase *caveGenerator;  // field_28076_u

	std::vector<BiomeGenBase *> field_28075_v;

	std::vector<double> field_28093_d;
	std::vector<double> field_28092_e;
	std::vector<double> field_28091_f;
	std::vector<double> field_28090_g;
	std::vector<double> field_28089_h;
	int_t field_28088_i[32][32];

	std::vector<double> field_28074_w;
};
