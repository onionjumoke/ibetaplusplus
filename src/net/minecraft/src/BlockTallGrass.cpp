#include "BlockTallGrass.h"
#include "IBlockAccess.h"
#include "WorldChunkManager.h"
#include "ColorizerGrass.h"
#include "Item.h"
#include "java/Random.h"

BlockTallGrass::BlockTallGrass(int_t i, int_t j) : BlockFlower(i, j)
{
	float f = 0.4f;
	setBlockBounds(0.5f - f, 0.0f, 0.5f - f, 0.5f + f, 0.8f, 0.5f + f);
}

int_t BlockTallGrass::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
	if (j == 1) return blockIndexInTexture;
	if (j == 2) return blockIndexInTexture + 16 + 1;
	if (j == 0) return blockIndexInTexture + 16;
	return blockIndexInTexture;
}

int_t BlockTallGrass::colorMultiplier(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	int_t l = iblockaccess->getBlockMetadata(i, j, k);
	if (l == 0) return 0xffffff;
	long_t l1 = (long_t)i * 0x2fc20f + (long_t)k * 0x5d8875 + j;
	l1 = l1 * l1 * 0x285b825L + l1 * 11LL;
	i = (int_t)((long_t)i + (l1 >> 14 & 31LL));
	j = (int_t)((long_t)j + (l1 >> 19 & 31LL));
	k = (int_t)((long_t)k + (l1 >> 24 & 31LL));
	iblockaccess->getWorldChunkManager()->getBiomeBlock(i, k, 1, 1);
	double d  = iblockaccess->getWorldChunkManager()->temperature[0];
	double d1 = iblockaccess->getWorldChunkManager()->humidity[0];
	return ColorizerGrass::getGrassColor(d, d1);
}

int_t BlockTallGrass::idDropped(int_t i, Random &random)
{
	if (random.nextInt(8) == 0) return Item::seeds->shiftedIndex;
	return -1;
}
