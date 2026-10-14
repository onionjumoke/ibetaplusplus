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

// net.minecraft.src.ChunkProviderGenerate
class ChunkProviderGenerate : public IChunkProvider
{
public:
	ChunkProviderGenerate(World *world, long_t l);
	~ChunkProviderGenerate() override;

	void generateTerrain(int_t i, int_t j, byte_t *abyte0, BiomeGenBase **abiomegenbase, double *ad);
	void replaceBlocksForBiome(int_t i, int_t j, byte_t *abyte0, BiomeGenBase **abiomegenbase);

	Chunk *prepareChunk(int_t i, int_t j) override;
	Chunk *provideChunk(int_t i, int_t j) override;

	bool    chunkExists(int_t i, int_t j) override;
	void    populate(IChunkProvider *ichunkprovider, int_t i, int_t j) override;
	bool    saveChunks(bool flag, IProgressUpdate *iprogressupdate) override;
	bool    unload100OldestChunks() override;
	bool    canSave() override;
	jstring makeString() override;

	// `rand` MUST be declared before every NoiseGeneratorOctaves member: C++
	// constructs members in declaration order, not initializer-list order. When
	// a generator was declared first it captured a default-constructed Random
	// (clock-seeded) instead of `rand(l)`, so terrain noise differed on every
	// process start and generated chunk seams at session boundaries.
	Random rand;
	NoiseGeneratorOctaves field_922_a;
	NoiseGeneratorOctaves field_921_b;
	NoiseGeneratorOctaves mobSpawnerNoise;

private:
	std::vector<double> &generateNoiseField(std::vector<double> &ad, int_t i, int_t j, int_t k,
	                                        int_t l, int_t i1, int_t j1); // func_4061_a

	NoiseGeneratorOctaves field_912_k;
	NoiseGeneratorOctaves field_911_l;
	NoiseGeneratorOctaves field_910_m;
	NoiseGeneratorOctaves field_909_n;
	NoiseGeneratorOctaves field_908_o;

	World *worldObj;

	std::vector<double> field_4180_q;
	std::vector<double> sandNoise;
	std::vector<double> gravelNoise;
	std::vector<double> stoneNoise;

	MapGenBase *caveGenerator;  // field_902_u

	std::vector<BiomeGenBase *> biomesForGeneration;

	std::vector<double> field_4185_d;
	std::vector<double> field_4184_e;
	std::vector<double> field_4183_f;
	std::vector<double> field_4182_g;
	std::vector<double> field_4181_h;
	int_t field_914_i[32][32];

	std::vector<double> generatedTemperatures;
};
