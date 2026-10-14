#include "ChunkProviderGenerate.h"

#include <cmath>
#include <cstring>

#include "World.h"
#include "Chunk.h"
#include "Block.h"
#include "BiomeGenBase.h"
#include "Material.h"
#include "MapGenCaves.h"
#include "WorldChunkManager.h"
#include "WorldGenLakes.h"
#include "WorldGenDungeons.h"
#include "WorldGenClay.h"
#include "WorldGenMinable.h"
#include "WorldGenFlowers.h"
#include "WorldGenTallGrass.h"
#include "WorldGenDeadBush.h"
#include "WorldGenReed.h"
#include "WorldGenPumpkin.h"
#include "WorldGenCactus.h"
#include "WorldGenLiquids.h"
#include "BlockSand.h"
#include "BlockDeadBush.h"
#include "BlockFlower.h"
#include "BlockTallGrass.h"

ChunkProviderGenerate::ChunkProviderGenerate(World *world, long_t l)
	: worldObj(world)
	, rand(l)
	, field_912_k(rand, 16)
	, field_911_l(rand, 16)
	, field_910_m(rand, 8)
	, field_909_n(rand, 4)
	, field_908_o(rand, 4)
	, field_922_a(rand, 10)
	, field_921_b(rand, 16)
	, mobSpawnerNoise(rand, 8)
	, biomesForGeneration()
{
	sandNoise.resize(256);
	gravelNoise.resize(256);
	stoneNoise.resize(256);
	caveGenerator = new MapGenCaves();
	std::memset(field_914_i, 0, sizeof(field_914_i));
}

ChunkProviderGenerate::~ChunkProviderGenerate()
{
	delete caveGenerator;
}

void ChunkProviderGenerate::generateTerrain(int_t i, int_t j, byte_t *abyte0,
	BiomeGenBase **abiomegenbase, double *ad)
{
	const int_t byte0 = 4;
	const int_t byte1 = 64;
	int_t k = byte0 + 1;
	const int_t byte2 = 17;
	int_t l = byte0 + 1;
	field_4180_q = generateNoiseField(field_4180_q, i * byte0, 0, j * byte0, k, byte2, l);

	for (int_t i1 = 0; i1 < byte0; i1++)
	{
		for (int_t j1 = 0; j1 < byte0; j1++)
		{
			for (int_t k1 = 0; k1 < 16; k1++)
			{
				double d = 0.125;
				double d1 = field_4180_q[((i1 + 0) * l + (j1 + 0)) * byte2 + (k1 + 0)];
				double d2 = field_4180_q[((i1 + 0) * l + (j1 + 1)) * byte2 + (k1 + 0)];
				double d3 = field_4180_q[((i1 + 1) * l + (j1 + 0)) * byte2 + (k1 + 0)];
				double d4 = field_4180_q[((i1 + 1) * l + (j1 + 1)) * byte2 + (k1 + 0)];
				double d5 = (field_4180_q[((i1 + 0) * l + (j1 + 0)) * byte2 + (k1 + 1)] - d1) * d;
				double d6 = (field_4180_q[((i1 + 0) * l + (j1 + 1)) * byte2 + (k1 + 1)] - d2) * d;
				double d7 = (field_4180_q[((i1 + 1) * l + (j1 + 0)) * byte2 + (k1 + 1)] - d3) * d;
				double d8 = (field_4180_q[((i1 + 1) * l + (j1 + 1)) * byte2 + (k1 + 1)] - d4) * d;

				for (int_t l1 = 0; l1 < 8; l1++)
				{
					double d9  = 0.25;
					double d10 = d1;
					double d11 = d2;
					double d12 = (d3 - d1) * d9;
					double d13 = (d4 - d2) * d9;

					for (int_t i2 = 0; i2 < 4; i2++)
					{
						int_t j2 = (i2 + i1 * 4) << 11 | (0 + j1 * 4) << 7 | (k1 * 8 + l1);
						const int_t stride = 128; // '\200'
						double d14 = 0.25;
						double d15 = d10;
						double d16 = (d11 - d10) * d14;

						for (int_t k2 = 0; k2 < 4; k2++)
						{
							double d17 = ad[(i1 * 4 + i2) * 16 + (j1 * 4 + k2)];
							int_t l2 = 0;
							if (k1 * 8 + l1 < byte1)
							{
								if (d17 < 0.5 && k1 * 8 + l1 >= byte1 - 1)
									l2 = Block::ice->blockID;
								else
									l2 = Block::waterStill->blockID;
							}
							if (d15 > 0.0)
								l2 = Block::stone->blockID;
							abyte0[j2] = (byte_t)l2;
							j2 += stride;
							d15 += d16;
						}
						d10 += d12;
						d11 += d13;
					}
					d1 += d5;
					d2 += d6;
					d3 += d7;
					d4 += d8;
				}
			}
		}
	}
}

void ChunkProviderGenerate::replaceBlocksForBiome(int_t i, int_t j, byte_t *abyte0,
	BiomeGenBase **abiomegenbase)
{
	const int_t byte0 = 64;
	double d = 0.03125;
	sandNoise   = field_909_n.generateNoiseOctaves(sandNoise,   (double)(i * 16), (double)(j * 16), 0.0, 16, 16, 1, d, d, 1.0);
	gravelNoise = field_909_n.generateNoiseOctaves(gravelNoise, (double)(i * 16), 109.0134, (double)(j * 16), 16, 1, 16, d, 1.0, d);
	stoneNoise  = field_908_o.generateNoiseOctaves(stoneNoise,  (double)(i * 16), (double)(j * 16), 0.0, 16, 16, 1, d * 2.0, d * 2.0, d * 2.0);

	for (int_t k = 0; k < 16; k++)
	{
		for (int_t l = 0; l < 16; l++)
		{
			BiomeGenBase *biomegenbase = abiomegenbase[k + l * 16];
			bool flag  = sandNoise[k + l * 16]   + rand.nextDouble() * 0.2 > 0.0;
			bool flag1 = gravelNoise[k + l * 16] + rand.nextDouble() * 0.2 > 3.0;
			int_t i1 = (int_t)(stoneNoise[k + l * 16] / 3.0 + 3.0 + rand.nextDouble() * 0.25);
			int_t j1 = -1;
			byte_t byte1 = biomegenbase->topBlock;
			byte_t byte2 = biomegenbase->fillerBlock;

			for (int_t k1 = 127; k1 >= 0; k1--)
			{
				int_t l1 = (l * 16 + k) * 128 + k1;
				if (k1 <= 0 + rand.nextInt(5))
				{
					abyte0[l1] = (byte_t)Block::bedrock->blockID;
					continue;
				}
				byte_t byte3 = abyte0[l1];
				if (byte3 == 0) { j1 = -1; continue; }
				if (byte3 != (byte_t)Block::stone->blockID) continue;

				if (j1 == -1)
				{
					if (i1 <= 0)
					{
						byte1 = 0;
						byte2 = (byte_t)Block::stone->blockID;
					}
					else if (k1 >= byte0 - 4 && k1 <= byte0 + 1)
					{
						byte1 = biomegenbase->topBlock;
						byte2 = biomegenbase->fillerBlock;
						if (flag1) { byte1 = 0; }
						if (flag1) { byte2 = (byte_t)Block::gravel->blockID; }
						if (flag)  { byte1 = (byte_t)Block::sand->blockID; }
						if (flag)  { byte2 = (byte_t)Block::sand->blockID; }
					}
					if (k1 < byte0 && byte1 == 0)
						byte1 = (byte_t)Block::waterStill->blockID;
					j1 = i1;
					if (k1 >= byte0 - 1)
						abyte0[l1] = byte1;
					else
						abyte0[l1] = byte2;
					continue;
				}
				if (j1 <= 0) continue;
				j1--;
				abyte0[l1] = byte2;
				if (j1 == 0 && byte2 == (byte_t)Block::sand->blockID)
				{
					j1 = rand.nextInt(4);
					byte2 = (byte_t)Block::sandStone->blockID;
				}
			}
		}
	}
}

Chunk *ChunkProviderGenerate::prepareChunk(int_t i, int_t j)
{
	return provideChunk(i, j);
}

Chunk *ChunkProviderGenerate::provideChunk(int_t i, int_t j)
{
	rand.setSeed((long_t)i * 0x4f9939f508LL + (long_t)j * 0x1ef1565bd5LL);
	std::vector<byte_t> abyte0(32768, 0);
	biomesForGeneration = worldObj->getWorldChunkManager()->loadBlockGeneratorData(biomesForGeneration, i * 16, j * 16, 16, 16);
	double *ad = worldObj->getWorldChunkManager()->temperature.data();
	generateTerrain(i, j, abyte0.data(), biomesForGeneration.data(), ad);
	replaceBlocksForBiome(i, j, abyte0.data(), biomesForGeneration.data());
	caveGenerator->generate(this, worldObj, i, j, abyte0.data());
	Chunk *chunk = new Chunk(worldObj, abyte0, i, j);
		chunk->hasRealData = true;
	chunk->generateSkylightMap();
	return chunk;
}

std::vector<double> &ChunkProviderGenerate::generateNoiseField(std::vector<double> &ad,
	int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1)
{
	if (ad.empty())
		ad.resize((size_t)(l * i1 * j1));

	const double d  = 684.412;
	const double d1 = 684.412;
	double *ad1 = worldObj->getWorldChunkManager()->temperature.data();
	double *ad2 = worldObj->getWorldChunkManager()->humidity.data();

	field_4182_g = field_922_a.getBiomeGenForCoords(field_4182_g, i, k, l, j1, 1.121, 1.121, 0.5);
	field_4181_h = field_921_b.getBiomeGenForCoords(field_4181_h, i, k, l, j1, 200.0, 200.0, 0.5);
	field_4185_d = field_910_m.generateNoiseOctaves(field_4185_d, (double)i, (double)j, (double)k, l, i1, j1, d / 80.0, d1 / 160.0, d / 80.0);
	field_4184_e = field_912_k.generateNoiseOctaves(field_4184_e, (double)i, (double)j, (double)k, l, i1, j1, d,        d1,          d);
	field_4183_f = field_911_l.generateNoiseOctaves(field_4183_f, (double)i, (double)j, (double)k, l, i1, j1, d,        d1,          d);

	int_t k1 = 0;
	int_t l1 = 0;
	int_t i2 = 16 / l;

	for (int_t j2 = 0; j2 < l; j2++)
	{
		int_t k2 = j2 * i2 + i2 / 2;
		for (int_t l2 = 0; l2 < j1; l2++)
		{
			int_t i3 = l2 * i2 + i2 / 2;
			double d2 = ad1[k2 * 16 + i3];
			double d3 = ad2[k2 * 16 + i3] * d2;
			double d4 = 1.0 - d3;
			d4 *= d4;
			d4 *= d4;
			d4 = 1.0 - d4;
			double d5 = (field_4182_g[l1] + 256.0) / 512.0;
			d5 *= d4;
			if (d5 > 1.0) d5 = 1.0;
			double d6 = field_4181_h[l1] / 8000.0;
			if (d6 < 0.0) d6 = -d6 * 0.3;
			d6 = d6 * 3.0 - 2.0;
			if (d6 < 0.0)
			{
				d6 /= 2.0;
				if (d6 < -1.0) d6 = -1.0;
				d6 /= 1.4;
				d6 /= 2.0;
				d5 = 0.0;
			}
			else
			{
				if (d6 > 1.0) d6 = 1.0;
				d6 /= 8.0;
			}
			if (d5 < 0.0) d5 = 0.0;
			d5 += 0.5;
			d6 = (d6 * (double)i1) / 16.0;
			l1++;
			double d7 = (double)i1 / 2.0 + d6 * 4.0;

			for (int_t j3 = 0; j3 < i1; j3++)
			{
				double d8 = 0.0;
				double d9 = ((double)j3 - d7) * 12.0 / d5;
				if (d9 < 0.0) d9 *= 4.0;
				double d10 = field_4184_e[k1] / 512.0;
				double d11 = field_4183_f[k1] / 512.0;
				double d12 = (field_4185_d[k1] / 10.0 + 1.0) / 2.0;
				if (d12 < 0.0)       d8 = d10;
				else if (d12 > 1.0)  d8 = d11;
				else                  d8 = d10 + (d11 - d10) * d12;
				d8 -= d9;
				if (j3 > i1 - 4)
				{
					double d13 = (float)(j3 - (i1 - 4)) / 3.0f;
					d8 = d8 * (1.0 - d13) + -10.0 * d13;
				}
				ad[k1] = d8;
				k1++;
			}
		}
	}
	return ad;
}

bool ChunkProviderGenerate::chunkExists(int_t i, int_t j)
{
	return true;
}

void ChunkProviderGenerate::populate(IChunkProvider *ichunkprovider, int_t i, int_t j)
{
	BlockSand::fallInstantly = true;
	int_t k = i * 16;
	int_t l = j * 16;
	BiomeGenBase *biomegenbase = worldObj->getWorldChunkManager()->getBiomeGenAt(k + 16, l + 16);
	rand.setSeed(worldObj->getRandomSeed());
	long_t l1 = (rand.nextLong() / 2LL) * 2LL + 1LL;
	long_t l2 = (rand.nextLong() / 2LL) * 2LL + 1LL;
	rand.setSeed((long_t)i * l1 + (long_t)j * l2 ^ worldObj->getRandomSeed());

	if (rand.nextInt(4) == 0)
	{
		int_t i1 = k + rand.nextInt(16) + 8;
		int_t l4 = rand.nextInt(128);
		int_t i8 = l + rand.nextInt(16) + 8;
		WorldGenLakes(Block::waterStill->blockID).generate(worldObj, rand, i1, l4, i8);
	}
	if (rand.nextInt(8) == 0)
	{
		int_t j1 = k + rand.nextInt(16) + 8;
		int_t i5 = rand.nextInt(rand.nextInt(120) + 8);
		int_t j8 = l + rand.nextInt(16) + 8;
		if (i5 < 64 || rand.nextInt(10) == 0)
			WorldGenLakes(Block::lavaStill->blockID).generate(worldObj, rand, j1, i5, j8);
	}
	for (int_t k1 = 0; k1 < 8; k1++)
	{
		int_t j5 = k + rand.nextInt(16) + 8;
		int_t k8 = rand.nextInt(128);
		int_t j11 = l + rand.nextInt(16) + 8;
		WorldGenDungeons().generate(worldObj, rand, j5, k8, j11);
	}
	for (int_t i2 = 0; i2 < 10; i2++)
	{
		int_t k5 = k + rand.nextInt(16);
		int_t l8 = rand.nextInt(128);
		int_t k11 = l + rand.nextInt(16);
		WorldGenClay(32).generate(worldObj, rand, k5, l8, k11);
	}
	for (int_t j2 = 0; j2 < 20; j2++)
	{
		int_t l5 = k + rand.nextInt(16);
		int_t i9 = rand.nextInt(128);
		int_t l11 = l + rand.nextInt(16);
		WorldGenMinable(Block::dirt->blockID, 32).generate(worldObj, rand, l5, i9, l11);
	}
	for (int_t k2 = 0; k2 < 10; k2++)
	{
		int_t i6 = k + rand.nextInt(16);
		int_t j9 = rand.nextInt(128);
		int_t i12 = l + rand.nextInt(16);
		WorldGenMinable(Block::gravel->blockID, 32).generate(worldObj, rand, i6, j9, i12);
	}
	for (int_t i3 = 0; i3 < 20; i3++)
	{
		int_t j6 = k + rand.nextInt(16);
		int_t k9 = rand.nextInt(128);
		int_t j12 = l + rand.nextInt(16);
		WorldGenMinable(Block::oreCoal->blockID, 16).generate(worldObj, rand, j6, k9, j12);
	}
	for (int_t j3 = 0; j3 < 20; j3++)
	{
		int_t k6 = k + rand.nextInt(16);
		int_t l9 = rand.nextInt(64);
		int_t k12 = l + rand.nextInt(16);
		WorldGenMinable(Block::oreIron->blockID, 8).generate(worldObj, rand, k6, l9, k12);
	}
	for (int_t k3 = 0; k3 < 2; k3++)
	{
		int_t l6 = k + rand.nextInt(16);
		int_t i10 = rand.nextInt(32);
		int_t l12 = l + rand.nextInt(16);
		WorldGenMinable(Block::oreGold->blockID, 8).generate(worldObj, rand, l6, i10, l12);
	}
	for (int_t l3 = 0; l3 < 8; l3++)
	{
		int_t i7 = k + rand.nextInt(16);
		int_t j10 = rand.nextInt(16);
		int_t i13 = l + rand.nextInt(16);
		WorldGenMinable(Block::oreRedstone->blockID, 7).generate(worldObj, rand, i7, j10, i13);
	}
	for (int_t i4 = 0; i4 < 1; i4++)
	{
		int_t j7 = k + rand.nextInt(16);
		int_t k10 = rand.nextInt(16);
		int_t j13 = l + rand.nextInt(16);
		WorldGenMinable(Block::oreDiamond->blockID, 7).generate(worldObj, rand, j7, k10, j13);
	}
	for (int_t j4 = 0; j4 < 1; j4++)
	{
		int_t k7 = k + rand.nextInt(16);
		int_t l10 = rand.nextInt(16) + rand.nextInt(16);
		int_t k13 = l + rand.nextInt(16);
		WorldGenMinable(Block::oreLapis->blockID, 6).generate(worldObj, rand, k7, l10, k13);
	}

	double d = 0.5;
	int_t k4 = (int_t)((mobSpawnerNoise.getExperiencePoints((double)k * d, (double)l * d) / 8.0 + rand.nextDouble() * 4.0 + 4.0) / 3.0);
	int_t l7 = 0;
	if (rand.nextInt(10) == 0) l7++;
	if (biomegenbase == BiomeGenBase::forest)        l7 += k4 + 5;
	if (biomegenbase == BiomeGenBase::rainforest)    l7 += k4 + 5;
	if (biomegenbase == BiomeGenBase::seasonalForest)l7 += k4 + 2;
	if (biomegenbase == BiomeGenBase::taiga)         l7 += k4 + 5;
	if (biomegenbase == BiomeGenBase::desert)        l7 -= 20;
	if (biomegenbase == BiomeGenBase::tundra)        l7 -= 20;
	if (biomegenbase == BiomeGenBase::plains)        l7 -= 20;

	for (int_t i11 = 0; i11 < l7; i11++)
	{
		int_t l13 = k + rand.nextInt(16) + 8;
		int_t j14 = l + rand.nextInt(16) + 8;
		WorldGenerator *worldgenerator = biomegenbase->getRandomWorldGenForTrees(rand);
		worldgenerator->func_517_a(1.0, 1.0, 1.0);
		worldgenerator->generate(worldObj, rand, l13, worldObj->getHeightValue(l13, j14), j14);
		delete worldgenerator;
	}

	int_t byte0tmp = 0;
	if (biomegenbase == BiomeGenBase::forest)        byte0tmp = 2;
	if (biomegenbase == BiomeGenBase::seasonalForest)byte0tmp = 4;
	if (biomegenbase == BiomeGenBase::taiga)         byte0tmp = 2;
	if (biomegenbase == BiomeGenBase::plains)        byte0tmp = 3;
	for (int_t i14 = 0; i14 < byte0tmp; i14++)
	{
		int_t k14 = k + rand.nextInt(16) + 8;
		int_t l16 = rand.nextInt(128);
		int_t k19 = l + rand.nextInt(16) + 8;
		WorldGenFlowers(Block::plantYellow->blockID).generate(worldObj, rand, k14, l16, k19);
	}

	int_t byte1tmp = 0;
	if (biomegenbase == BiomeGenBase::forest)        byte1tmp = 2;
	if (biomegenbase == BiomeGenBase::rainforest)    byte1tmp = 10;
	if (biomegenbase == BiomeGenBase::seasonalForest)byte1tmp = 2;
	if (biomegenbase == BiomeGenBase::taiga)         byte1tmp = 1;
	if (biomegenbase == BiomeGenBase::plains)        byte1tmp = 10;
	for (int_t l14 = 0; l14 < byte1tmp; l14++)
	{
		int_t grassMeta = 1;
		if (biomegenbase == BiomeGenBase::rainforest && rand.nextInt(3) != 0)
			grassMeta = 2;
		int_t l19 = k + rand.nextInt(16) + 8;
		int_t k22 = rand.nextInt(128);
		int_t j24 = l + rand.nextInt(16) + 8;
		WorldGenTallGrass(Block::tallGrass->blockID, grassMeta).generate(worldObj, rand, l19, k22, j24);
	}

	int_t deadBushCount = 0;
	if (biomegenbase == BiomeGenBase::desert) deadBushCount = 2;
	for (int_t i15 = 0; i15 < deadBushCount; i15++)
	{
		int_t i17 = k + rand.nextInt(16) + 8;
		int_t i20 = rand.nextInt(128);
		int_t l22 = l + rand.nextInt(16) + 8;
		WorldGenDeadBush(Block::deadBush->blockID).generate(worldObj, rand, i17, i20, l22);
	}

	if (rand.nextInt(2) == 0)
	{
		int_t j15 = k + rand.nextInt(16) + 8;
		int_t j17 = rand.nextInt(128);
		int_t j20 = l + rand.nextInt(16) + 8;
		WorldGenFlowers(Block::plantRed->blockID).generate(worldObj, rand, j15, j17, j20);
	}
	if (rand.nextInt(4) == 0)
	{
		int_t k15 = k + rand.nextInt(16) + 8;
		int_t k17 = rand.nextInt(128);
		int_t k20 = l + rand.nextInt(16) + 8;
		WorldGenFlowers(Block::mushroomBrown->blockID).generate(worldObj, rand, k15, k17, k20);
	}
	if (rand.nextInt(8) == 0)
	{
		int_t l15 = k + rand.nextInt(16) + 8;
		int_t l17 = rand.nextInt(128);
		int_t l20 = l + rand.nextInt(16) + 8;
		WorldGenFlowers(Block::mushroomRed->blockID).generate(worldObj, rand, l15, l17, l20);
	}
	for (int_t i16 = 0; i16 < 10; i16++)
	{
		int_t i18 = k + rand.nextInt(16) + 8;
		int_t i21 = rand.nextInt(128);
		int_t i23 = l + rand.nextInt(16) + 8;
		WorldGenReed().generate(worldObj, rand, i18, i21, i23);
	}
	if (rand.nextInt(32) == 0)
	{
		int_t j16 = k + rand.nextInt(16) + 8;
		int_t j18 = rand.nextInt(128);
		int_t j21 = l + rand.nextInt(16) + 8;
		WorldGenPumpkin().generate(worldObj, rand, j16, j18, j21);
	}
	int_t k16 = 0;
	if (biomegenbase == BiomeGenBase::desert) k16 += 10;
	for (int_t k18 = 0; k18 < k16; k18++)
	{
		int_t k21 = k + rand.nextInt(16) + 8;
		int_t j23 = rand.nextInt(128);
		int_t k24 = l + rand.nextInt(16) + 8;
		WorldGenCactus().generate(worldObj, rand, k21, j23, k24);
	}
	for (int_t l18 = 0; l18 < 50; l18++)
	{
		int_t l21 = k + rand.nextInt(16) + 8;
		int_t k23 = rand.nextInt(rand.nextInt(120) + 8);
		int_t l24 = l + rand.nextInt(16) + 8;
		WorldGenLiquids(Block::waterMoving->blockID).generate(worldObj, rand, l21, k23, l24);
	}
	for (int_t i19 = 0; i19 < 20; i19++)
	{
		int_t i22 = k + rand.nextInt(16) + 8;
		int_t l23 = rand.nextInt(rand.nextInt(rand.nextInt(112) + 8) + 8);
		int_t i25 = l + rand.nextInt(16) + 8;
		WorldGenLiquids(Block::lavaMoving->blockID).generate(worldObj, rand, i22, l23, i25);
	}

	generatedTemperatures = worldObj->getWorldChunkManager()->getTemperatures(generatedTemperatures, k + 8, l + 8, 16, 16);
	for (int_t j19 = k + 8; j19 < k + 8 + 16; j19++)
	{
		for (int_t j22 = l + 8; j22 < l + 8 + 16; j22++)
		{
			int_t i24 = j19 - (k + 8);
			int_t j25 = j22 - (l + 8);
			int_t k25 = worldObj->findTopSolidBlock(j19, j22);
			double d1 = generatedTemperatures[i24 * 16 + j25] - ((double)(k25 - 64) / 64.0) * 0.3;
			if (d1 < 0.5 && k25 > 0 && k25 < 128
				&& worldObj->isAirBlock(j19, k25, j22)
				&& worldObj->getBlockMaterial(j19, k25 - 1, j22)->getIsSolid()
				&& worldObj->getBlockMaterial(j19, k25 - 1, j22) != Material::ice)
			{
				worldObj->setBlockWithNotify(j19, k25, j22, Block::snow->blockID);
			}
		}
	}

	BlockSand::fallInstantly = false;
}

bool ChunkProviderGenerate::saveChunks(bool flag, IProgressUpdate *iprogressupdate)
{
	return true;
}

bool ChunkProviderGenerate::unload100OldestChunks()
{
	return false;
}

bool ChunkProviderGenerate::canSave()
{
	return true;
}

jstring ChunkProviderGenerate::makeString()
{
	return u"RandomLevelSource";
}
