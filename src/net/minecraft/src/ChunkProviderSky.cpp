#include "ChunkProviderSky.h"

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
#include "WorldGenReed.h"
#include "WorldGenPumpkin.h"
#include "WorldGenCactus.h"
#include "WorldGenLiquids.h"
#include "BlockSand.h"
#include "BlockFlower.h"

ChunkProviderSky::ChunkProviderSky(World *world, long_t l)
	: field_28081_p(world)
	, field_28087_j(l)
	, field_28086_k(field_28087_j, 16)
	, field_28085_l(field_28087_j, 16)
	, field_28084_m(field_28087_j, 8)
	, field_28083_n(field_28087_j, 4)
	, field_28082_o(field_28087_j, 4)
	, field_28096_a(field_28087_j, 10)
	, field_28095_b(field_28087_j, 16)
	, field_28094_c(field_28087_j, 8)
	, field_28075_v()
{
	field_28079_r.resize(256);
	field_28078_s.resize(256);
	field_28077_t.resize(256);
	caveGenerator = new MapGenCaves();
	std::memset(field_28088_i, 0, sizeof(field_28088_i));
}

ChunkProviderSky::~ChunkProviderSky()
{
	delete caveGenerator;
}

void ChunkProviderSky::generateSkyTerrain(int_t i, int_t j, byte_t *abyte0,
	BiomeGenBase **abiomegenbase, double *ad)
{
	const int_t byte0 = 2;
	int_t k = byte0 + 1;
	const int_t byte1 = 33;
	int_t l = byte0 + 1;
	field_28080_q = generateNoiseField(field_28080_q, i * byte0, 0, j * byte0, k, byte1, l);

	for (int_t i1 = 0; i1 < byte0; i1++)
	{
		for (int_t j1 = 0; j1 < byte0; j1++)
		{
			for (int_t k1 = 0; k1 < 32; k1++)
			{
				double d = 0.25;
				double d1 = field_28080_q[((i1 + 0) * l + (j1 + 0)) * byte1 + (k1 + 0)];
				double d2 = field_28080_q[((i1 + 0) * l + (j1 + 1)) * byte1 + (k1 + 0)];
				double d3 = field_28080_q[((i1 + 1) * l + (j1 + 0)) * byte1 + (k1 + 0)];
				double d4 = field_28080_q[((i1 + 1) * l + (j1 + 1)) * byte1 + (k1 + 0)];
				double d5 = (field_28080_q[((i1 + 0) * l + (j1 + 0)) * byte1 + (k1 + 1)] - d1) * d;
				double d6 = (field_28080_q[((i1 + 0) * l + (j1 + 1)) * byte1 + (k1 + 1)] - d2) * d;
				double d7 = (field_28080_q[((i1 + 1) * l + (j1 + 0)) * byte1 + (k1 + 1)] - d3) * d;
				double d8 = (field_28080_q[((i1 + 1) * l + (j1 + 1)) * byte1 + (k1 + 1)] - d4) * d;

				for (int_t l1 = 0; l1 < 4; l1++)
				{
					double d9  = 0.125;
					double d10 = d1;
					double d11 = d2;
					double d12 = (d3 - d1) * d9;
					double d13 = (d4 - d2) * d9;

					for (int_t i2 = 0; i2 < 8; i2++)
					{
						int_t j2 = (i2 + i1 * 8) << 11 | (0 + j1 * 8) << 7 | (k1 * 4 + l1);
						const int_t stride = 128;
						double d14 = 0.125;
						double d15 = d10;
						double d16 = (d11 - d10) * d14;

						for (int_t k2 = 0; k2 < 8; k2++)
						{
							int_t l2 = 0;
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

void ChunkProviderSky::replaceSkyBlocks(int_t i, int_t j, byte_t *abyte0,
	BiomeGenBase **abiomegenbase)
{
	double d = 0.03125;
	field_28079_r = field_28083_n.generateNoiseOctaves(field_28079_r, (double)(i * 16), (double)(j * 16), 0.0, 16, 16, 1, d, d, 1.0);
	field_28078_s = field_28083_n.generateNoiseOctaves(field_28078_s, (double)(i * 16), 109.0134, (double)(j * 16), 16, 1, 16, d, 1.0, d);
	field_28077_t = field_28082_o.generateNoiseOctaves(field_28077_t, (double)(i * 16), (double)(j * 16), 0.0, 16, 16, 1, d * 2.0, d * 2.0, d * 2.0);

	for (int_t k = 0; k < 16; k++)
	{
		for (int_t l = 0; l < 16; l++)
		{
			BiomeGenBase *biomegenbase = abiomegenbase[k + l * 16];
			int_t i1 = (int_t)(field_28077_t[k + l * 16] / 3.0 + 3.0 + field_28087_j.nextDouble() * 0.25);
			int_t j1 = -1;
			byte_t byte0 = biomegenbase->topBlock;
			byte_t byte1 = biomegenbase->fillerBlock;

			for (int_t k1 = 127; k1 >= 0; k1--)
			{
				int_t l1 = (l * 16 + k) * 128 + k1;
				byte_t byte2 = abyte0[l1];
				if (byte2 == 0) { j1 = -1; continue; }
				if (byte2 != (byte_t)Block::stone->blockID) continue;

				if (j1 == -1)
				{
					if (i1 <= 0)
					{
						byte0 = 0;
						byte1 = (byte_t)Block::stone->blockID;
					}
					j1 = i1;
					if (k1 >= 0)
						abyte0[l1] = byte0;
					else
						abyte0[l1] = byte1;
					continue;
				}
				if (j1 <= 0) continue;
				j1--;
				abyte0[l1] = byte1;
				if (j1 == 0 && byte1 == (byte_t)Block::sand->blockID)
				{
					j1 = field_28087_j.nextInt(4);
					byte1 = (byte_t)Block::sandStone->blockID;
				}
			}
		}
	}
}

Chunk *ChunkProviderSky::prepareChunk(int_t i, int_t j)
{
	return provideChunk(i, j);
}

Chunk *ChunkProviderSky::provideChunk(int_t i, int_t j)
{
	field_28087_j.setSeed((long_t)i * 0x4f9939f508LL + (long_t)j * 0x1ef1565bd5LL);
	std::vector<byte_t> abyte0(32768, 0);
	field_28075_v = field_28081_p->getWorldChunkManager()->loadBlockGeneratorData(field_28075_v, i * 16, j * 16, 16, 16);
	double *ad = field_28081_p->getWorldChunkManager()->temperature.data();
	generateSkyTerrain(i, j, abyte0.data(), field_28075_v.data(), ad);
	replaceSkyBlocks(i, j, abyte0.data(), field_28075_v.data());
	caveGenerator->generate(this, field_28081_p, i, j, abyte0.data());
	Chunk *chunk = new Chunk(field_28081_p, abyte0, i, j);
	chunk->generateSkylightMap();
	return chunk;
}

std::vector<double> &ChunkProviderSky::generateNoiseField(std::vector<double> &ad,
	int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1)
{
	if (ad.empty())
		ad.resize((size_t)(l * i1 * j1));

	double d  = 684.412;
	const double d1 = 684.412;
	double *ad1 = field_28081_p->getWorldChunkManager()->temperature.data();
	double *ad2 = field_28081_p->getWorldChunkManager()->humidity.data();

	field_28090_g = field_28096_a.getBiomeGenForCoords(field_28090_g, i, k, l, j1, 1.121, 1.121, 0.5);
	field_28089_h = field_28095_b.getBiomeGenForCoords(field_28089_h, i, k, l, j1, 200.0, 200.0, 0.5);
	d *= 2.0;
	field_28093_d = field_28084_m.generateNoiseOctaves(field_28093_d, (double)i, (double)j, (double)k, l, i1, j1, d / 80.0, d1 / 160.0, d / 80.0);
	field_28092_e = field_28086_k.generateNoiseOctaves(field_28092_e, (double)i, (double)j, (double)k, l, i1, j1, d, d1, d);
	field_28091_f = field_28085_l.generateNoiseOctaves(field_28091_f, (double)i, (double)j, (double)k, l, i1, j1, d, d1, d);

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
			double d5 = (field_28090_g[l1] + 256.0) / 512.0;
			d5 *= d4;
			if (d5 > 1.0) d5 = 1.0;
			double d6 = field_28089_h[l1] / 8000.0;
			if (d6 < 0.0) d6 = -d6 * 0.3;
			d6 = d6 * 3.0 - 2.0;
			if (d6 > 1.0) d6 = 1.0;
			d6 /= 8.0;
			d6 = 0.0;  // Java: d6 = 0.0 (overwrite, matches original)
			if (d5 < 0.0) d5 = 0.0;
			d5 += 0.5;
			d6 = (d6 * (double)i1) / 16.0;
			l1++;
			double d7 = (double)i1 / 2.0;

			for (int_t j3 = 0; j3 < i1; j3++)
			{
				double d8 = 0.0;
				double d9 = ((double)j3 - d7) * 8.0 / d5;
				if (d9 < 0.0) d9 *= -1.0;
				double d10 = field_28092_e[k1] / 512.0;
				double d11 = field_28091_f[k1] / 512.0;
				double d12 = (field_28093_d[k1] / 10.0 + 1.0) / 2.0;
				if (d12 < 0.0)       d8 = d10;
				else if (d12 > 1.0)  d8 = d11;
				else                  d8 = d10 + (d11 - d10) * d12;
				d8 -= 8.0;
				const int_t k3_top = 32;
				if (j3 > i1 - k3_top)
				{
					double d13 = (float)(j3 - (i1 - k3_top)) / ((float)k3_top - 1.0f);
					d8 = d8 * (1.0 - d13) + -30.0 * d13;
				}
				const int_t k3_bot = 8;
				if (j3 < k3_bot)
				{
					double d14 = (float)(k3_bot - j3) / ((float)k3_bot - 1.0f);
					d8 = d8 * (1.0 - d14) + -30.0 * d14;
				}
				ad[k1] = d8;
				k1++;
			}
		}
	}
	return ad;
}

bool ChunkProviderSky::chunkExists(int_t i, int_t j)
{
	return true;
}

void ChunkProviderSky::populate(IChunkProvider *ichunkprovider, int_t i, int_t j)
{
	BlockSand::fallInstantly = true;
	int_t k = i * 16;
	int_t l = j * 16;
	BiomeGenBase *biomegenbase = field_28081_p->getWorldChunkManager()->getBiomeGenAt(k + 16, l + 16);
	field_28087_j.setSeed(field_28081_p->getRandomSeed());
	long_t l1 = (field_28087_j.nextLong() / 2LL) * 2LL + 1LL;
	long_t l2 = (field_28087_j.nextLong() / 2LL) * 2LL + 1LL;
	field_28087_j.setSeed((long_t)i * l1 + (long_t)j * l2 ^ field_28081_p->getRandomSeed());

	if (field_28087_j.nextInt(4) == 0)
	{
		int_t i1 = k + field_28087_j.nextInt(16) + 8;
		int_t l4 = field_28087_j.nextInt(128);
		int_t i8 = l + field_28087_j.nextInt(16) + 8;
		WorldGenLakes(Block::waterStill->blockID).generate(field_28081_p, field_28087_j, i1, l4, i8);
	}
	if (field_28087_j.nextInt(8) == 0)
	{
		int_t j1 = k + field_28087_j.nextInt(16) + 8;
		int_t i5 = field_28087_j.nextInt(field_28087_j.nextInt(120) + 8);
		int_t j8 = l + field_28087_j.nextInt(16) + 8;
		if (i5 < 64 || field_28087_j.nextInt(10) == 0)
			WorldGenLakes(Block::lavaStill->blockID).generate(field_28081_p, field_28087_j, j1, i5, j8);
	}
	for (int_t k1 = 0; k1 < 8; k1++)
	{
		int_t j5 = k + field_28087_j.nextInt(16) + 8;
		int_t k8 = field_28087_j.nextInt(128);
		int_t i13 = l + field_28087_j.nextInt(16) + 8;
		WorldGenDungeons().generate(field_28081_p, field_28087_j, j5, k8, i13);
	}
	for (int_t i2 = 0; i2 < 10; i2++)
	{
		int_t k5 = k + field_28087_j.nextInt(16);
		int_t l8 = field_28087_j.nextInt(128);
		int_t j13 = l + field_28087_j.nextInt(16);
		WorldGenClay(32).generate(field_28081_p, field_28087_j, k5, l8, j13);
	}
	for (int_t j2 = 0; j2 < 20; j2++)
	{
		int_t l5 = k + field_28087_j.nextInt(16);
		int_t i9 = field_28087_j.nextInt(128);
		int_t k13 = l + field_28087_j.nextInt(16);
		WorldGenMinable(Block::dirt->blockID, 32).generate(field_28081_p, field_28087_j, l5, i9, k13);
	}
	for (int_t k2 = 0; k2 < 10; k2++)
	{
		int_t i6 = k + field_28087_j.nextInt(16);
		int_t j9 = field_28087_j.nextInt(128);
		int_t l13 = l + field_28087_j.nextInt(16);
		WorldGenMinable(Block::gravel->blockID, 32).generate(field_28081_p, field_28087_j, i6, j9, l13);
	}
	for (int_t i3 = 0; i3 < 20; i3++)
	{
		int_t j6 = k + field_28087_j.nextInt(16);
		int_t k9 = field_28087_j.nextInt(128);
		int_t i14 = l + field_28087_j.nextInt(16);
		WorldGenMinable(Block::oreCoal->blockID, 16).generate(field_28081_p, field_28087_j, j6, k9, i14);
	}
	for (int_t j3 = 0; j3 < 20; j3++)
	{
		int_t k6 = k + field_28087_j.nextInt(16);
		int_t l9 = field_28087_j.nextInt(64);
		int_t j14 = l + field_28087_j.nextInt(16);
		WorldGenMinable(Block::oreIron->blockID, 8).generate(field_28081_p, field_28087_j, k6, l9, j14);
	}
	for (int_t k3 = 0; k3 < 2; k3++)
	{
		int_t l6 = k + field_28087_j.nextInt(16);
		int_t i10 = field_28087_j.nextInt(32);
		int_t k14 = l + field_28087_j.nextInt(16);
		WorldGenMinable(Block::oreGold->blockID, 8).generate(field_28081_p, field_28087_j, l6, i10, k14);
	}
	for (int_t l3 = 0; l3 < 8; l3++)
	{
		int_t i7 = k + field_28087_j.nextInt(16);
		int_t j10 = field_28087_j.nextInt(16);
		int_t l14 = l + field_28087_j.nextInt(16);
		WorldGenMinable(Block::oreRedstone->blockID, 7).generate(field_28081_p, field_28087_j, i7, j10, l14);
	}
	for (int_t i4 = 0; i4 < 1; i4++)
	{
		int_t j7 = k + field_28087_j.nextInt(16);
		int_t k10 = field_28087_j.nextInt(16);
		int_t i15 = l + field_28087_j.nextInt(16);
		WorldGenMinable(Block::oreDiamond->blockID, 7).generate(field_28081_p, field_28087_j, j7, k10, i15);
	}
	for (int_t j4 = 0; j4 < 1; j4++)
	{
		int_t k7 = k + field_28087_j.nextInt(16);
		int_t l10 = field_28087_j.nextInt(16) + field_28087_j.nextInt(16);
		int_t j15 = l + field_28087_j.nextInt(16);
		WorldGenMinable(Block::oreLapis->blockID, 6).generate(field_28081_p, field_28087_j, k7, l10, j15);
	}

	double d = 0.5;
	int_t k4 = (int_t)((field_28094_c.getExperiencePoints((double)k * d, (double)l * d) / 8.0 + field_28087_j.nextDouble() * 4.0 + 4.0) / 3.0);
	int_t l7 = 0;
	if (field_28087_j.nextInt(10) == 0) l7++;
	if (biomegenbase == BiomeGenBase::forest)        l7 += k4 + 5;
	if (biomegenbase == BiomeGenBase::rainforest)    l7 += k4 + 5;
	if (biomegenbase == BiomeGenBase::seasonalForest)l7 += k4 + 2;
	if (biomegenbase == BiomeGenBase::taiga)         l7 += k4 + 5;
	if (biomegenbase == BiomeGenBase::desert)        l7 -= 20;
	if (biomegenbase == BiomeGenBase::tundra)        l7 -= 20;
	if (biomegenbase == BiomeGenBase::plains)        l7 -= 20;

	for (int_t i11 = 0; i11 < l7; i11++)
	{
		int_t k15 = k + field_28087_j.nextInt(16) + 8;
		int_t j18 = l + field_28087_j.nextInt(16) + 8;
		WorldGenerator *worldgenerator = biomegenbase->getRandomWorldGenForTrees(field_28087_j);
		worldgenerator->func_517_a(1.0, 1.0, 1.0);
		worldgenerator->generate(field_28081_p, field_28087_j, k15, field_28081_p->getHeightValue(k15, j18), j18);
		delete worldgenerator;
	}

	for (int_t j11 = 0; j11 < 2; j11++)
	{
		int_t l15 = k + field_28087_j.nextInt(16) + 8;
		int_t k18 = field_28087_j.nextInt(128);
		int_t i21 = l + field_28087_j.nextInt(16) + 8;
		WorldGenFlowers(Block::plantYellow->blockID).generate(field_28081_p, field_28087_j, l15, k18, i21);
	}
	if (field_28087_j.nextInt(2) == 0)
	{
		int_t k11 = k + field_28087_j.nextInt(16) + 8;
		int_t i16 = field_28087_j.nextInt(128);
		int_t l18 = l + field_28087_j.nextInt(16) + 8;
		WorldGenFlowers(Block::plantRed->blockID).generate(field_28081_p, field_28087_j, k11, i16, l18);
	}
	if (field_28087_j.nextInt(4) == 0)
	{
		int_t l11 = k + field_28087_j.nextInt(16) + 8;
		int_t j16 = field_28087_j.nextInt(128);
		int_t i19 = l + field_28087_j.nextInt(16) + 8;
		WorldGenFlowers(Block::mushroomBrown->blockID).generate(field_28081_p, field_28087_j, l11, j16, i19);
	}
	if (field_28087_j.nextInt(8) == 0)
	{
		int_t i12 = k + field_28087_j.nextInt(16) + 8;
		int_t k16 = field_28087_j.nextInt(128);
		int_t j19 = l + field_28087_j.nextInt(16) + 8;
		WorldGenFlowers(Block::mushroomRed->blockID).generate(field_28081_p, field_28087_j, i12, k16, j19);
	}
	for (int_t j12 = 0; j12 < 10; j12++)
	{
		int_t l16 = k + field_28087_j.nextInt(16) + 8;
		int_t k19 = field_28087_j.nextInt(128);
		int_t j21 = l + field_28087_j.nextInt(16) + 8;
		WorldGenReed().generate(field_28081_p, field_28087_j, l16, k19, j21);
	}
	if (field_28087_j.nextInt(32) == 0)
	{
		int_t k12 = k + field_28087_j.nextInt(16) + 8;
		int_t i17 = field_28087_j.nextInt(128);
		int_t l19 = l + field_28087_j.nextInt(16) + 8;
		WorldGenPumpkin().generate(field_28081_p, field_28087_j, k12, i17, l19);
	}
	int_t l12 = 0;
	if (biomegenbase == BiomeGenBase::desert) l12 += 10;
	for (int_t j17 = 0; j17 < l12; j17++)
	{
		int_t i20 = k + field_28087_j.nextInt(16) + 8;
		int_t k21 = field_28087_j.nextInt(128);
		int_t k22 = l + field_28087_j.nextInt(16) + 8;
		WorldGenCactus().generate(field_28081_p, field_28087_j, i20, k21, k22);
	}
	for (int_t k17 = 0; k17 < 50; k17++)
	{
		int_t j20 = k + field_28087_j.nextInt(16) + 8;
		int_t l21 = field_28087_j.nextInt(field_28087_j.nextInt(120) + 8);
		int_t l22 = l + field_28087_j.nextInt(16) + 8;
		WorldGenLiquids(Block::waterMoving->blockID).generate(field_28081_p, field_28087_j, j20, l21, l22);
	}
	for (int_t l17 = 0; l17 < 20; l17++)
	{
		int_t k20 = k + field_28087_j.nextInt(16) + 8;
		int_t i22 = field_28087_j.nextInt(field_28087_j.nextInt(field_28087_j.nextInt(112) + 8) + 8);
		int_t i23 = l + field_28087_j.nextInt(16) + 8;
		WorldGenLiquids(Block::lavaMoving->blockID).generate(field_28081_p, field_28087_j, k20, i22, i23);
	}

	field_28074_w = field_28081_p->getWorldChunkManager()->getTemperatures(field_28074_w, k + 8, l + 8, 16, 16);
	for (int_t i18 = k + 8; i18 < k + 8 + 16; i18++)
	{
		for (int_t l20 = l + 8; l20 < l + 8 + 16; l20++)
		{
			int_t j22 = i18 - (k + 8);
			int_t j23 = l20 - (l + 8);
			int_t k23 = field_28081_p->findTopSolidBlock(i18, l20);
			double d1 = field_28074_w[j22 * 16 + j23] - ((double)(k23 - 64) / 64.0) * 0.3;
			if (d1 < 0.5 && k23 > 0 && k23 < 128
				&& field_28081_p->isAirBlock(i18, k23, l20)
				&& field_28081_p->getBlockMaterial(i18, k23 - 1, l20)->getIsSolid()
				&& field_28081_p->getBlockMaterial(i18, k23 - 1, l20) != Material::ice)
			{
				field_28081_p->setBlockWithNotify(i18, k23, l20, Block::snow->blockID);
			}
		}
	}

	BlockSand::fallInstantly = false;
}

bool ChunkProviderSky::saveChunks(bool flag, IProgressUpdate *iprogressupdate)
{
	return true;
}

bool ChunkProviderSky::unload100OldestChunks()
{
	return false;
}

bool ChunkProviderSky::canSave()
{
	return true;
}

jstring ChunkProviderSky::makeString()
{
	return u"RandomLevelSource";
}
