#include "BlockFluid.h"

#include "IBlockAccess.h"
#include "Material.h"
#include "Vec3D.h"
#include "World.h"
#include "java/Random.h"
#include <cmath>

BlockFluid::BlockFluid(int_t i, Material *material) :
	Block(i, (material != Material::lava ? 12 : 14) * 16 + 13, material)
{
	float f = 0.0f;
	float f1 = 0.0f;
	setBlockBounds(0.0f + f1, 0.0f + f, 0.0f + f1, 1.0f + f1, 1.0f + f, 1.0f + f1);
	setTickOnLoad(true);
}

int_t BlockFluid::colorMultiplier(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	return 0xffffff;
}

float BlockFluid::getPercentAir(int_t i)
{
	if (i >= 8)
	{
		i = 0;
	}
	float f = (float)(i + 1) / 9.0f;
	return f;
}

int_t BlockFluid::getBlockTextureFromSide(int_t i)
{
	if (i == 0 || i == 1)
	{
		return blockIndexInTexture;
	}
	else
	{
		return blockIndexInTexture + 1;
	}
}

int_t BlockFluid::getFlowDecay(World *world, int_t i, int_t j, int_t k)
{
	if (world->getBlockMaterial(i, j, k) != blockMaterial)
	{
		return -1;
	}
	else
	{
		return world->getBlockMetadata(i, j, k);
	}
}

int_t BlockFluid::getEffectiveFlowDecay(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	if (iblockaccess->getBlockMaterial(i, j, k) != blockMaterial)
	{
		return -1;
	}
	int_t l = iblockaccess->getBlockMetadata(i, j, k);
	if (l >= 8)
	{
		l = 0;
	}
	return l;
}

bool BlockFluid::renderAsNormalBlock()
{
	return false;
}

bool BlockFluid::isOpaqueCube()
{
	return false;
}

bool BlockFluid::canCollideCheck(int_t i, bool flag)
{
	return flag && i == 0;
}

bool BlockFluid::getIsBlockSolid(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
	Material *material = iblockaccess->getBlockMaterial(i, j, k);
	if (material == blockMaterial)
	{
		return false;
	}
	if (material == Material::ice)
	{
		return false;
	}
	if (l == 1)
	{
		return true;
	}
	else
	{
		return Block::getIsBlockSolid(iblockaccess, i, j, k, l);
	}
}

bool BlockFluid::shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l)
{
	Material *material = iblockaccess->getBlockMaterial(i, j, k);
	if (material == blockMaterial)
	{
		return false;
	}
	if (material == Material::ice)
	{
		return false;
	}
	if (l == 1)
	{
		return true;
	}
	else
	{
		return Block::shouldSideBeRendered(iblockaccess, i, j, k, l);
	}
}

AxisAlignedBB *BlockFluid::getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k)
{
	return nullptr;
}

int_t BlockFluid::getRenderType()
{
	return 4;
}

int_t BlockFluid::idDropped(int_t i, Random &random)
{
	return 0;
}

int_t BlockFluid::quantityDropped(Random &random)
{
	return 0;
}

Vec3D *BlockFluid::getFlowVector(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	Vec3D *vec3d = Vec3D::createVector(0.0, 0.0, 0.0);
	int_t l = getEffectiveFlowDecay(iblockaccess, i, j, k);
	for (int_t i1 = 0; i1 < 4; i1++)
	{
		int_t j1 = i;
		int_t k1 = j;
		int_t l1 = k;
		if (i1 == 0)
		{
			j1--;
		}
		if (i1 == 1)
		{
			l1--;
		}
		if (i1 == 2)
		{
			j1++;
		}
		if (i1 == 3)
		{
			l1++;
		}
		int_t i2 = getEffectiveFlowDecay(iblockaccess, j1, k1, l1);
		if (i2 < 0)
		{
			if (iblockaccess->getBlockMaterial(j1, k1, l1)->getIsSolid())
			{
				continue;
			}
			i2 = getEffectiveFlowDecay(iblockaccess, j1, k1 - 1, l1);
			if (i2 >= 0)
			{
				int_t j2 = i2 - (l - 8);
				vec3d = vec3d->addVector((j1 - i) * j2, (k1 - j) * j2, (l1 - k) * j2);
			}
			continue;
		}
		if (i2 >= 0)
		{
			int_t k2 = i2 - l;
			vec3d = vec3d->addVector((j1 - i) * k2, (k1 - j) * k2, (l1 - k) * k2);
		}
	}

	if (iblockaccess->getBlockMetadata(i, j, k) >= 8)
	{
		bool flag = false;
		if (flag || getIsBlockSolid(iblockaccess, i, j, k - 1, 2)) flag = true;
		if (flag || getIsBlockSolid(iblockaccess, i, j, k + 1, 3)) flag = true;
		if (flag || getIsBlockSolid(iblockaccess, i - 1, j, k, 4)) flag = true;
		if (flag || getIsBlockSolid(iblockaccess, i + 1, j, k, 5)) flag = true;
		if (flag || getIsBlockSolid(iblockaccess, i, j + 1, k - 1, 2)) flag = true;
		if (flag || getIsBlockSolid(iblockaccess, i, j + 1, k + 1, 3)) flag = true;
		if (flag || getIsBlockSolid(iblockaccess, i - 1, j + 1, k, 4)) flag = true;
		if (flag || getIsBlockSolid(iblockaccess, i + 1, j + 1, k, 5)) flag = true;
		if (flag)
		{
			vec3d = vec3d->normalize()->addVector(0.0, -6.0, 0.0);
		}
	}
	vec3d = vec3d->normalize();
	return vec3d;
}

void BlockFluid::velocityToAddToEntity(World *world, int_t i, int_t j, int_t k, Entity *entity, Vec3D *vec3d)
{
	Vec3D *vec3d1 = getFlowVector(world, i, j, k);
	vec3d->xCoord += vec3d1->xCoord;
	vec3d->yCoord += vec3d1->yCoord;
	vec3d->zCoord += vec3d1->zCoord;
}

int_t BlockFluid::tickRate()
{
	if (blockMaterial == Material::water)
	{
		return 5;
	}
	return blockMaterial != Material::lava ? 0 : 30;
}

float BlockFluid::getBlockBrightness(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k)
{
	float f = iblockaccess->getLightBrightness(i, j, k);
	float f1 = iblockaccess->getLightBrightness(i, j + 1, k);
	return f <= f1 ? f1 : f;
}

void BlockFluid::updateTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	Block::updateTick(world, i, j, k, random);
}

int_t BlockFluid::getRenderBlockPass()
{
	return blockMaterial != Material::water ? 0 : 1;
}

void BlockFluid::randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random)
{
	if (blockMaterial == Material::water && random.nextInt(64) == 0)
	{
		int_t l = world->getBlockMetadata(i, j, k);
		if (l > 0 && l < 8)
		{
			world->playSoundEffect((float)i + 0.5f, (float)j + 0.5f, (float)k + 0.5f, u"liquid.water", random.nextFloat() * 0.25f + 0.75f, random.nextFloat() * 1.0f + 0.5f);
		}
	}
	if (blockMaterial == Material::lava && world->getBlockMaterial(i, j + 1, k) == Material::air && !world->isBlockOpaqueCube(i, j + 1, k) && random.nextInt(100) == 0)
	{
		double d = (float)i + random.nextFloat();
		double d1 = (double)j + maxY;
		double d2 = (float)k + random.nextFloat();
		world->spawnParticle(u"lava", d, d1, d2, 0.0, 0.0, 0.0);
	}
}

double BlockFluid::getFlowDirection(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, Material *material)
{
	Vec3D *vec3d = nullptr;
	if (material == Material::water)
	{
		vec3d = ((BlockFluid *)Block::waterMoving)->getFlowVector(iblockaccess, i, j, k);
	}
	if (material == Material::lava)
	{
		vec3d = ((BlockFluid *)Block::lavaMoving)->getFlowVector(iblockaccess, i, j, k);
	}
	if (vec3d->xCoord == 0.0 && vec3d->zCoord == 0.0)
	{
		return -1000.0;
	}
	else
	{
		return std::atan2(vec3d->zCoord, vec3d->xCoord) - 1.5707963267948966;
	}
}

double BlockFluid::func_293_a(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, Material *material)
{
	return getFlowDirection(iblockaccess, i, j, k, material);
}

void BlockFluid::onBlockAdded(World *world, int_t i, int_t j, int_t k)
{
	checkForHarden(world, i, j, k);
}

void BlockFluid::onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l)
{
	checkForHarden(world, i, j, k);
}

void BlockFluid::checkForHarden(World *world, int_t i, int_t j, int_t k)
{
	if (world->getBlockId(i, j, k) != blockID)
	{
		return;
	}
	if (blockMaterial == Material::lava)
	{
		bool flag = false;
		if (flag || world->getBlockMaterial(i, j, k - 1) == Material::water) flag = true;
		if (flag || world->getBlockMaterial(i, j, k + 1) == Material::water) flag = true;
		if (flag || world->getBlockMaterial(i - 1, j, k) == Material::water) flag = true;
		if (flag || world->getBlockMaterial(i + 1, j, k) == Material::water) flag = true;
		if (flag || world->getBlockMaterial(i, j + 1, k) == Material::water) flag = true;
		if (flag)
		{
			int_t l = world->getBlockMetadata(i, j, k);
			if (l == 0)
			{
				world->setBlockWithNotify(i, j, k, Block::obsidian->blockID);
			}
			else if (l <= 4)
			{
				world->setBlockWithNotify(i, j, k, Block::cobblestone->blockID);
			}
			triggerLavaMixEffects(world, i, j, k);
		}
	}
}

void BlockFluid::triggerLavaMixEffects(World *world, int_t i, int_t j, int_t k)
{
	world->playSoundEffect((float)i + 0.5f, (float)j + 0.5f, (float)k + 0.5f, u"random.fizz", 0.5f, 2.6f + (world->rand.nextFloat() - world->rand.nextFloat()) * 0.8f);
	for (int_t l = 0; l < 8; l++)
	{
		world->spawnParticle(u"largesmoke", (double)i + world->rand.nextDouble(), (double)j + 1.2, (double)k + world->rand.nextDouble(), 0.0, 0.0, 0.0);
	}
}
