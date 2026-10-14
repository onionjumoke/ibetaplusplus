#include "BlockStationary.h"
#include "Material.h"
#include "World.h"
#include "Block.h"
#include "BlockFire.h"

BlockStationary::BlockStationary(int_t i, Material *material) : BlockFluid(i, material)
{
	setTickOnLoad(false);
	if (material == Material::lava)
	{
		setTickOnLoad(true);
	}
}

void BlockStationary::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
	BlockFluid::onNeighborBlockChange(world, i, j, k, l);
	if (world->getBlockId(i, j, k) == blockID)
	{
		getLiquidMaterial(world, i, j, k);
	}
}

void BlockStationary::getLiquidMaterial(World *world, int_t i, int_t j, int_t k)
{
	int_t l = world->getBlockMetadata(i, j, k);
	world->editingBlocks = true;
	world->setBlockAndMetadata(i, j, k, blockID - 1, l);
	world->markBlocksDirty(i, j, k, i, j, k);
	world->scheduleBlockUpdate(i, j, k, blockID - 1, tickRate());
	world->editingBlocks = false;
}

void BlockStationary::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	if (blockMaterial == Material::lava)
	{
		int_t l = random.nextInt(3);
		for (int_t i1 = 0; i1 < l; i1++)
		{
			i += random.nextInt(3) - 1;
			j++;
			k += random.nextInt(3) - 1;
			int_t j1 = world->getBlockId(i, j, k);
			if (j1 == 0)
			{
				if (getIsStationary(world, i - 1, j, k) || getIsStationary(world, i + 1, j, k) ||
				    getIsStationary(world, i, j, k - 1) || getIsStationary(world, i, j, k + 1) ||
				    getIsStationary(world, i, j - 1, k) || getIsStationary(world, i, j + 1, k))
				{
					world->setBlockWithNotify(i, j, k, Block::fire->blockID);
					return;
				}
				continue;
			}
			if (Block::blocksList[j1]->blockMaterial->getIsSolid())
			{
				return;
			}
		}
	}
}

bool BlockStationary::getIsStationary(World *world, int_t i, int_t j, int_t k)
{
	return world->getBlockMaterial(i, j, k)->getBurning();
}
