#include "BlockStep.h"
#include "Material.h"
#include "World.h"
#include "Block.h"
#include "IBlockAccess.h"

const jstring BlockStep::bedDirection[4] = { u"stone", u"sand", u"wood", u"cobble" };

BlockStep::BlockStep(int_t i, bool flag) : Block(i, 6, Material::rock)
{
	blockType = flag;
	if (!flag)
	{
		setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
	}
	setLightOpacity(255);
}

int_t BlockStep::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
	if (j == 0) return i > 1 ? 5 : 6;
	if (j == 1)
	{
		if (i == 0) return 208;
		return i != 1 ? 192 : 176;
	}
	if (j == 2) return 4;
	return j != 3 ? 6 : 16;
}

int_t BlockStep::getBlockTextureFromSide(int_t i)
{
	return getBlockTextureFromSideAndMetadata(i, 0);
}

bool BlockStep::isOpaqueCube() { return blockType; }

void BlockStep::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
	if (this != Block::stairSingle)
	{
		Block::onBlockAdded(world, i, j, k);
	}
	int_t l  = world->getBlockId(i, j - 1, k);
	int_t i1 = world->getBlockMetadata(i, j, k);
	int_t j1 = world->getBlockMetadata(i, j - 1, k);
	if (i1 != j1) return;
	if (l == Block::stairSingle->blockID)
	{
		world->setBlockWithNotify(i, j, k, 0);
		world->setBlockAndMetadataWithNotify(i, j - 1, k, Block::stairDouble->blockID, i1);
	}
}

int_t BlockStep::idDropped(int_t i, Random &random)
{
	return Block::stairSingle->blockID;
}

int_t BlockStep::quantityDropped(Random &random)
{
	return !blockType ? 1 : 2;
}

int_t BlockStep::damageDropped(int_t i) { return i; }

bool BlockStep::renderAsNormalBlock() { return blockType; }

bool BlockStep::shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
	if (this != Block::stairSingle)
	{
		Block::shouldSideBeRendered(iblockaccess, i, j, k, l);
	}
	if (l == 1) return true;
	if (!Block::shouldSideBeRendered(iblockaccess, i, j, k, l)) return false;
	if (l == 0) return true;
	return iblockaccess->getBlockId(i, j, k) != blockID;
}
