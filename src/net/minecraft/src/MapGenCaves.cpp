#include "MapGenCaves.h"

#include <cmath>
#include "World.h"
#include "Block.h"
#include "MathHelper.h"
#include "BlockGrass.h"

MapGenCaves::MapGenCaves()
{
}

void MapGenCaves::generateCaveNode(int_t i, int_t j, byte_t abyte0[], double d, double d1, double d2)
{
	releaseEntitySkin(i, j, abyte0, d, d1, d2,
		1.0f + rand.nextFloat() * 6.0f, 0.0f, 0.0f, -1, -1, 0.5);
}

void MapGenCaves::releaseEntitySkin(int_t i, int_t j, byte_t abyte0[], double d, double d1, double d2,
	float f, float f1, float f2, int_t k, int_t l, double d3)
{
	double d4 = i * 16 + 8;
	double d5 = j * 16 + 8;
	float f3 = 0.0f;
	float f4 = 0.0f;
	Random random(rand.nextLong());

	if (l <= 0)
	{
		int_t i1 = range * 16 - 16;
		l = i1 - random.nextInt(i1 / 4);
	}

	bool flag = false;
	if (k == -1)
	{
		k = l / 2;
		flag = true;
	}

	int_t j1 = random.nextInt(l / 2) + l / 4;
	bool flag1 = random.nextInt(6) == 0;

	for (; k < l; k++)
	{
		double d6 = 1.5 + (double)(MathHelper::sin(((float)k * 3.141593f) / (float)l) * f * 1.0f);
		double d7 = d6 * d3;
		float f5 = MathHelper::cos(f2);
		float f6 = MathHelper::sin(f2);
		d += MathHelper::cos(f1) * f5;
		d1 += f6;
		d2 += MathHelper::sin(f1) * f5;

		if (flag1)
			f2 *= 0.92f;
		else
			f2 *= 0.7f;

		f2 += f4 * 0.1f;
		f1 += f3 * 0.1f;
		f4 *= 0.9f;
		f3 *= 0.75f;
		f4 += (random.nextFloat() - random.nextFloat()) * random.nextFloat() * 2.0f;
		f3 += (random.nextFloat() - random.nextFloat()) * random.nextFloat() * 4.0f;

		if (!flag && k == j1 && f > 1.0f)
		{
			releaseEntitySkin(i, j, abyte0, d, d1, d2,
				random.nextFloat() * 0.5f + 0.5f, f1 - 1.570796f, f2 / 3.0f, k, l, 1.0);
			releaseEntitySkin(i, j, abyte0, d, d1, d2,
				random.nextFloat() * 0.5f + 0.5f, f1 + 1.570796f, f2 / 3.0f, k, l, 1.0);
			return;
		}

		if (!flag && random.nextInt(4) == 0)
			continue;

		double d8a = d - d4;
		double d9a = d2 - d5;
		double d10a = l - k;
		double d11 = f + 2.0f + 16.0f;
		if ((d8a * d8a + d9a * d9a) - d10a * d10a > d11 * d11)
			return;

		if (d < d4 - 16.0 - d6 * 2.0 || d2 < d5 - 16.0 - d6 * 2.0 ||
		    d > d4 + 16.0 + d6 * 2.0 || d2 > d5 + 16.0 + d6 * 2.0)
			continue;

		int_t d8 = MathHelper::floor_double(d - d6) - i * 16 - 1;
		int_t k1 = (MathHelper::floor_double(d + d6) - i * 16) + 1;
		int_t d9 = MathHelper::floor_double(d1 - d7) - 1;
		int_t l1 = MathHelper::floor_double(d1 + d7) + 1;
		int_t d10 = MathHelper::floor_double(d2 - d6) - j * 16 - 1;
		int_t i2 = (MathHelper::floor_double(d2 + d6) - j * 16) + 1;

		if (d8 < 0) d8 = 0;
		if (k1 > 16) k1 = 16;
		if (d9 < 1) d9 = 1;
		if (l1 > 120) l1 = 120;
		if (d10 < 0) d10 = 0;
		if (i2 > 16) i2 = 16;

		bool flag2 = false;
		for (int_t j2 = d8; !flag2 && j2 < k1; j2++)
		{
			for (int_t l2 = d10; !flag2 && l2 < i2; l2++)
			{
				for (int_t i3 = l1 + 1; !flag2 && i3 >= d9 - 1; i3--)
				{
					int_t j3 = (j2 * 16 + l2) * 128 + i3;
					if (i3 < 0 || i3 >= 128) continue;
					if (abyte0[j3] == Block::waterMoving->blockID || abyte0[j3] == Block::waterStill->blockID)
						flag2 = true;
					if (i3 != d9 - 1 && j2 != d8 && j2 != k1 - 1 && l2 != d10 && l2 != i2 - 1)
						i3 = d9;
				}
			}
		}

		if (flag2) continue;

		for (int_t k2 = d8; k2 < k1; k2++)
		{
			double d12 = (((double)(k2 + i * 16) + 0.5) - d) / d6;
			for (int_t k3 = d10; k3 < i2; k3++)
			{
				double d13 = (((double)(k3 + j * 16) + 0.5) - d2) / d6;
				int_t l3 = (k2 * 16 + k3) * 128 + l1;
				bool flag3 = false;
				if (d12 * d12 + d13 * d13 >= 1.0) continue;
				for (int_t i4 = l1 - 1; i4 >= d9; i4--)
				{
					double d14 = (((double)i4 + 0.5) - d1) / d7;
					if (d14 > -0.7 && d12 * d12 + d14 * d14 + d13 * d13 < 1.0)
					{
						byte_t byte0 = abyte0[l3];
						if (byte0 == Block::grass->blockID)
							flag3 = true;
						if (byte0 == Block::stone->blockID || byte0 == Block::dirt->blockID || byte0 == Block::grass->blockID)
						{
							if (i4 < 10)
							{
								abyte0[l3] = (byte_t)Block::lavaMoving->blockID;
							}
							else
							{
								abyte0[l3] = 0;
								if (flag3 && abyte0[l3 - 1] == Block::dirt->blockID)
								{
									abyte0[l3 - 1] = (byte_t)Block::grass->blockID;
								}
							}
						}
					}
					l3--;
				}
			}
		}

		if (flag) break;
	}
}

void MapGenCaves::generateChunk(World *world, int_t i, int_t j, int_t k, int_t l, byte_t abyte0[])
{
	int_t i1 = rand.nextInt(rand.nextInt(rand.nextInt(40) + 1) + 1);
	if (rand.nextInt(15) != 0)
		i1 = 0;

	for (int_t j1 = 0; j1 < i1; j1++)
	{
		double d = i * 16 + rand.nextInt(16);
		double d1 = rand.nextInt(rand.nextInt(120) + 8);
		double d2 = j * 16 + rand.nextInt(16);
		int_t k1 = 1;
		if (rand.nextInt(4) == 0)
		{
			generateCaveNode(k, l, abyte0, d, d1, d2);
			k1 += rand.nextInt(4);
		}
		for (int_t l1 = 0; l1 < k1; l1++)
		{
			float f = rand.nextFloat() * 3.141593f * 2.0f;
			float f1 = ((rand.nextFloat() - 0.5f) * 2.0f) / 8.0f;
			float f2 = rand.nextFloat() * 2.0f + rand.nextFloat();
			releaseEntitySkin(k, l, abyte0, d, d1, d2, f2, f, f1, 0, 0, 1.0);
		}
	}
}
