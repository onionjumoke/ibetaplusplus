#include "WorldGenLakes.h"

#include "Block.h"
#include "BlockGrass.h"
#include "EnumSkyBlock.h"
#include "Material.h"
#include "World.h"

WorldGenLakes::WorldGenLakes(int_t i)
{
	field_15235_a = i;
}

bool WorldGenLakes::generate(World *world, Random &random, int_t i, int_t j, int_t k)
{
	i -= 8;
	for (k -= 8; j > 0 && world->isAirBlock(i, j, k); j--)
	{
	}
	j -= 4;
	bool aflag[2048] = {};
	int_t l = random.nextInt(4) + 4;
	for (int_t i1 = 0; i1 < l; i1++)
	{
		double d = random.nextDouble() * 6.0 + 3.0;
		double d1 = random.nextDouble() * 4.0 + 2.0;
		double d2 = random.nextDouble() * 6.0 + 3.0;
		double d3 = random.nextDouble() * (16.0 - d - 2.0) + 1.0 + d / 2.0;
		double d4 = random.nextDouble() * (8.0 - d1 - 4.0) + 2.0 + d1 / 2.0;
		double d5 = random.nextDouble() * (16.0 - d2 - 2.0) + 1.0 + d2 / 2.0;
		for (int_t j4 = 1; j4 < 15; j4++)
		{
			for (int_t k4 = 1; k4 < 15; k4++)
			{
				for (int_t l4 = 1; l4 < 7; l4++)
				{
					double d6 = ((double)j4 - d3) / (d / 2.0);
					double d7 = ((double)l4 - d4) / (d1 / 2.0);
					double d8 = ((double)k4 - d5) / (d2 / 2.0);
					double d9 = d6 * d6 + d7 * d7 + d8 * d8;
					if (d9 < 1.0)
						aflag[(j4 * 16 + k4) * 8 + l4] = true;
				}
			}
		}
	}
	for (int_t j1 = 0; j1 < 16; j1++)
	{
		for (int_t j2 = 0; j2 < 16; j2++)
		{
			for (int_t j3 = 0; j3 < 8; j3++)
			{
				bool flag = !aflag[(j1 * 16 + j2) * 8 + j3] && (j1 < 15 && aflag[((j1 + 1) * 16 + j2) * 8 + j3] || j1 > 0 && aflag[((j1 - 1) * 16 + j2) * 8 + j3] || j2 < 15 && aflag[(j1 * 16 + (j2 + 1)) * 8 + j3] || j2 > 0 && aflag[(j1 * 16 + (j2 - 1)) * 8 + j3] || j3 < 7 && aflag[(j1 * 16 + j2) * 8 + (j3 + 1)] || j3 > 0 && aflag[(j1 * 16 + j2) * 8 + (j3 - 1)]);
				if (!flag)
					continue;
				Material *material = world->getBlockMaterial(i + j1, j + j3, k + j2);
				if (j3 >= 4 && material->getIsLiquid())
					return false;
				if (j3 < 4 && !material->isSolid() && world->getBlockId(i + j1, j + j3, k + j2) != field_15235_a)
					return false;
			}
		}
	}
	for (int_t k1 = 0; k1 < 16; k1++)
	{
		for (int_t k2 = 0; k2 < 16; k2++)
		{
			for (int_t k3 = 0; k3 < 8; k3++)
			{
				if (aflag[(k1 * 16 + k2) * 8 + k3])
					world->setBlock(i + k1, j + k3, k + k2, k3 < 4 ? field_15235_a : 0);
			}
		}
	}
	for (int_t l1 = 0; l1 < 16; l1++)
	{
		for (int_t l2 = 0; l2 < 16; l2++)
		{
			for (int_t l3 = 4; l3 < 8; l3++)
			{
				if (aflag[(l1 * 16 + l2) * 8 + l3] && world->getBlockId(i + l1, (j + l3) - 1, k + l2) == Block::dirt->blockID && world->getSavedLightValue(EnumSkyBlock::Sky, i + l1, j + l3, k + l2) > 0)
					world->setBlock(i + l1, (j + l3) - 1, k + l2, Block::grass->blockID);
			}
		}
	}
	if (Block::blocksList[field_15235_a]->blockMaterial == Material::lava)
	{
		for (int_t i2 = 0; i2 < 16; i2++)
		{
			for (int_t i3 = 0; i3 < 16; i3++)
			{
				for (int_t i4 = 0; i4 < 8; i4++)
				{
					bool flag1 = !aflag[(i2 * 16 + i3) * 8 + i4] && (i2 < 15 && aflag[((i2 + 1) * 16 + i3) * 8 + i4] || i2 > 0 && aflag[((i2 - 1) * 16 + i3) * 8 + i4] || i3 < 15 && aflag[(i2 * 16 + (i3 + 1)) * 8 + i4] || i3 > 0 && aflag[(i2 * 16 + (i3 - 1)) * 8 + i4] || i4 < 7 && aflag[(i2 * 16 + i3) * 8 + (i4 + 1)] || i4 > 0 && aflag[(i2 * 16 + i3) * 8 + (i4 - 1)]);
					if (flag1 && (i4 < 4 || random.nextInt(2) != 0) && world->getBlockMaterial(i + i2, j + i4, k + i3)->isSolid())
						world->setBlock(i + i2, j + i4, k + i3, Block::stone->blockID);
				}
			}
		}
	}
	return true;
}
