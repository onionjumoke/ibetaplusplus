#pragma once

#include <vector>

#include "IChunkProvider.h"
#include "java/Type.h"
#include "java/Random.h"
#include "java/String.h"
#include "NoiseGeneratorOctaves.h"

class World;
class Chunk;
class MapGenBase;
class IProgressUpdate;

// net.minecraft.src.ChunkProviderHell
class ChunkProviderHell : public IChunkProvider
{
public:
	ChunkProviderHell(World *world, long_t l);
	~ChunkProviderHell() override;

	void generateHellTerrain(int_t i, int_t j, byte_t *abyte0);  // func_4059_a
	void replaceHellBlocks(int_t i, int_t j, byte_t *abyte0);    // func_4058_b

	Chunk *prepareChunk(int_t i, int_t j) override;
	Chunk *provideChunk(int_t i, int_t j) override;

	bool    chunkExists(int_t i, int_t j) override;
	void    populate(IChunkProvider *ichunkprovider, int_t i, int_t j) override;
	bool    saveChunks(bool flag, IProgressUpdate *iprogressupdate) override;
	bool    unload100OldestChunks() override;
	bool    canSave() override;
	jstring makeString() override;

	// Must precede the NoiseGeneratorOctaves members (see ChunkProviderGenerate.h).
	Random hellRNG;
	NoiseGeneratorOctaves field_4177_a;
	NoiseGeneratorOctaves field_4176_b;

private:
	std::vector<double> &generateNoiseField(std::vector<double> &ad, int_t i, int_t j, int_t k,
	                                        int_t l, int_t i1, int_t j1); // func_4057_a

	NoiseGeneratorOctaves field_4169_i;
	NoiseGeneratorOctaves field_4168_j;
	NoiseGeneratorOctaves field_4167_k;
	NoiseGeneratorOctaves field_4166_l;
	NoiseGeneratorOctaves field_4165_m;

	World *worldObj;

	std::vector<double> field_4163_o;
	std::vector<double> field_4162_p;
	std::vector<double> field_4161_q;
	std::vector<double> field_4160_r;

	MapGenBase *caveGenerator;  // field_4159_s

	std::vector<double> field_4175_c;
	std::vector<double> field_4174_d;
	std::vector<double> field_4173_e;
	std::vector<double> field_4172_f;
	std::vector<double> field_4171_g;
};
