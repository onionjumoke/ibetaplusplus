#include "ChunkProviderHell.h"

#include <cmath>

#include "World.h"
#include "Chunk.h"
#include "Block.h"
#include "MapGenCavesHell.h"
#include "WorldGenHellLava.h"
#include "WorldGenFire.h"
#include "WorldGenGlowStone1.h"
#include "WorldGenGlowStone2.h"
#include "WorldGenFlowers.h"
#include "BlockSand.h"
#include "BlockFlower.h"

ChunkProviderHell::ChunkProviderHell(World *world, long_t l)
	: worldObj(world)
	, hellRNG(l)
	, field_4169_i(hellRNG, 16)
	, field_4168_j(hellRNG, 16)
	, field_4167_k(hellRNG, 8)
	, field_4166_l(hellRNG, 4)
	, field_4165_m(hellRNG, 4)
	, field_4177_a(hellRNG, 10)
	, field_4176_b(hellRNG, 16)
{
	field_4162_p.resize(256);
	field_4161_q.resize(256);
	field_4160_r.resize(256);
	caveGenerator = new MapGenCavesHell();
}

ChunkProviderHell::~ChunkProviderHell()
{
	delete caveGenerator;
}

void ChunkProviderHell::generateHellTerrain(int_t i, int_t j, byte_t *abyte0)
{
	const int_t byte0 = 4;
	const int_t byte1 = 32;
	int_t k = byte0 + 1;
	const int_t byte2 = 17;
	int_t l = byte0 + 1;
	field_4163_o = generateNoiseField(field_4163_o, i * byte0, 0, j * byte0, k, byte2, l);

	for (int_t i1 = 0; i1 < byte0; i1++)
	{
		for (int_t j1 = 0; j1 < byte0; j1++)
		{
			for (int_t k1 = 0; k1 < 16; k1++)
			{
				double d = 0.125;
				double d1 = field_4163_o[((i1 + 0) * l + (j1 + 0)) * byte2 + (k1 + 0)];
				double d2 = field_4163_o[((i1 + 0) * l + (j1 + 1)) * byte2 + (k1 + 0)];
				double d3 = field_4163_o[((i1 + 1) * l + (j1 + 0)) * byte2 + (k1 + 0)];
				double d4 = field_4163_o[((i1 + 1) * l + (j1 + 1)) * byte2 + (k1 + 0)];
				double d5 = (field_4163_o[((i1 + 0) * l + (j1 + 0)) * byte2 + (k1 + 1)] - d1) * d;
				double d6 = (field_4163_o[((i1 + 0) * l + (j1 + 1)) * byte2 + (k1 + 1)] - d2) * d;
				double d7 = (field_4163_o[((i1 + 1) * l + (j1 + 0)) * byte2 + (k1 + 1)] - d3) * d;
				double d8 = (field_4163_o[((i1 + 1) * l + (j1 + 1)) * byte2 + (k1 + 1)] - d4) * d;

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
						const int_t stride = 128;
						double d14 = 0.25;
						double d15 = d10;
						double d16 = (d11 - d10) * d14;

						for (int_t k2 = 0; k2 < 4; k2++)
						{
							int_t l2 = 0;
							if (k1 * 8 + l1 < byte1)
								l2 = Block::lavaStill->blockID;
							if (d15 > 0.0)
								l2 = Block::netherrack->blockID;
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

void ChunkProviderHell::replaceHellBlocks(int_t i, int_t j, byte_t *abyte0)
{
	const int_t byte0 = 64;
	double d = 0.03125;
	field_4162_p = field_4166_l.generateNoiseOctaves(field_4162_p, (double)(i * 16), (double)(j * 16), 0.0, 16, 16, 1, d, d, 1.0);
	field_4161_q = field_4166_l.generateNoiseOctaves(field_4161_q, (double)(i * 16), 109.0134, (double)(j * 16), 16, 1, 16, d, 1.0, d);
	field_4160_r = field_4165_m.generateNoiseOctaves(field_4160_r, (double)(i * 16), (double)(j * 16), 0.0, 16, 16, 1, d * 2.0, d * 2.0, d * 2.0);

	for (int_t k = 0; k < 16; k++)
	{
		for (int_t l = 0; l < 16; l++)
		{
			bool flag  = field_4162_p[k + l * 16] + hellRNG.nextDouble() * 0.2 > 0.0;
			bool flag1 = field_4161_q[k + l * 16] + hellRNG.nextDouble() * 0.2 > 0.0;
			int_t i1 = (int_t)(field_4160_r[k + l * 16] / 3.0 + 3.0 + hellRNG.nextDouble() * 0.25);
			int_t j1 = -1;
			byte_t byte1 = (byte_t)Block::netherrack->blockID;
			byte_t byte2 = (byte_t)Block::netherrack->blockID;

			for (int_t k1 = 127; k1 >= 0; k1--)
			{
				int_t l1 = (l * 16 + k) * 128 + k1;
				if (k1 >= 127 - hellRNG.nextInt(5))
				{
					abyte0[l1] = (byte_t)Block::bedrock->blockID;
					continue;
				}
				if (k1 <= 0 + hellRNG.nextInt(5))
				{
					abyte0[l1] = (byte_t)Block::bedrock->blockID;
					continue;
				}
				byte_t byte3 = abyte0[l1];
				if (byte3 == 0) { j1 = -1; continue; }
				if (byte3 != (byte_t)Block::netherrack->blockID) continue;

				if (j1 == -1)
				{
					if (i1 <= 0)
					{
						byte1 = 0;
						byte2 = (byte_t)Block::netherrack->blockID;
					}
					else if (k1 >= byte0 - 4 && k1 <= byte0 + 1)
					{
						byte1 = (byte_t)Block::netherrack->blockID;
						byte2 = (byte_t)Block::netherrack->blockID;
						if (flag1) { byte1 = (byte_t)Block::gravel->blockID; }
						if (flag1) { byte2 = (byte_t)Block::netherrack->blockID; }
						if (flag)  { byte1 = (byte_t)Block::slowSand->blockID; }
						if (flag)  { byte2 = (byte_t)Block::slowSand->blockID; }
					}
					if (k1 < byte0 && byte1 == 0)
						byte1 = (byte_t)Block::lavaStill->blockID;
					j1 = i1;
					if (k1 >= byte0 - 1)
						abyte0[l1] = byte1;
					else
						abyte0[l1] = byte2;
					continue;
				}
				if (j1 > 0)
				{
					j1--;
					abyte0[l1] = byte2;
				}
			}
		}
	}
}

Chunk *ChunkProviderHell::prepareChunk(int_t i, int_t j)
{
	return provideChunk(i, j);
}

Chunk *ChunkProviderHell::provideChunk(int_t i, int_t j)
{
	hellRNG.setSeed((long_t)i * 0x4f9939f508LL + (long_t)j * 0x1ef1565bd5LL);
	std::vector<byte_t> abyte0(32768, 0);
	generateHellTerrain(i, j, abyte0.data());
	replaceHellBlocks(i, j, abyte0.data());
	caveGenerator->generate(this, worldObj, i, j, abyte0.data());
	Chunk *chunk = new Chunk(worldObj, abyte0, i, j);
	return chunk;
}

std::vector<double> &ChunkProviderHell::generateNoiseField(std::vector<double> &ad,
	int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1)
{
	if (ad.empty())
		ad.resize((size_t)(l * i1 * j1));

	const double d  = 684.412;
	const double d1 = 2053.236;

	field_4172_f = field_4177_a.generateNoiseOctaves(field_4172_f, (double)i, (double)j, (double)k, l, 1, j1, 1.0, 0.0, 1.0);
	field_4171_g = field_4176_b.generateNoiseOctaves(field_4171_g, (double)i, (double)j, (double)k, l, 1, j1, 100.0, 0.0, 100.0);
	field_4175_c = field_4167_k.generateNoiseOctaves(field_4175_c, (double)i, (double)j, (double)k, l, i1, j1, d / 80.0, d1 / 60.0, d / 80.0);
	field_4174_d = field_4169_i.generateNoiseOctaves(field_4174_d, (double)i, (double)j, (double)k, l, i1, j1, d, d1, d);
	field_4173_e = field_4168_j.generateNoiseOctaves(field_4173_e, (double)i, (double)j, (double)k, l, i1, j1, d, d1, d);

	int_t k1 = 0;
	int_t l1 = 0;

	std::vector<double> ad1(i1);
	for (int_t i2 = 0; i2 < i1; i2++)
	{
		ad1[i2] = std::cos(((double)i2 * 3.14159265358979 * 6.0) / (double)i1) * 2.0;
		double d2 = (double)i2;
		if (i2 > i1 / 2) d2 = i1 - 1 - i2;
		if (d2 < 4.0)
		{
			d2 = 4.0 - d2;
			ad1[i2] -= d2 * d2 * d2 * 10.0;
		}
	}

	for (int_t j2 = 0; j2 < l; j2++)
	{
		for (int_t k2 = 0; k2 < j1; k2++)
		{
			double d3 = (field_4172_f[l1] + 256.0) / 512.0;
			if (d3 > 1.0) d3 = 1.0;
			double d4 = 0.0;
			double d5 = field_4171_g[l1] / 8000.0;
			if (d5 < 0.0) d5 = -d5;
			d5 = d5 * 3.0 - 3.0;
			if (d5 < 0.0)
			{
				d5 /= 2.0;
				if (d5 < -1.0) d5 = -1.0;
				d5 /= 1.4;
				d5 /= 2.0;
				d3 = 0.0;
			}
			else
			{
				if (d5 > 1.0) d5 = 1.0;
				d5 /= 6.0;
			}
			d3 += 0.5;
			d5 = (d5 * (double)i1) / 16.0;
			l1++;

			for (int_t l2 = 0; l2 < i1; l2++)
			{
				double d6 = 0.0;
				double d7 = ad1[l2];
				double d8 = field_4174_d[k1] / 512.0;
				double d9 = field_4173_e[k1] / 512.0;
				double d10 = (field_4175_c[k1] / 10.0 + 1.0) / 2.0;
				if (d10 < 0.0)       d6 = d8;
				else if (d10 > 1.0)  d6 = d9;
				else                  d6 = d8 + (d9 - d8) * d10;
				d6 -= d7;
				if (l2 > i1 - 4)
				{
					double d11 = (float)(l2 - (i1 - 4)) / 3.0f;
					d6 = d6 * (1.0 - d11) + -10.0 * d11;
				}
				if ((double)l2 < d4)
				{
					double d12 = (d4 - (double)l2) / 4.0;
					if (d12 < 0.0) d12 = 0.0;
					if (d12 > 1.0) d12 = 1.0;
					d6 = d6 * (1.0 - d12) + -10.0 * d12;
				}
				ad[k1] = d6;
				k1++;
			}
		}
	}
	return ad;
}

bool ChunkProviderHell::chunkExists(int_t i, int_t j)
{
	return true;
}

void ChunkProviderHell::populate(IChunkProvider *ichunkprovider, int_t i, int_t j)
{
	BlockSand::fallInstantly = true;
	int_t k = i * 16;
	int_t l = j * 16;

	for (int_t i1 = 0; i1 < 8; i1++)
	{
		int_t k1 = k + hellRNG.nextInt(16) + 8;
		int_t i3 = hellRNG.nextInt(120) + 4;
		int_t k4 = l + hellRNG.nextInt(16) + 8;
		WorldGenHellLava(Block::lavaMoving->blockID).generate(worldObj, hellRNG, k1, i3, k4);
	}

	int_t j1 = hellRNG.nextInt(hellRNG.nextInt(10) + 1) + 1;
	for (int_t l1 = 0; l1 < j1; l1++)
	{
		int_t j3 = k + hellRNG.nextInt(16) + 8;
		int_t l4 = hellRNG.nextInt(120) + 4;
		int_t i6 = l + hellRNG.nextInt(16) + 8;
		WorldGenFire().generate(worldObj, hellRNG, j3, l4, i6);
	}

	j1 = hellRNG.nextInt(hellRNG.nextInt(10) + 1);
	for (int_t i2 = 0; i2 < j1; i2++)
	{
		int_t k3 = k + hellRNG.nextInt(16) + 8;
		int_t i5 = hellRNG.nextInt(120) + 4;
		int_t j6 = l + hellRNG.nextInt(16) + 8;
		WorldGenGlowStone1().generate(worldObj, hellRNG, k3, i5, j6);
	}

	for (int_t j2 = 0; j2 < 10; j2++)
	{
		int_t l3 = k + hellRNG.nextInt(16) + 8;
		int_t j5 = hellRNG.nextInt(128);
		int_t k6 = l + hellRNG.nextInt(16) + 8;
		WorldGenGlowStone2().generate(worldObj, hellRNG, l3, j5, k6);
	}

	if (hellRNG.nextInt(1) == 0)
	{
		int_t k2 = k + hellRNG.nextInt(16) + 8;
		int_t i4 = hellRNG.nextInt(128);
		int_t k5 = l + hellRNG.nextInt(16) + 8;
		WorldGenFlowers(Block::mushroomBrown->blockID).generate(worldObj, hellRNG, k2, i4, k5);
	}
	if (hellRNG.nextInt(1) == 0)
	{
		int_t l2 = k + hellRNG.nextInt(16) + 8;
		int_t j4 = hellRNG.nextInt(128);
		int_t l5 = l + hellRNG.nextInt(16) + 8;
		WorldGenFlowers(Block::mushroomRed->blockID).generate(worldObj, hellRNG, l2, j4, l5);
	}

	BlockSand::fallInstantly = false;
}

bool ChunkProviderHell::saveChunks(bool flag, IProgressUpdate *iprogressupdate)
{
	return true;
}

bool ChunkProviderHell::unload100OldestChunks()
{
	return false;
}

bool ChunkProviderHell::canSave()
{
	return true;
}

jstring ChunkProviderHell::makeString()
{
	return u"HellRandomLevelSource";
}
