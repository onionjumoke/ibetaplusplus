#include "WorldGenClay.h"

#include "Block.h"
#include "Material.h"
#include "MathHelper.h"
#include "World.h"

WorldGenClay::WorldGenClay(int_t i)
{
	clayBlockId = Block::blockClay->blockID;
	numberOfBlocks = i;
}

bool WorldGenClay::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	if (world->getBlockMaterial(i, j, k) != Material::water)
		return false;
	float f = random.nextFloat() * 3.141593f;
	double d = (float)(i + 8) + (MathHelper::sin(f) * (float)numberOfBlocks) / 8.0f;
	double d1 = (float)(i + 8) - (MathHelper::sin(f) * (float)numberOfBlocks) / 8.0f;
	double d2 = (float)(k + 8) + (MathHelper::cos(f) * (float)numberOfBlocks) / 8.0f;
	double d3 = (float)(k + 8) - (MathHelper::cos(f) * (float)numberOfBlocks) / 8.0f;
	double d4 = j + random.nextInt(3) + 2;
	double d5 = j + random.nextInt(3) + 2;
	for (int_t l = 0; l <= numberOfBlocks; l++)
	{
		double d6 = d + ((d1 - d) * (double)l) / (double)numberOfBlocks;
		double d7 = d4 + ((d5 - d4) * (double)l) / (double)numberOfBlocks;
		double d8 = d2 + ((d3 - d2) * (double)l) / (double)numberOfBlocks;
		double d9 = (random.nextDouble() * (double)numberOfBlocks) / 16.0;
		double d10 = (double)(MathHelper::sin(((float)l * 3.141593f) / (float)numberOfBlocks) + 1.0f) * d9 + 1.0;
		double d11 = (double)(MathHelper::sin(((float)l * 3.141593f) / (float)numberOfBlocks) + 1.0f) * d9 + 1.0;
		int_t i1 = MathHelper::floor_double(d6 - d10 / 2.0);
		int_t j1 = MathHelper::floor_double(d6 + d10 / 2.0);
		int_t k1 = MathHelper::floor_double(d7 - d11 / 2.0);
		int_t l1 = MathHelper::floor_double(d7 + d11 / 2.0);
		int_t i2 = MathHelper::floor_double(d8 - d10 / 2.0);
		int_t j2 = MathHelper::floor_double(d8 + d10 / 2.0);
		for (int_t k2 = i1; k2 <= j1; k2++)
		{
			for (int_t l2 = k1; l2 <= l1; l2++)
			{
				for (int_t i3 = i2; i3 <= j2; i3++)
				{
					double d12 = (((double)k2 + 0.5) - d6) / (d10 / 2.0);
					double d13 = (((double)l2 + 0.5) - d7) / (d11 / 2.0);
					double d14 = (((double)i3 + 0.5) - d8) / (d10 / 2.0);
					if (d12 * d12 + d13 * d13 + d14 * d14 >= 1.0)
						continue;
					int_t j3 = world->getBlockId(k2, l2, i3);
					if (j3 == Block::sand->blockID)
						world->setBlock(k2, l2, i3, clayBlockId);
				}
			}
		}
	}
	return true;
}
