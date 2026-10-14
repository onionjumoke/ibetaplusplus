#include "BlockGrass.h"
#include "Material.h"
#include "IBlockAccess.h"
#include "WorldChunkManager.h"
#include "ColorizerGrass.h"
#include "World.h"
#include "Block.h"

BlockGrass::BlockGrass(int_t i) : Block(i, Material::grassMaterial)
{
	blockIndexInTexture = 3;
	setTickOnLoad(true);
}

int_t BlockGrass::getBlockTexture(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
	if (l == 1) return 0;
	if (l == 0) return 2;
	Material *material = iblockaccess->getBlockMaterial(i, j + 1, k);
	return material != Material::snow && material != Material::builtSnow ? 3 : 68;
}

int_t BlockGrass::colorMultiplier(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	iblockaccess->getWorldChunkManager()->getBiomeBlock(i, k, 1, 1);  // Java: func_4069_a — populates temperature[]/humidity[] caches
	double d  = iblockaccess->getWorldChunkManager()->temperature[0];
	double d1 = iblockaccess->getWorldChunkManager()->humidity[0];
	return ColorizerGrass::getGrassColor(d, d1);
}

void BlockGrass::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	if (world->multiplayerWorld) return;
	if (world->getBlockLightValue(i, j + 1, k) < 4 && Block::lightOpacity[world->getBlockId(i, j + 1, k)] > 2)
	{
		if (random.nextInt(4) != 0) return;
		world->setBlockWithNotify(i, j, k, Block::dirt->blockID);
	}
	else if (world->getBlockLightValue(i, j + 1, k) >= 9)
	{
		int_t l  = (i + random.nextInt(3)) - 1;
		int_t i1 = (j + random.nextInt(5)) - 3;
		int_t j1 = (k + random.nextInt(3)) - 1;
		int_t k1 = world->getBlockId(l, i1 + 1, j1);
		if (world->getBlockId(l, i1, j1) == Block::dirt->blockID &&
		    world->getBlockLightValue(l, i1 + 1, j1) >= 4 &&
		    Block::lightOpacity[k1] <= 2)
		{
			world->setBlockWithNotify(l, i1, j1, Block::grass->blockID);
		}
	}
}

int_t BlockGrass::idDropped(int_t i, Random &random)
{
	return Block::dirt->idDropped(0, random);
}
