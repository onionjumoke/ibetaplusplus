#include "BlockFire.h"

#include "BlockPortal.h"
#include "IBlockAccess.h"
#include "Material.h"
#include "World.h"
#include "java/Random.h"
#include "BlockLeaves.h"
#include "BlockTallGrass.h"

BlockFire::BlockFire(int_t i, int_t j) :
	Block(i, j, Material::fire)
{
	for (int_t l = 0; l < 256; l++)
	{
		chanceToEncourageFire[l] = 0;
		abilityToCatchFire[l] = 0;
	}
	setTickOnLoad(true);
}

void BlockFire::initializeBlock()
{
	setBurnRate(Block::planks->blockID, 5, 20);
	setBurnRate(Block::fence->blockID, 5, 20);
	setBurnRate(Block::stairCompactPlanks->blockID, 5, 20);
	setBurnRate(Block::wood->blockID, 5, 5);
	setBurnRate(Block::leaves->blockID, 30, 60);
	setBurnRate(Block::bookShelf->blockID, 30, 20);
	setBurnRate(Block::tnt->blockID, 15, 100);
	setBurnRate(Block::tallGrass->blockID, 60, 100);
	setBurnRate(Block::cloth->blockID, 30, 60);
}

void BlockFire::setBurnRate(int_t i, int_t j, int_t k)
{
	chanceToEncourageFire[i] = j;
	abilityToCatchFire[i] = k;
}

AxisAlignedBB *BlockFire::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	return nullptr;
}

bool BlockFire::isOpaqueCube()
{
	return false;
}

bool BlockFire::renderAsNormalBlock()
{
	return false;
}

int_t BlockFire::getRenderType()
{
	return 3;
}

int_t BlockFire::quantityDropped(Random &random)
{
	return 0;
}

int_t BlockFire::tickRate()
{
	return 40;
}

void BlockFire::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	bool flag = world->getBlockId(i, j - 1, k) == Block::netherrack->blockID;
	if (!canPlaceBlockAt(world, i, j, k))
	{
		world->setBlockWithNotify(i, j, k, 0);
	}
	if (!flag && (world->canBlockBeRainedOn(i, j, k) || world->canBlockBeRainedOn(i - 1, j, k) || world->canBlockBeRainedOn(i + 1, j, k) || world->canBlockBeRainedOn(i, j, k - 1) || world->canBlockBeRainedOn(i, j, k + 1)))
	{
		world->setBlockWithNotify(i, j, k, 0);
		return;
	}
	int_t l = world->getBlockMetadata(i, j, k);
	if (l < 15)
	{
		world->setBlockMetadata(i, j, k, l + random.nextInt(3) / 2);
	}
	world->scheduleBlockUpdate(i, j, k, blockID, tickRate());
	if (!flag && !hasFlammableNeighbor(world, i, j, k))
	{
		if (!world->isBlockNormalCube(i, j - 1, k) || l > 3)
		{
			world->setBlockWithNotify(i, j, k, 0);
		}
		return;
	}
	if (!flag && !canBlockCatchFire(world, i, j - 1, k) && l == 15 && random.nextInt(4) == 0)
	{
		world->setBlockWithNotify(i, j, k, 0);
		return;
	}
	tryToCatchBlockOnFire(world, i + 1, j, k, 300, random, l);
	tryToCatchBlockOnFire(world, i - 1, j, k, 300, random, l);
	tryToCatchBlockOnFire(world, i, j - 1, k, 250, random, l);
	tryToCatchBlockOnFire(world, i, j + 1, k, 250, random, l);
	tryToCatchBlockOnFire(world, i, j, k - 1, 300, random, l);
	tryToCatchBlockOnFire(world, i, j, k + 1, 300, random, l);
	for (int_t i1 = i - 1; i1 <= i + 1; i1++)
	{
		for (int_t j1 = k - 1; j1 <= k + 1; j1++)
		{
			for (int_t k1 = j - 1; k1 <= j + 4; k1++)
			{
				if (i1 == i && k1 == j && j1 == k)
				{
					continue;
				}
				int_t l1 = 100;
				if (k1 > j + 1)
				{
					l1 += (k1 - (j + 1)) * 100;
				}
				int_t i2 = getChanceOfNeighborsEncouragingFire(world, i1, k1, j1);
				if (i2 <= 0)
				{
					continue;
				}
				int_t j2 = (i2 + 40) / (l + 30);
				if (j2 <= 0 || random.nextInt(l1) > j2 || world->canBlockBeRainedOn(i1, k1, j1) || world->canBlockBeRainedOn(i1 - 1, k1, k) || world->canBlockBeRainedOn(i1 + 1, k1, j1) || world->canBlockBeRainedOn(i1, k1, j1 - 1) || world->canBlockBeRainedOn(i1, k1, j1 + 1))
				{
					continue;
				}
				int_t k2 = l + random.nextInt(5) / 4;
				if (k2 > 15)
				{
					k2 = 15;
				}
				world->setBlockAndMetadataWithNotify(i1, k1, j1, blockID, k2);
			}
		}
	}
}

void BlockFire::tryToCatchBlockOnFire(World *world, int_t i, int_t j, int_t k, int_t l, Random &random, int_t i1)
{
	int_t j1 = abilityToCatchFire[world->getBlockId(i, j, k)];
	if (random.nextInt(l) < j1)
	{
		bool flag = world->getBlockId(i, j, k) == Block::tnt->blockID;
		if (random.nextInt(i1 + 10) < 5 && !world->canBlockBeRainedOn(i, j, k))
		{
			int_t k1 = i1 + random.nextInt(5) / 4;
			if (k1 > 15)
			{
				k1 = 15;
			}
			world->setBlockAndMetadataWithNotify(i, j, k, blockID, k1);
		}
		else
		{
			world->setBlockWithNotify(i, j, k, 0);
		}
		if (flag)
		{
			Block::tnt->onBlockDestroyedByPlayer(world, i, j, k, 1);
		}
	}
}

bool BlockFire::hasFlammableNeighbor(World *world, int_t i, int_t j, int_t k)
{
	if (canBlockCatchFire(world, i + 1, j, k)) return true;
	if (canBlockCatchFire(world, i - 1, j, k)) return true;
	if (canBlockCatchFire(world, i, j - 1, k)) return true;
	if (canBlockCatchFire(world, i, j + 1, k)) return true;
	if (canBlockCatchFire(world, i, j, k - 1)) return true;
	return canBlockCatchFire(world, i, j, k + 1);
}

int_t BlockFire::getChanceOfNeighborsEncouragingFire(World *world, int_t i, int_t j, int_t k)
{
	int_t l = 0;
	if (!world->isAirBlock(i, j, k))
	{
		return 0;
	}
	else
	{
		l = getChanceToEncourageFire(world, i + 1, j, k, l);
		l = getChanceToEncourageFire(world, i - 1, j, k, l);
		l = getChanceToEncourageFire(world, i, j - 1, k, l);
		l = getChanceToEncourageFire(world, i, j + 1, k, l);
		l = getChanceToEncourageFire(world, i, j, k - 1, l);
		l = getChanceToEncourageFire(world, i, j, k + 1, l);
		return l;
	}
}

bool BlockFire::isCollidable()
{
	return false;
}

bool BlockFire::canBlockCatchFire(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	return chanceToEncourageFire[iblockaccess->getBlockId(i, j, k)] > 0;
}

int_t BlockFire::getChanceToEncourageFire(World *world, int_t i, int_t j, int_t k, int_t l)
{
	int_t i1 = chanceToEncourageFire[world->getBlockId(i, j, k)];
	if (i1 > l)
	{
		return i1;
	}
	else
	{
		return l;
	}
}

bool BlockFire::canPlaceBlockAt(World *world, int_t i, int_t j, int_t k)
{
	return world->isBlockNormalCube(i, j - 1, k) || hasFlammableNeighbor(world, i, j, k);
}

void BlockFire::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
	if (!world->isBlockNormalCube(i, j - 1, k) && !hasFlammableNeighbor(world, i, j, k))
	{
		world->setBlockWithNotify(i, j, k, 0);
		return;
	}
	else
	{
		return;
	}
}

void BlockFire::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
	if (world->getBlockId(i, j - 1, k) == Block::obsidian->blockID && Block::portal->tryToCreatePortal(world, i, j, k))
	{
		return;
	}
	if (!world->isBlockNormalCube(i, j - 1, k) && !hasFlammableNeighbor(world, i, j, k))
	{
		world->setBlockWithNotify(i, j, k, 0);
		return;
	}
	else
	{
		world->scheduleBlockUpdate(i, j, k, blockID, tickRate());
		return;
	}
}

void BlockFire::randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	if (random.nextInt(24) == 0)
	{
		world->playSoundEffect((float)i + 0.5f, (float)j + 0.5f, (float)k + 0.5f, u"fire.fire", 1.0f + random.nextFloat(), random.nextFloat() * 0.7f + 0.3f);
	}
	if (world->isBlockNormalCube(i, j - 1, k) || Block::fire->canBlockCatchFire(world, i, j - 1, k))
	{
		for (int_t l = 0; l < 3; l++)
		{
			float f = (float)i + random.nextFloat();
			float f6 = (float)j + random.nextFloat() * 0.5f + 0.5f;
			float f12 = (float)k + random.nextFloat();
			world->spawnParticle(u"largesmoke", f, f6, f12, 0.0, 0.0, 0.0);
		}
	}
	else
	{
		if (Block::fire->canBlockCatchFire(world, i - 1, j, k))
		{
			for (int_t i1 = 0; i1 < 2; i1++)
			{
				float f1 = (float)i + random.nextFloat() * 0.1f;
				float f7 = (float)j + random.nextFloat();
				float f13 = (float)k + random.nextFloat();
				world->spawnParticle(u"largesmoke", f1, f7, f13, 0.0, 0.0, 0.0);
			}
		}
		if (Block::fire->canBlockCatchFire(world, i + 1, j, k))
		{
			for (int_t j1 = 0; j1 < 2; j1++)
			{
				float f2 = (float)(i + 1) - random.nextFloat() * 0.1f;
				float f8 = (float)j + random.nextFloat();
				float f14 = (float)k + random.nextFloat();
				world->spawnParticle(u"largesmoke", f2, f8, f14, 0.0, 0.0, 0.0);
			}
		}
		if (Block::fire->canBlockCatchFire(world, i, j, k - 1))
		{
			for (int_t k1 = 0; k1 < 2; k1++)
			{
				float f3 = (float)i + random.nextFloat();
				float f9 = (float)j + random.nextFloat();
				float f15 = (float)k + random.nextFloat() * 0.1f;
				world->spawnParticle(u"largesmoke", f3, f9, f15, 0.0, 0.0, 0.0);
			}
		}
		if (Block::fire->canBlockCatchFire(world, i, j, k + 1))
		{
			for (int_t l1 = 0; l1 < 2; l1++)
			{
				float f4 = (float)i + random.nextFloat();
				float f10 = (float)j + random.nextFloat();
				float f16 = (float)(k + 1) - random.nextFloat() * 0.1f;
				world->spawnParticle(u"largesmoke", f4, f10, f16, 0.0, 0.0, 0.0);
			}
		}
		if (Block::fire->canBlockCatchFire(world, i, j + 1, k))
		{
			for (int_t i2 = 0; i2 < 2; i2++)
			{
				float f5 = (float)i + random.nextFloat();
				float f11 = (float)(j + 1) - random.nextFloat() * 0.1f;
				float f17 = (float)k + random.nextFloat();
				world->spawnParticle(u"largesmoke", f5, f11, f17, 0.0, 0.0, 0.0);
			}
		}
	}
}
