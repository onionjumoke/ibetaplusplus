#include "BlockPumpkin.h"
#include "Material.h"
#include "World.h"
#include "Block.h"
#include "EntityLiving.h"
#include "MathHelper.h"

BlockPumpkin::BlockPumpkin(int_t i, int_t j, bool flag) : Block(i, Material::pumpkin)
{
	blockIndexInTexture = j;
	setTickOnLoad(true);
	blockType = flag;
}

int_t BlockPumpkin::getBlockTextureFromSideAndMetadata(int_t i, int_t j)
{
	if (i == 1) return blockIndexInTexture;
	if (i == 0) return blockIndexInTexture;
	int_t k = blockIndexInTexture + 1 + 16;
	if (blockType) k++;
	if (j == 2 && i == 2) return k;
	if (j == 3 && i == 5) return k;
	if (j == 0 && i == 3) return k;
	if (j == 1 && i == 4) return k;
	return blockIndexInTexture + 16;
}

int_t BlockPumpkin::getBlockTextureFromSide(int_t i)
{
	if (i == 1) return blockIndexInTexture;
	if (i == 0) return blockIndexInTexture;
	if (i == 3) return blockIndexInTexture + 1 + 16;
	return blockIndexInTexture + 16;
}

void BlockPumpkin::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
	Block::onBlockAdded(world, i, j, k);
}

bool BlockPumpkin::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
	int_t l = world->getBlockId(i, j, k);
	return (l == 0 || Block::blocksList[l]->blockMaterial->getIsGroundCover()) &&
	       world->isBlockNormalCube(i, j - 1, k);
}

void BlockPumpkin::onBlockPlacedBy(World *world, int_t i, int_t j, int_t k, EntityLiving *entityliving)
{
	int_t l = MathHelper::floor_double((double)(entityliving->rotationYaw * 4.0f / 360.0f) + 2.5) & 3;
	world->setBlockMetadataWithNotify(i, j, k, l);
}
