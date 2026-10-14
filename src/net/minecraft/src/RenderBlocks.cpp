#include "RenderBlocks.h"

#include "Block.h"
#include "BlockBed.h"
#include "BlockDoor.h"
#include "BlockFire.h"
#include "BlockFluid.h"
#include "BlockGrass.h"
#include "BlockPistonBase.h"
#include "BlockPistonExtension.h"
#include "BlockRail.h"
#include "BlockRedstoneRepeater.h"
#include "BlockRedstoneWire.h"
#include "BlockTallGrass.h"
#include "IBlockAccess.h"
#include "Material.h"
#include "MathHelper.h"
#include "ModelBed.h"
#include "Tessellator.h"
#include "Vec3D.h"
#include "World.h"
#include "client/Minecraft.h"
#include "pc/OpenGL.h"

bool RenderBlocks::fancyGrass = true;

RenderBlocks::RenderBlocks(IBlockAccess *iblockaccess) :
	overrideBlockTexture(-1),
	flipTexture(false),
	renderAllFaces(false),
	field_31088_b(true),
	eastFaceRotation(0),
	westFaceRotation(0),
	southFaceRotation(0),
	northFaceRotation(0),
	topFaceRotation(0),
	bottomFaceRotation(0),
	lightingQuality(1),
	blockAccess(iblockaccess),
	enableAO(false),
	lightValueOwn(0.0f),
	aoLightValueXNeg(0.0f),
	aoLightValueYNeg(0.0f),
	aoLightValueZNeg(0.0f),
	aoLightValueXPos(0.0f),
	aoLightValueYPos(0.0f),
	aoLightValueZPos(0.0f),
	aoLightValueXYZNNN(0.0f),
	aoLightValueXNeg2(0.0f),
	aoLightValueXYZNNP(0.0f),
	aoLightValueYNeg2(0.0f),
	aoLightValueXYZPNN(0.0f),
	aoLightValueXYZPNP(0.0f),
	aoLightValueXPos2(0.0f),
	aoLightValueXYZPPP(0.0f),
	aoLightValueXYZNNP2(0.0f),
	aoLightValueXPosUp(0.0f),
	aoLightValueXYZPNP2(0.0f),
	aoLightValueZNegUp(0.0f),
	aoLightValueXYZPPN(0.0f),
	aoLightValueXPosUp2(0.0f),
	aoLightValueZPosUp(0.0f),
	aoLightValueXYZPPP2(0.0f),
	aoLightValueXNegNorth(0.0f),
	aoLightValueXPosNorth(0.0f),
	aoLightValueXNegSouth(0.0f),
	aoLightValueXPosSouth(0.0f),
	colorRedTopLeft(0.0f),
	colorRedBottomLeft(0.0f),
	colorRedBottomRight(0.0f),
	colorRedTopRight(0.0f),
	colorGreenTopLeft(0.0f),
	colorGreenBottomLeft(0.0f),
	colorGreenBottomRight(0.0f),
	colorGreenTopRight(0.0f),
	colorBlueTopLeft(0.0f),
	colorBlueBottomLeft(0.0f),
	colorBlueBottomRight(0.0f),
	colorBlueTopRight(0.0f),
	canBlockGrassAboveNorth(false),
	canBlockGrassAboveEast(false),
	canBlockGrassAboveWest(false),
	canBlockGrassAboveSouth(false),
	canBlockGrassBelowNorth(false),
	canBlockGrassBelowEast(false),
	canBlockGrassBelowSouth(false),
	canBlockGrassBelowWest(false),
	canBlockGrassBelowNorthWest(false),
	canBlockGrassBelowEastSouth(false),
	canBlockGrassBelowWestSouth(false),
	canBlockGrassBelowSouthEast(false)
{
}

RenderBlocks::RenderBlocks() :
	overrideBlockTexture(-1),
	flipTexture(false),
	renderAllFaces(false),
	field_31088_b(true),
	eastFaceRotation(0),
	westFaceRotation(0),
	southFaceRotation(0),
	northFaceRotation(0),
	topFaceRotation(0),
	bottomFaceRotation(0),
	lightingQuality(1),
	blockAccess(nullptr),
	enableAO(false),
	lightValueOwn(0.0f),
	aoLightValueXNeg(0.0f),
	aoLightValueYNeg(0.0f),
	aoLightValueZNeg(0.0f),
	aoLightValueXPos(0.0f),
	aoLightValueYPos(0.0f),
	aoLightValueZPos(0.0f),
	aoLightValueXYZNNN(0.0f),
	aoLightValueXNeg2(0.0f),
	aoLightValueXYZNNP(0.0f),
	aoLightValueYNeg2(0.0f),
	aoLightValueXYZPNN(0.0f),
	aoLightValueXYZPNP(0.0f),
	aoLightValueXPos2(0.0f),
	aoLightValueXYZPPP(0.0f),
	aoLightValueXYZNNP2(0.0f),
	aoLightValueXPosUp(0.0f),
	aoLightValueXYZPNP2(0.0f),
	aoLightValueZNegUp(0.0f),
	aoLightValueXYZPPN(0.0f),
	aoLightValueXPosUp2(0.0f),
	aoLightValueZPosUp(0.0f),
	aoLightValueXYZPPP2(0.0f),
	aoLightValueXNegNorth(0.0f),
	aoLightValueXPosNorth(0.0f),
	aoLightValueXNegSouth(0.0f),
	aoLightValueXPosSouth(0.0f),
	colorRedTopLeft(0.0f),
	colorRedBottomLeft(0.0f),
	colorRedBottomRight(0.0f),
	colorRedTopRight(0.0f),
	colorGreenTopLeft(0.0f),
	colorGreenBottomLeft(0.0f),
	colorGreenBottomRight(0.0f),
	colorGreenTopRight(0.0f),
	colorBlueTopLeft(0.0f),
	colorBlueBottomLeft(0.0f),
	colorBlueBottomRight(0.0f),
	colorBlueTopRight(0.0f),
	canBlockGrassAboveNorth(false),
	canBlockGrassAboveEast(false),
	canBlockGrassAboveWest(false),
	canBlockGrassAboveSouth(false),
	canBlockGrassBelowNorth(false),
	canBlockGrassBelowEast(false),
	canBlockGrassBelowSouth(false),
	canBlockGrassBelowWest(false),
	canBlockGrassBelowNorthWest(false),
	canBlockGrassBelowEastSouth(false),
	canBlockGrassBelowWestSouth(false),
	canBlockGrassBelowSouthEast(false)
{
}

void RenderBlocks::renderBlockUsingTexture(Block *block, int_t i, int_t j, int_t k, int_t l)
{
	overrideBlockTexture = l;
	renderBlockByRenderType(block, i, j, k);
	overrideBlockTexture = -1;
}

void RenderBlocks::renderBlockAllFaces(Block *block, int_t i, int_t j, int_t k)
{
	renderAllFaces = true;
	renderBlockByRenderType(block, i, j, k);
	renderAllFaces = false;
}

bool RenderBlocks::renderBlockByRenderType(Block *block, int_t i, int_t j, int_t k)
{
	int_t l = block->getRenderType();
	block->setBlockBoundsBasedOnState(blockAccess, i, j, k);
	if (l == 0)
	{
		return renderStandardBlock(block, i, j, k);
	}
	if (l == 4)
	{
		return renderBlockFluids(block, i, j, k);
	}
	if (l == 13)
	{
		return renderBlockCactus(block, i, j, k);
	}
	if (l == 1)
	{
		return renderBlockReed(block, i, j, k);
	}
	if (l == 6)
	{
		return renderBlockCrops(block, i, j, k);
	}
	if (l == 2)
	{
		return renderBlockTorch(block, i, j, k);
	}
	if (l == 3)
	{
		return renderBlockFire(block, i, j, k);
	}
	if (l == 5)
	{
		return renderBlockRedstoneWire(block, i, j, k);
	}
	if (l == 8)
	{
		return renderBlockLadder(block, i, j, k);
	}
	if (l == 7)
	{
		return renderBlockDoor(block, i, j, k);
	}
	if (l == 9)
	{
		return renderBlockMinecartTrack((BlockRail *)block, i, j, k);
	}
	if (l == 10)
	{
		return renderBlockStairs(block, i, j, k);
	}
	if (l == 11)
	{
		return renderBlockFence(block, i, j, k);
	}
	if (l == 12)
	{
		return renderBlockLever(block, i, j, k);
	}
	if (l == 14)
	{
		return renderBlockBed(block, i, j, k);
	}
	if (l == 15)
	{
		return renderBlockRepeater(block, i, j, k);
	}
	if (l == 16)
	{
		return renderPistonBase(block, i, j, k, false);
	}
	if (l == 17)
	{
		return renderPistonExtension(block, i, j, k, true);
	}
	else
	{
		return false;
	}
}

bool RenderBlocks::renderBlockBed(Block *block, int_t i, int_t j, int_t k)
{
	Tessellator *tessellator = &Tessellator::instance;
	int_t l = blockAccess->getBlockMetadata(i, j, k);
	int_t i1 = BlockBed::getDirectionFromMetadata(l);
	bool flag = BlockBed::isBlockFootOfBed(l);
	float f = 0.5f;
	float f1 = 1.0f;
	float f2 = 0.8f;
	float f3 = 0.6f;
	float f4 = f1;
	float f5 = f1;
	float f6 = f1;
	float f7 = f;
	float f8 = f2;
	float f9 = f3;
	float f10 = f;
	float f11 = f2;
	float f12 = f3;
	float f13 = f;
	float f14 = f2;
	float f15 = f3;
	float f16 = block->getBlockBrightness(blockAccess, i, j, k);
	tessellator->setColorOpaque_F(f7 * f16, f10 * f16, f13 * f16);
	int_t j1 = block->getBlockTexture(blockAccess, i, j, k, 0);
	int_t k1 = (j1 & 0xf) << 4;
	int_t l1 = j1 & 0xf0;
	double d = (float)k1 / 256.0f;
	double d2 = ((double)(k1 + 16) - 0.01) / 256.0;
	double d4 = (float)l1 / 256.0f;
	double d6 = ((double)(l1 + 16) - 0.01) / 256.0;
	double d8 = (double)i + block->minX;
	double d10 = (double)i + block->maxX;
	double d12 = (double)j + block->minY + 0.1875;
	double d14 = (double)k + block->minZ;
	double d16 = (double)k + block->maxZ;
	tessellator->addVertexWithUV(d8, d12, d16, d, d6);
	tessellator->addVertexWithUV(d8, d12, d14, d, d4);
	tessellator->addVertexWithUV(d10, d12, d14, d2, d4);
	tessellator->addVertexWithUV(d10, d12, d16, d2, d6);
	float f17 = block->getBlockBrightness(blockAccess, i, j + 1, k);
	tessellator->setColorOpaque_F(f4 * f17, f5 * f17, f6 * f17);
	j1 = block->getBlockTexture(blockAccess, i, j, k, 1);
	k1 = (j1 & 0xf) << 4;
	d = j1 & 0xf0;
	double d1 = (float)k1 / 256.0f;
	double d3 = ((double)(k1 + 16) - 0.01) / 256.0;
	double d5 = (float)d / 256.0f;
	double d7 = ((double)(d + 16) - 0.01) / 256.0;
	double d9 = d1;
	double d11 = d3;
	double d13 = d5;
	double d15 = d5;
	double d17 = d1;
	double d18 = d3;
	double d19 = d7;
	double d20 = d7;
	if (i1 == 0)
	{
		d11 = d1;
		d13 = d7;
		d17 = d3;
		d20 = d5;
	}
	else if (i1 == 2)
	{
		d9 = d3;
		d15 = d7;
		d18 = d1;
		d19 = d5;
	}
	else if (i1 == 3)
	{
		d9 = d3;
		d15 = d7;
		d18 = d1;
		d19 = d5;
		d11 = d1;
		d13 = d7;
		d17 = d3;
		d20 = d5;
	}
	double d21 = (double)i + block->minX;
	double d22 = (double)i + block->maxX;
	double d23 = (double)j + block->maxY;
	double d24 = (double)k + block->minZ;
	double d25 = (double)k + block->maxZ;
	tessellator->addVertexWithUV(d22, d23, d25, d17, d19);
	tessellator->addVertexWithUV(d22, d23, d24, d9, d13);
	tessellator->addVertexWithUV(d21, d23, d24, d11, d15);
	tessellator->addVertexWithUV(d21, d23, d25, d18, d20);
	int_t f17a = ModelBed::field_22280_a[i1];
	if (flag)
	{
		f17a = ModelBed::field_22280_a[ModelBed::field_22279_b[i1]];
	}
	j1 = 4;
	switch (i1)
	{
	case 0:
		j1 = 5;
		break;
	case 3:
		j1 = 2;
		break;
	case 1:
		j1 = 3;
		break;
	}
	if (f17a != 2 && (renderAllFaces || block->shouldSideBeRendered(blockAccess, i, j, k - 1, 2)))
	{
		float f18 = block->getBlockBrightness(blockAccess, i, j, k - 1);
		if (block->minZ > 0.0)
		{
			f18 = f16;
		}
		tessellator->setColorOpaque_F(f8 * f18, f11 * f18, f14 * f18);
		flipTexture = j1 == 2;
		renderEastFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 2));
	}
	if (f17a != 3 && (renderAllFaces || block->shouldSideBeRendered(blockAccess, i, j, k + 1, 3)))
	{
		float f19 = block->getBlockBrightness(blockAccess, i, j, k + 1);
		if (block->maxZ < 1.0)
		{
			f19 = f16;
		}
		tessellator->setColorOpaque_F(f8 * f19, f11 * f19, f14 * f19);
		flipTexture = j1 == 3;
		renderWestFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 3));
	}
	if (f17a != 4 && (renderAllFaces || block->shouldSideBeRendered(blockAccess, i - 1, j, k, 4)))
	{
		float f20 = block->getBlockBrightness(blockAccess, i - 1, j, k);
		if (block->minX > 0.0)
		{
			f20 = f16;
		}
		tessellator->setColorOpaque_F(f9 * f20, f12 * f20, f15 * f20);
		flipTexture = j1 == 4;
		renderNorthFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 4));
	}
	if (f17a != 5 && (renderAllFaces || block->shouldSideBeRendered(blockAccess, i + 1, j, k, 5)))
	{
		float f21 = block->getBlockBrightness(blockAccess, i + 1, j, k);
		if (block->maxX < 1.0)
		{
			f21 = f16;
		}
		tessellator->setColorOpaque_F(f9 * f21, f12 * f21, f15 * f21);
		flipTexture = j1 == 5;
		renderSouthFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 5));
	}
	flipTexture = false;
	return true;
}

bool RenderBlocks::renderBlockTorch(Block *block, int_t i, int_t j, int_t k)
{
	int_t l = blockAccess->getBlockMetadata(i, j, k);
	Tessellator *tessellator = &Tessellator::instance;
	float f = block->getBlockBrightness(blockAccess, i, j, k);
	if (Block::lightValue[block->blockID] > 0)
	{
		f = 1.0f;
	}
	tessellator->setColorOpaque_F(f, f, f);
	double d = 0.40000000596046448;
	double d1 = 0.5 - d;
	double d2 = 0.20000000298023224;
	if (l == 1)
	{
		renderTorchAtAngle(block, (double)i - d1, (double)j + d2, k, -d, 0.0);
	}
	else if (l == 2)
	{
		renderTorchAtAngle(block, (double)i + d1, (double)j + d2, k, d, 0.0);
	}
	else if (l == 3)
	{
		renderTorchAtAngle(block, i, (double)j + d2, (double)k - d1, 0.0, -d);
	}
	else if (l == 4)
	{
		renderTorchAtAngle(block, i, (double)j + d2, (double)k + d1, 0.0, d);
	}
	else
	{
		renderTorchAtAngle(block, i, j, k, 0.0, 0.0);
	}
	return true;
}

bool RenderBlocks::renderBlockRepeater(Block *block, int_t i, int_t j, int_t k)
{
	int_t l = blockAccess->getBlockMetadata(i, j, k);
	int_t i1 = l & 3;
	int_t j1 = (l & 0xc) >> 2;
	renderStandardBlock(block, i, j, k);
	Tessellator *tessellator = &Tessellator::instance;
	float f = block->getBlockBrightness(blockAccess, i, j, k);
	if (Block::lightValue[block->blockID] > 0)
	{
		f = (f + 1.0f) * 0.5f;
	}
	tessellator->setColorOpaque_F(f, f, f);
	double d = -0.1875;
	double d1 = 0.0;
	double d2 = 0.0;
	double d3 = 0.0;
	double d4 = 0.0;
	switch (i1)
	{
	case 0:
		d4 = -0.3125;
		d2 = BlockRedstoneRepeater::isPowered[j1];
		break;
	case 2:
		d4 = 0.3125;
		d2 = -BlockRedstoneRepeater::isPowered[j1];
		break;
	case 3:
		d3 = -0.3125;
		d1 = BlockRedstoneRepeater::isPowered[j1];
		break;
	case 1:
		d3 = 0.3125;
		d1 = -BlockRedstoneRepeater::isPowered[j1];
		break;
	}
	renderTorchAtAngle(block, (double)i + d1, (double)j + d, (double)k + d2, 0.0, 0.0);
	renderTorchAtAngle(block, (double)i + d3, (double)j + d, (double)k + d4, 0.0, 0.0);
	int_t k1 = block->getBlockTextureFromSide(1);
	int_t l1 = (k1 & 0xf) << 4;
	int_t i2 = k1 & 0xf0;
	double d5 = (float)l1 / 256.0f;
	double d6 = ((float)l1 + 15.99f) / 256.0f;
	double d7 = (float)i2 / 256.0f;
	double d8 = ((float)i2 + 15.99f) / 256.0f;
	float f1 = 0.125f;
	float f2 = i + 1;
	float f3 = i + 1;
	float f4 = i + 0;
	float f5 = i + 0;
	float f6 = k + 0;
	float f7 = k + 1;
	float f8 = k + 1;
	float f9 = k + 0;
	float f10 = (float)j + f1;
	if (i1 == 2)
	{
		f2 = f3 = i + 0;
		f4 = f5 = i + 1;
		f6 = f9 = k + 1;
		f7 = f8 = k + 0;
	}
	else if (i1 == 3)
	{
		f2 = f5 = i + 0;
		f3 = f4 = i + 1;
		f6 = f7 = k + 0;
		f8 = f9 = k + 1;
	}
	else if (i1 == 1)
	{
		f2 = f5 = i + 1;
		f3 = f4 = i + 0;
		f6 = f7 = k + 1;
		f8 = f9 = k + 0;
	}
	tessellator->addVertexWithUV(f5, f10, f9, d5, d7);
	tessellator->addVertexWithUV(f4, f10, f8, d5, d8);
	tessellator->addVertexWithUV(f3, f10, f7, d6, d8);
	tessellator->addVertexWithUV(f2, f10, f6, d6, d7);
	return true;
}

void RenderBlocks::renderPistonBaseAllFaces(Block *block, int_t i, int_t j, int_t k)
{
	renderAllFaces = true;
	renderPistonBase(block, i, j, k, true);
	renderAllFaces = false;
}

bool RenderBlocks::renderPistonBase(Block *block, int_t i, int_t j, int_t k, bool flag)
{
	int_t l = blockAccess->getBlockMetadata(i, j, k);
	bool flag1 = flag || (l & 8) != 0;
	int_t i1 = BlockPistonBase::getPistonOrientation(l);
	if (flag1)
	{
		switch (i1)
		{
		case 0:
			eastFaceRotation = 3;
			westFaceRotation = 3;
			southFaceRotation = 3;
			northFaceRotation = 3;
			block->setBlockBounds(0.0f, 0.25f, 0.0f, 1.0f, 1.0f, 1.0f);
			break;
		case 1:
			block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 0.75f, 1.0f);
			break;
		case 2:
			southFaceRotation = 1;
			northFaceRotation = 2;
			block->setBlockBounds(0.0f, 0.0f, 0.25f, 1.0f, 1.0f, 1.0f);
			break;
		case 3:
			southFaceRotation = 2;
			northFaceRotation = 1;
			topFaceRotation = 3;
			bottomFaceRotation = 3;
			block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.75f);
			break;
		case 4:
			eastFaceRotation = 1;
			westFaceRotation = 2;
			topFaceRotation = 2;
			bottomFaceRotation = 1;
			block->setBlockBounds(0.25f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
			break;
		case 5:
			eastFaceRotation = 2;
			westFaceRotation = 1;
			topFaceRotation = 1;
			bottomFaceRotation = 2;
			block->setBlockBounds(0.0f, 0.0f, 0.0f, 0.75f, 1.0f, 1.0f);
			break;
		}
		renderStandardBlock(block, i, j, k);
		eastFaceRotation = 0;
		westFaceRotation = 0;
		southFaceRotation = 0;
		northFaceRotation = 0;
		topFaceRotation = 0;
		bottomFaceRotation = 0;
		block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	}
	else
	{
		switch (i1)
		{
		case 0:
			eastFaceRotation = 3;
			westFaceRotation = 3;
			southFaceRotation = 3;
			northFaceRotation = 3;
			break;
		case 2:
			southFaceRotation = 1;
			northFaceRotation = 2;
			break;
		case 3:
			southFaceRotation = 2;
			northFaceRotation = 1;
			topFaceRotation = 3;
			bottomFaceRotation = 3;
			break;
		case 4:
			eastFaceRotation = 1;
			westFaceRotation = 2;
			topFaceRotation = 2;
			bottomFaceRotation = 1;
			break;
		case 5:
			eastFaceRotation = 2;
			westFaceRotation = 1;
			topFaceRotation = 1;
			bottomFaceRotation = 2;
			break;
		}
		renderStandardBlock(block, i, j, k);
		eastFaceRotation = 0;
		westFaceRotation = 0;
		southFaceRotation = 0;
		northFaceRotation = 0;
		topFaceRotation = 0;
		bottomFaceRotation = 0;
	}
	return true;
}

void RenderBlocks::renderPistonArmX(double d, double d1, double d2, double d3, double d4, double d5, float f, double d6)
{
	int_t i = 108;
	if (overrideBlockTexture >= 0)
	{
		i = overrideBlockTexture;
	}
	int_t j = (i & 0xf) << 4;
	int_t k = i & 0xf0;
	Tessellator *tessellator = &Tessellator::instance;
	double d7 = (float)(j + 0) / 256.0f;
	double d8 = (float)(k + 0) / 256.0f;
	double d9 = (((double)j + d6) - 0.01) / 256.0;
	double d10 = ((double)((float)k + 4.0f) - 0.01) / 256.0;
	tessellator->setColorOpaque_F(f, f, f);
	tessellator->addVertexWithUV(d, d3, d4, d9, d8);
	tessellator->addVertexWithUV(d, d2, d4, d7, d8);
	tessellator->addVertexWithUV(d1, d2, d5, d7, d10);
	tessellator->addVertexWithUV(d1, d3, d5, d9, d10);
}

void RenderBlocks::renderPistonArmY(double d, double d1, double d2, double d3, double d4, double d5, float f, double d6)
{
	int_t i = 108;
	if (overrideBlockTexture >= 0)
	{
		i = overrideBlockTexture;
	}
	int_t j = (i & 0xf) << 4;
	int_t k = i & 0xf0;
	Tessellator *tessellator = &Tessellator::instance;
	double d7 = (float)(j + 0) / 256.0f;
	double d8 = (float)(k + 0) / 256.0f;
	double d9 = (((double)j + d6) - 0.01) / 256.0;
	double d10 = ((double)((float)k + 4.0f) - 0.01) / 256.0;
	tessellator->setColorOpaque_F(f, f, f);
	tessellator->addVertexWithUV(d, d2, d5, d9, d8);
	tessellator->addVertexWithUV(d, d2, d4, d7, d8);
	tessellator->addVertexWithUV(d1, d3, d4, d7, d10);
	tessellator->addVertexWithUV(d1, d3, d5, d9, d10);
}

void RenderBlocks::renderPistonArmZ(double d, double d1, double d2, double d3, double d4, double d5, float f, double d6)
{
	int_t i = 108;
	if (overrideBlockTexture >= 0)
	{
		i = overrideBlockTexture;
	}
	int_t j = (i & 0xf) << 4;
	int_t k = i & 0xf0;
	Tessellator *tessellator = &Tessellator::instance;
	double d7 = (float)(j + 0) / 256.0f;
	double d8 = (float)(k + 0) / 256.0f;
	double d9 = (((double)j + d6) - 0.01) / 256.0;
	double d10 = ((double)((float)k + 4.0f) - 0.01) / 256.0;
	tessellator->setColorOpaque_F(f, f, f);
	tessellator->addVertexWithUV(d1, d2, d4, d9, d8);
	tessellator->addVertexWithUV(d, d2, d4, d7, d8);
	tessellator->addVertexWithUV(d, d3, d5, d7, d10);
	tessellator->addVertexWithUV(d1, d3, d5, d9, d10);
}

void RenderBlocks::renderPistonExtensionAllFaces(Block *block, int_t i, int_t j, int_t k, bool flag)
{
	renderAllFaces = true;
	renderPistonExtension(block, i, j, k, flag);
	renderAllFaces = false;
}

bool RenderBlocks::renderPistonExtension(Block *block, int_t i, int_t j, int_t k, bool flag)
{
	int_t l = blockAccess->getBlockMetadata(i, j, k);
	int_t i1 = BlockPistonExtension::getPistonExtensionType(l); // orientation from metadata low 3 bits
	float f = block->getBlockBrightness(blockAccess, i, j, k);
	float f1 = flag ? 1.0f : 0.5f;
	double d = flag ? 16.0 : 8.0;
	switch (i1)
	{
	case 0:
		eastFaceRotation = 3;
		westFaceRotation = 3;
		southFaceRotation = 3;
		northFaceRotation = 3;
		block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 0.25f, 1.0f);
		renderStandardBlock(block, i, j, k);
		renderPistonArmX((float)i + 0.375f, (float)i + 0.625f, (float)j + 0.25f, (float)j + 0.25f + f1, (float)k + 0.625f, (float)k + 0.625f, f * 0.8f, d);
		renderPistonArmX((float)i + 0.625f, (float)i + 0.375f, (float)j + 0.25f, (float)j + 0.25f + f1, (float)k + 0.375f, (float)k + 0.375f, f * 0.8f, d);
		renderPistonArmX((float)i + 0.375f, (float)i + 0.375f, (float)j + 0.25f, (float)j + 0.25f + f1, (float)k + 0.375f, (float)k + 0.625f, f * 0.6f, d);
		renderPistonArmX((float)i + 0.625f, (float)i + 0.625f, (float)j + 0.25f, (float)j + 0.25f + f1, (float)k + 0.625f, (float)k + 0.375f, f * 0.6f, d);
		break;
	case 1:
		block->setBlockBounds(0.0f, 0.75f, 0.0f, 1.0f, 1.0f, 1.0f);
		renderStandardBlock(block, i, j, k);
		renderPistonArmX((float)i + 0.375f, (float)i + 0.625f, (((float)j - 0.25f) + 1.0f) - f1, ((float)j - 0.25f) + 1.0f, (float)k + 0.625f, (float)k + 0.625f, f * 0.8f, d);
		renderPistonArmX((float)i + 0.625f, (float)i + 0.375f, (((float)j - 0.25f) + 1.0f) - f1, ((float)j - 0.25f) + 1.0f, (float)k + 0.375f, (float)k + 0.375f, f * 0.8f, d);
		renderPistonArmX((float)i + 0.375f, (float)i + 0.375f, (((float)j - 0.25f) + 1.0f) - f1, ((float)j - 0.25f) + 1.0f, (float)k + 0.375f, (float)k + 0.625f, f * 0.6f, d);
		renderPistonArmX((float)i + 0.625f, (float)i + 0.625f, (((float)j - 0.25f) + 1.0f) - f1, ((float)j - 0.25f) + 1.0f, (float)k + 0.625f, (float)k + 0.375f, f * 0.6f, d);
		break;
	case 2:
		southFaceRotation = 1;
		northFaceRotation = 2;
		block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.25f);
		renderStandardBlock(block, i, j, k);
		renderPistonArmY((float)i + 0.375f, (float)i + 0.375f, (float)j + 0.625f, (float)j + 0.375f, (float)k + 0.25f, (float)k + 0.25f + f1, f * 0.6f, d);
		renderPistonArmY((float)i + 0.625f, (float)i + 0.625f, (float)j + 0.375f, (float)j + 0.625f, (float)k + 0.25f, (float)k + 0.25f + f1, f * 0.6f, d);
		renderPistonArmY((float)i + 0.375f, (float)i + 0.625f, (float)j + 0.375f, (float)j + 0.375f, (float)k + 0.25f, (float)k + 0.25f + f1, f * 0.5f, d);
		renderPistonArmY((float)i + 0.625f, (float)i + 0.375f, (float)j + 0.625f, (float)j + 0.625f, (float)k + 0.25f, (float)k + 0.25f + f1, f, d);
		break;
	case 3:
		southFaceRotation = 2;
		northFaceRotation = 1;
		topFaceRotation = 3;
		bottomFaceRotation = 3;
		block->setBlockBounds(0.0f, 0.0f, 0.75f, 1.0f, 1.0f, 1.0f);
		renderStandardBlock(block, i, j, k);
		renderPistonArmY((float)i + 0.375f, (float)i + 0.375f, (float)j + 0.625f, (float)j + 0.375f, (((float)k - 0.25f) + 1.0f) - f1, ((float)k - 0.25f) + 1.0f, f * 0.6f, d);
		renderPistonArmY((float)i + 0.625f, (float)i + 0.625f, (float)j + 0.375f, (float)j + 0.625f, (((float)k - 0.25f) + 1.0f) - f1, ((float)k - 0.25f) + 1.0f, f * 0.6f, d);
		renderPistonArmY((float)i + 0.375f, (float)i + 0.625f, (float)j + 0.375f, (float)j + 0.375f, (((float)k - 0.25f) + 1.0f) - f1, ((float)k - 0.25f) + 1.0f, f * 0.5f, d);
		renderPistonArmY((float)i + 0.625f, (float)i + 0.375f, (float)j + 0.625f, (float)j + 0.625f, (((float)k - 0.25f) + 1.0f) - f1, ((float)k - 0.25f) + 1.0f, f, d);
		break;
	case 4:
		eastFaceRotation = 1;
		westFaceRotation = 2;
		topFaceRotation = 2;
		bottomFaceRotation = 1;
		block->setBlockBounds(0.0f, 0.0f, 0.0f, 0.25f, 1.0f, 1.0f);
		renderStandardBlock(block, i, j, k);
		renderPistonArmZ((float)i + 0.25f, (float)i + 0.25f + f1, (float)j + 0.375f, (float)j + 0.375f, (float)k + 0.625f, (float)k + 0.375f, f * 0.5f, d);
		renderPistonArmZ((float)i + 0.25f, (float)i + 0.25f + f1, (float)j + 0.625f, (float)j + 0.625f, (float)k + 0.375f, (float)k + 0.625f, f, d);
		renderPistonArmZ((float)i + 0.25f, (float)i + 0.25f + f1, (float)j + 0.375f, (float)j + 0.625f, (float)k + 0.375f, (float)k + 0.375f, f * 0.6f, d);
		renderPistonArmZ((float)i + 0.25f, (float)i + 0.25f + f1, (float)j + 0.625f, (float)j + 0.375f, (float)k + 0.625f, (float)k + 0.625f, f * 0.6f, d);
		break;
	case 5:
		eastFaceRotation = 2;
		westFaceRotation = 1;
		topFaceRotation = 1;
		bottomFaceRotation = 2;
		block->setBlockBounds(0.75f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
		renderStandardBlock(block, i, j, k);
		renderPistonArmZ((((float)i - 0.25f) + 1.0f) - f1, ((float)i - 0.25f) + 1.0f, (float)j + 0.375f, (float)j + 0.375f, (float)k + 0.625f, (float)k + 0.375f, f * 0.5f, d);
		renderPistonArmZ((((float)i - 0.25f) + 1.0f) - f1, ((float)i - 0.25f) + 1.0f, (float)j + 0.625f, (float)j + 0.625f, (float)k + 0.375f, (float)k + 0.625f, f, d);
		renderPistonArmZ((((float)i - 0.25f) + 1.0f) - f1, ((float)i - 0.25f) + 1.0f, (float)j + 0.375f, (float)j + 0.625f, (float)k + 0.375f, (float)k + 0.375f, f * 0.6f, d);
		renderPistonArmZ((((float)i - 0.25f) + 1.0f) - f1, ((float)i - 0.25f) + 1.0f, (float)j + 0.625f, (float)j + 0.375f, (float)k + 0.625f, (float)k + 0.625f, f * 0.6f, d);
		break;
	}
	eastFaceRotation = 0;
	westFaceRotation = 0;
	southFaceRotation = 0;
	northFaceRotation = 0;
	topFaceRotation = 0;
	bottomFaceRotation = 0;
	block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	return true;
}

bool RenderBlocks::renderBlockLever(Block *block, int_t i, int_t j, int_t k)
{
	int_t l = blockAccess->getBlockMetadata(i, j, k);
	int_t i1 = l & 7;
	bool flag = (l & 8) > 0;
	Tessellator *tessellator = &Tessellator::instance;
	bool flag1 = overrideBlockTexture >= 0;
	if (!flag1)
	{
		overrideBlockTexture = Block::cobblestone->blockIndexInTexture;
	}
	float f = 0.25f;
	float f1 = 0.1875f;
	float f2 = 0.1875f;
	if (i1 == 5)
	{
		block->setBlockBounds(0.5f - f1, 0.0f, 0.5f - f, 0.5f + f1, f2, 0.5f + f);
	}
	else if (i1 == 6)
	{
		block->setBlockBounds(0.5f - f, 0.0f, 0.5f - f1, 0.5f + f, f2, 0.5f + f1);
	}
	else if (i1 == 4)
	{
		block->setBlockBounds(0.5f - f1, 0.5f - f, 1.0f - f2, 0.5f + f1, 0.5f + f, 1.0f);
	}
	else if (i1 == 3)
	{
		block->setBlockBounds(0.5f - f1, 0.5f - f, 0.0f, 0.5f + f1, 0.5f + f, f2);
	}
	else if (i1 == 2)
	{
		block->setBlockBounds(1.0f - f2, 0.5f - f, 0.5f - f1, 1.0f, 0.5f + f, 0.5f + f1);
	}
	else if (i1 == 1)
	{
		block->setBlockBounds(0.0f, 0.5f - f, 0.5f - f1, f2, 0.5f + f, 0.5f + f1);
	}
	renderStandardBlock(block, i, j, k);
	if (!flag1)
	{
		overrideBlockTexture = -1;
	}
	float f3 = block->getBlockBrightness(blockAccess, i, j, k);
	if (Block::lightValue[block->blockID] > 0)
	{
		f3 = 1.0f;
	}
	tessellator->setColorOpaque_F(f3, f3, f3);
	int_t j1 = block->getBlockTextureFromSide(0);
	if (overrideBlockTexture >= 0)
	{
		j1 = overrideBlockTexture;
	}
	int_t k1 = (j1 & 0xf) << 4;
	int_t l1 = j1 & 0xf0;
	float f4 = (float)k1 / 256.0f;
	float f5 = ((float)k1 + 15.99f) / 256.0f;
	float f6 = (float)l1 / 256.0f;
	float f7 = ((float)l1 + 15.99f) / 256.0f;
	Vec3D *avec3d[8];
	float f8 = 0.0625f;
	float f9 = 0.0625f;
	float f10 = 0.625f;
	avec3d[0] = Vec3D::createVector(-f8, 0.0, -f9);
	avec3d[1] = Vec3D::createVector(f8, 0.0, -f9);
	avec3d[2] = Vec3D::createVector(f8, 0.0, f9);
	avec3d[3] = Vec3D::createVector(-f8, 0.0, f9);
	avec3d[4] = Vec3D::createVector(-f8, f10, -f9);
	avec3d[5] = Vec3D::createVector(f8, f10, -f9);
	avec3d[6] = Vec3D::createVector(f8, f10, f9);
	avec3d[7] = Vec3D::createVector(-f8, f10, f9);
	for (int_t i2 = 0; i2 < 8; i2++)
	{
		if (flag)
		{
			avec3d[i2]->zCoord -= 0.0625;
			avec3d[i2]->rotateAroundX(0.6981317f);
		}
		else
		{
			avec3d[i2]->zCoord += 0.0625;
			avec3d[i2]->rotateAroundX(-0.6981317f);
		}
		if (i1 == 6)
		{
			avec3d[i2]->rotateAroundY(1.570796f);
		}
		if (i1 < 5)
		{
			avec3d[i2]->yCoord -= 0.375;
			avec3d[i2]->rotateAroundX(1.570796f);
			if (i1 == 4)
			{
				avec3d[i2]->rotateAroundY(0.0f);
			}
			if (i1 == 3)
			{
				avec3d[i2]->rotateAroundY(3.141593f);
			}
			if (i1 == 2)
			{
				avec3d[i2]->rotateAroundY(1.570796f);
			}
			if (i1 == 1)
			{
				avec3d[i2]->rotateAroundY(-1.570796f);
			}
			avec3d[i2]->xCoord += (double)i + 0.5;
			avec3d[i2]->yCoord += (float)j + 0.5f;
			avec3d[i2]->zCoord += (double)k + 0.5;
		}
		else
		{
			avec3d[i2]->xCoord += (double)i + 0.5;
			avec3d[i2]->yCoord += (float)j + 0.125f;
			avec3d[i2]->zCoord += (double)k + 0.5;
		}
	}

	Vec3D *vec3d = nullptr;
	Vec3D *vec3d1 = nullptr;
	Vec3D *vec3d2 = nullptr;
	Vec3D *vec3d3 = nullptr;
	for (int_t j2 = 0; j2 < 6; j2++)
	{
		if (j2 == 0)
		{
			f4 = (float)(k1 + 7) / 256.0f;
			f5 = ((float)(k1 + 9) - 0.01f) / 256.0f;
			f6 = (float)(l1 + 6) / 256.0f;
			f7 = ((float)(l1 + 8) - 0.01f) / 256.0f;
		}
		else if (j2 == 2)
		{
			f4 = (float)(k1 + 7) / 256.0f;
			f5 = ((float)(k1 + 9) - 0.01f) / 256.0f;
			f6 = (float)(l1 + 6) / 256.0f;
			f7 = ((float)(l1 + 16) - 0.01f) / 256.0f;
		}
		if (j2 == 0)
		{
			vec3d = avec3d[0];
			vec3d1 = avec3d[1];
			vec3d2 = avec3d[2];
			vec3d3 = avec3d[3];
		}
		else if (j2 == 1)
		{
			vec3d = avec3d[7];
			vec3d1 = avec3d[6];
			vec3d2 = avec3d[5];
			vec3d3 = avec3d[4];
		}
		else if (j2 == 2)
		{
			vec3d = avec3d[1];
			vec3d1 = avec3d[0];
			vec3d2 = avec3d[4];
			vec3d3 = avec3d[5];
		}
		else if (j2 == 3)
		{
			vec3d = avec3d[2];
			vec3d1 = avec3d[1];
			vec3d2 = avec3d[5];
			vec3d3 = avec3d[6];
		}
		else if (j2 == 4)
		{
			vec3d = avec3d[3];
			vec3d1 = avec3d[2];
			vec3d2 = avec3d[6];
			vec3d3 = avec3d[7];
		}
		else if (j2 == 5)
		{
			vec3d = avec3d[0];
			vec3d1 = avec3d[3];
			vec3d2 = avec3d[7];
			vec3d3 = avec3d[4];
		}
		tessellator->addVertexWithUV(vec3d->xCoord, vec3d->yCoord, vec3d->zCoord, f4, f7);
		tessellator->addVertexWithUV(vec3d1->xCoord, vec3d1->yCoord, vec3d1->zCoord, f5, f7);
		tessellator->addVertexWithUV(vec3d2->xCoord, vec3d2->yCoord, vec3d2->zCoord, f5, f6);
		tessellator->addVertexWithUV(vec3d3->xCoord, vec3d3->yCoord, vec3d3->zCoord, f4, f6);
	}

	return true;
}

bool RenderBlocks::renderBlockFire(Block *block, int_t i, int_t j, int_t k)
{
	Tessellator *tessellator = &Tessellator::instance;
	int_t l = block->getBlockTextureFromSide(0);
	if (overrideBlockTexture >= 0)
	{
		l = overrideBlockTexture;
	}
	float f = block->getBlockBrightness(blockAccess, i, j, k);
	tessellator->setColorOpaque_F(f, f, f);
	int_t i1 = (l & 0xf) << 4;
	int_t j1 = l & 0xf0;
	double d = (float)i1 / 256.0f;
	double d2 = ((float)i1 + 15.99f) / 256.0f;
	double d4 = (float)j1 / 256.0f;
	double d6 = ((float)j1 + 15.99f) / 256.0f;
	float f1 = 1.4f;
	if (blockAccess->isBlockNormalCube(i, j - 1, k) || ((BlockFire*)Block::fire)->canBlockCatchFire(blockAccess, i, j - 1, k))
	{
		double d8 = (double)i + 0.5 + 0.20000000000000001;
		double d9 = ((double)i + 0.5) - 0.20000000000000001;
		double d12 = (double)k + 0.5 + 0.20000000000000001;
		double d14 = ((double)k + 0.5) - 0.20000000000000001;
		double d16 = ((double)i + 0.5) - 0.29999999999999999;
		double d18 = (double)i + 0.5 + 0.29999999999999999;
		double d20 = ((double)k + 0.5) - 0.29999999999999999;
		double d22 = (double)k + 0.5 + 0.29999999999999999;
		tessellator->addVertexWithUV(d16, (float)j + f1, k + 1, d2, d4);
		tessellator->addVertexWithUV(d8, j + 0, k + 1, d2, d6);
		tessellator->addVertexWithUV(d8, j + 0, k + 0, d, d6);
		tessellator->addVertexWithUV(d16, (float)j + f1, k + 0, d, d4);
		tessellator->addVertexWithUV(d18, (float)j + f1, k + 0, d2, d4);
		tessellator->addVertexWithUV(d9, j + 0, k + 0, d2, d6);
		tessellator->addVertexWithUV(d9, j + 0, k + 1, d, d6);
		tessellator->addVertexWithUV(d18, (float)j + f1, k + 1, d, d4);
		d = (float)i1 / 256.0f;
		d2 = ((float)i1 + 15.99f) / 256.0f;
		d4 = (float)(j1 + 16) / 256.0f;
		d6 = ((float)j1 + 15.99f + 16.0f) / 256.0f;
		tessellator->addVertexWithUV(i + 1, (float)j + f1, d22, d2, d4);
		tessellator->addVertexWithUV(i + 1, j + 0, d14, d2, d6);
		tessellator->addVertexWithUV(i + 0, j + 0, d14, d, d6);
		tessellator->addVertexWithUV(i + 0, (float)j + f1, d22, d, d4);
		tessellator->addVertexWithUV(i + 0, (float)j + f1, d20, d2, d4);
		tessellator->addVertexWithUV(i + 0, j + 0, d12, d2, d6);
		tessellator->addVertexWithUV(i + 1, j + 0, d12, d, d6);
		tessellator->addVertexWithUV(i + 1, (float)j + f1, d20, d, d4);
		d8 = ((double)i + 0.5) - 0.5;
		d9 = (double)i + 0.5 + 0.5;
		d12 = ((double)k + 0.5) - 0.5;
		d14 = (double)k + 0.5 + 0.5;
 
		d18 = (double)i + 0.5 + 0.40000000000000002;
		d20 = ((double)k + 0.5) - 0.40000000000000002;
		d22 = (double)k + 0.5 + 0.40000000000000002;
		tessellator->addVertexWithUV(d16, (float)j + f1, k + 0, d, d4);
		tessellator->addVertexWithUV(d8, j + 0, k + 0, d, d6);
		tessellator->addVertexWithUV(d8, j + 0, k + 1, d2, d6);
		tessellator->addVertexWithUV(d16, (float)j + f1, k + 1, d2, d4);
		tessellator->addVertexWithUV(d18, (float)j + f1, k + 1, d, d4);
		tessellator->addVertexWithUV(d9, j + 0, k + 1, d, d6);
		tessellator->addVertexWithUV(d9, j + 0, k + 0, d2, d6);
		tessellator->addVertexWithUV(d18, (float)j + f1, k + 0, d2, d4);
		d = (float)i1 / 256.0f;
		d2 = ((float)i1 + 15.99f) / 256.0f;
		d4 = (float)j1 / 256.0f;
		d6 = ((float)j1 + 15.99f) / 256.0f;
		tessellator->addVertexWithUV(i + 0, (float)j + f1, d22, d, d4);
		tessellator->addVertexWithUV(i + 0, j + 0, d14, d, d6);
		tessellator->addVertexWithUV(i + 1, j + 0, d14, d2, d6);
		tessellator->addVertexWithUV(i + 1, (float)j + f1, d22, d2, d4);
		tessellator->addVertexWithUV(i + 1, (float)j + f1, d20, d, d4);
		tessellator->addVertexWithUV(i + 1, j + 0, d12, d, d6);
		tessellator->addVertexWithUV(i + 0, j + 0, d12, d2, d6);
		tessellator->addVertexWithUV(i + 0, (float)j + f1, d20, d2, d4);
	}
	else
	{
		float f3 = 0.2f;
		float f4 = 0.0625f;
		if ((i + j + k & 1) == 1)
		{
			d = (float)i1 / 256.0f;
			d2 = ((float)i1 + 15.99f) / 256.0f;
			d4 = (float)(j1 + 16) / 256.0f;
			d6 = ((float)j1 + 15.99f + 16.0f) / 256.0f;
		}
		if ((i / 2 + j / 2 + k / 2 & 1) == 1)
		{
			double d10 = d2;
			d2 = d;
			d = d10;
		}
		if (((BlockFire*)Block::fire)->canBlockCatchFire(blockAccess, i - 1, j, k))
		{
			tessellator->addVertexWithUV((float)i + f3, (float)j + f1 + f4, k + 1, d2, d4);
			tessellator->addVertexWithUV(i + 0, (float)(j + 0) + f4, k + 1, d2, d6);
			tessellator->addVertexWithUV(i + 0, (float)(j + 0) + f4, k + 0, d, d6);
			tessellator->addVertexWithUV((float)i + f3, (float)j + f1 + f4, k + 0, d, d4);
			tessellator->addVertexWithUV((float)i + f3, (float)j + f1 + f4, k + 0, d, d4);
			tessellator->addVertexWithUV(i + 0, (float)(j + 0) + f4, k + 0, d, d6);
			tessellator->addVertexWithUV(i + 0, (float)(j + 0) + f4, k + 1, d2, d6);
			tessellator->addVertexWithUV((float)i + f3, (float)j + f1 + f4, k + 1, d2, d4);
		}
		if (((BlockFire*)Block::fire)->canBlockCatchFire(blockAccess, i + 1, j, k))
		{
			tessellator->addVertexWithUV((float)(i + 1) - f3, (float)j + f1 + f4, k + 0, d, d4);
			tessellator->addVertexWithUV((i + 1) - 0, (float)(j + 0) + f4, k + 0, d, d6);
			tessellator->addVertexWithUV((i + 1) - 0, (float)(j + 0) + f4, k + 1, d2, d6);
			tessellator->addVertexWithUV((float)(i + 1) - f3, (float)j + f1 + f4, k + 1, d2, d4);
			tessellator->addVertexWithUV((float)(i + 1) - f3, (float)j + f1 + f4, k + 1, d2, d4);
			tessellator->addVertexWithUV((i + 1) - 0, (float)(j + 0) + f4, k + 1, d2, d6);
			tessellator->addVertexWithUV((i + 1) - 0, (float)(j + 0) + f4, k + 0, d, d6);
			tessellator->addVertexWithUV((float)(i + 1) - f3, (float)j + f1 + f4, k + 0, d, d4);
		}
		if (((BlockFire*)Block::fire)->canBlockCatchFire(blockAccess, i, j, k - 1))
		{
			tessellator->addVertexWithUV(i + 0, (float)j + f1 + f4, (float)k + f3, d2, d4);
			tessellator->addVertexWithUV(i + 0, (float)(j + 0) + f4, k + 0, d2, d6);
			tessellator->addVertexWithUV(i + 1, (float)(j + 0) + f4, k + 0, d, d6);
			tessellator->addVertexWithUV(i + 1, (float)j + f1 + f4, (float)k + f3, d, d4);
			tessellator->addVertexWithUV(i + 1, (float)j + f1 + f4, (float)k + f3, d, d4);
			tessellator->addVertexWithUV(i + 1, (float)(j + 0) + f4, k + 0, d, d6);
			tessellator->addVertexWithUV(i + 0, (float)(j + 0) + f4, k + 0, d2, d6);
			tessellator->addVertexWithUV(i + 0, (float)j + f1 + f4, (float)k + f3, d2, d4);
		}
		if (((BlockFire*)Block::fire)->canBlockCatchFire(blockAccess, i, j, k + 1))
		{
			tessellator->addVertexWithUV(i + 1, (float)j + f1 + f4, (float)(k + 1) - f3, d, d4);
			tessellator->addVertexWithUV(i + 1, (float)(j + 0) + f4, (k + 1) - 0, d, d6);
			tessellator->addVertexWithUV(i + 0, (float)(j + 0) + f4, (k + 1) - 0, d2, d6);
			tessellator->addVertexWithUV(i + 0, (float)j + f1 + f4, (float)(k + 1) - f3, d2, d4);
			tessellator->addVertexWithUV(i + 0, (float)j + f1 + f4, (float)(k + 1) - f3, d2, d4);
			tessellator->addVertexWithUV(i + 0, (float)(j + 0) + f4, (k + 1) - 0, d2, d6);
			tessellator->addVertexWithUV(i + 1, (float)(j + 0) + f4, (k + 1) - 0, d, d6);
			tessellator->addVertexWithUV(i + 1, (float)j + f1 + f4, (float)(k + 1) - f3, d, d4);
		}
		if (((BlockFire*)Block::fire)->canBlockCatchFire(blockAccess, i, j + 1, k))
		{
			double d11 = (double)i + 0.5 + 0.5;
			double d13 = ((double)i + 0.5) - 0.5;
			double d15 = (double)k + 0.5 + 0.5;
			double d17 = ((double)k + 0.5) - 0.5;
			double d19 = ((double)i + 0.5) - 0.5;
			double d21 = (double)i + 0.5 + 0.5;
			double d23 = ((double)k + 0.5) - 0.5;
			double d24 = (double)k + 0.5 + 0.5;
			double d1 = (float)i1 / 256.0f;
			double d3 = ((float)i1 + 15.99f) / 256.0f;
			double d5 = (float)j1 / 256.0f;
			double d7 = ((float)j1 + 15.99f) / 256.0f;
			j++;
			float f2 = -0.2f;
			if ((i + j + k & 1) == 0)
			{
				tessellator->addVertexWithUV(d19, (float)j + f2, k + 0, d3, d5);
				tessellator->addVertexWithUV(d11, j + 0, k + 0, d3, d7);
				tessellator->addVertexWithUV(d11, j + 0, k + 1, d1, d7);
				tessellator->addVertexWithUV(d19, (float)j + f2, k + 1, d1, d5);
				d1 = (float)i1 / 256.0f;
				d3 = ((float)i1 + 15.99f) / 256.0f;
				d5 = (float)(j1 + 16) / 256.0f;
				d7 = ((float)j1 + 15.99f + 16.0f) / 256.0f;
				tessellator->addVertexWithUV(d21, (float)j + f2, k + 1, d3, d5);
				tessellator->addVertexWithUV(d13, j + 0, k + 1, d3, d7);
				tessellator->addVertexWithUV(d13, j + 0, k + 0, d1, d7);
				tessellator->addVertexWithUV(d21, (float)j + f2, k + 0, d1, d5);
			}
			else
			{
				tessellator->addVertexWithUV(i + 0, (float)j + f2, d24, d3, d5);
				tessellator->addVertexWithUV(i + 0, j + 0, d17, d3, d7);
				tessellator->addVertexWithUV(i + 1, j + 0, d17, d1, d7);
				tessellator->addVertexWithUV(i + 1, (float)j + f2, d24, d1, d5);
				d1 = (float)i1 / 256.0f;
				d3 = ((float)i1 + 15.99f) / 256.0f;
				d5 = (float)(j1 + 16) / 256.0f;
				d7 = ((float)j1 + 15.99f + 16.0f) / 256.0f;
				tessellator->addVertexWithUV(i + 1, (float)j + f2, d23, d3, d5);
				tessellator->addVertexWithUV(i + 1, j + 0, d15, d3, d7);
				tessellator->addVertexWithUV(i + 0, j + 0, d15, d1, d7);
				tessellator->addVertexWithUV(i + 0, (float)j + f2, d23, d1, d5);
			}
		}
	}
	return true;
}

bool RenderBlocks::renderBlockRedstoneWire(Block *block, int_t i, int_t j, int_t k)
{
	Tessellator *tessellator = &Tessellator::instance;
	int_t l = blockAccess->getBlockMetadata(i, j, k);
	int_t i1 = block->getBlockTextureFromSideAndMetadata(1, l);
	if (overrideBlockTexture >= 0)
	{
		i1 = overrideBlockTexture;
	}
	float f = block->getBlockBrightness(blockAccess, i, j, k);
	float f1 = (float)l / 15.0f;
	float f2 = f1 * 0.6f + 0.4f;
	if (l == 0)
	{
		f2 = 0.3f;
	}
	float f3 = f1 * f1 * 0.7f - 0.5f;
	float f4 = f1 * f1 * 0.6f - 0.7f;
	if (f3 < 0.0f)
	{
		f3 = 0.0f;
	}
	if (f4 < 0.0f)
	{
		f4 = 0.0f;
	}
	tessellator->setColorOpaque_F(f * f2, f * f3, f * f4);
	int_t j1 = (i1 & 0xf) << 4;
	int_t k1 = i1 & 0xf0;
	double d = (float)j1 / 256.0f;
	double d2 = ((float)j1 + 15.99f) / 256.0f;
	double d4 = (float)k1 / 256.0f;
	double d6 = ((float)k1 + 15.99f) / 256.0f;
	bool flag = BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i - 1, j, k, 1) ||
	            !blockAccess->isBlockNormalCube(i - 1, j, k) && BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i - 1, j - 1, k, -1);
	bool flag1 = BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i + 1, j, k, 3) ||
	             !blockAccess->isBlockNormalCube(i + 1, j, k) && BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i + 1, j - 1, k, -1);
	bool flag2 = BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i, j, k - 1, 2) ||
	             !blockAccess->isBlockNormalCube(i, j, k - 1) && BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i, j - 1, k - 1, -1);
	bool flag3 = BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i, j, k + 1, 0) ||
	             !blockAccess->isBlockNormalCube(i, j, k + 1) && BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i, j - 1, k + 1, -1);
	if (!blockAccess->isBlockNormalCube(i, j + 1, k))
	{
		if (blockAccess->isBlockNormalCube(i - 1, j, k) && BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i - 1, j + 1, k, -1))
		{
			flag = true;
		}
		if (blockAccess->isBlockNormalCube(i + 1, j, k) && BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i + 1, j + 1, k, -1))
		{
			flag1 = true;
		}
		if (blockAccess->isBlockNormalCube(i, j, k - 1) && BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i, j + 1, k - 1, -1))
		{
			flag2 = true;
		}
		if (blockAccess->isBlockNormalCube(i, j, k + 1) && BlockRedstoneWire::isPowerProviderOrWire(blockAccess, i, j + 1, k + 1, -1))
		{
			flag3 = true;
		}
	}
	float f5 = i + 0;
	float f6 = i + 1;
	float f7 = k + 0;
	float f8 = k + 1;
	byte_t byte0 = 0;
	if ((flag || flag1) && !flag2 && !flag3)
	{
		byte0 = 1;
	}
	if ((flag2 || flag3) && !flag1 && !flag)
	{
		byte0 = 2;
	}
	if (byte0 != 0)
	{
		d = (float)(j1 + 16) / 256.0f;
		d2 = ((float)(j1 + 16) + 15.99f) / 256.0f;
		d4 = (float)k1 / 256.0f;
		d6 = ((float)k1 + 15.99f) / 256.0f;
	}
	if (byte0 == 0)
	{
		if (flag1 || flag2 || flag3 || flag)
		{
			if (!flag)
			{
				f5 += 0.3125f;
			}
			if (!flag)
			{
				d += 0.01953125;
			}
			if (!flag1)
			{
				f6 -= 0.3125f;
			}
			if (!flag1)
			{
				d2 -= 0.01953125;
			}
			if (!flag2)
			{
				f7 += 0.3125f;
			}
			if (!flag2)
			{
				d4 += 0.01953125;
			}
			if (!flag3)
			{
				f8 -= 0.3125f;
			}
			if (!flag3)
			{
				d6 -= 0.01953125;
			}
		}
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f8, d2, d6);
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f7, d2, d4);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f7, d, d4);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f8, d, d6);
		tessellator->setColorOpaque_F(f, f, f);
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f8, d2, d6 + 0.0625);
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f7, d2, d4 + 0.0625);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f7, d, d4 + 0.0625);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f8, d, d6 + 0.0625);
	}
	else if (byte0 == 1)
	{
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f8, d2, d6);
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f7, d2, d4);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f7, d, d4);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f8, d, d6);
		tessellator->setColorOpaque_F(f, f, f);
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f8, d2, d6 + 0.0625);
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f7, d2, d4 + 0.0625);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f7, d, d4 + 0.0625);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f8, d, d6 + 0.0625);
	}
	else if (byte0 == 2)
	{
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f8, d2, d6);
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f7, d, d6);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f7, d, d4);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f8, d2, d4);
		tessellator->setColorOpaque_F(f, f, f);
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f8, d2, d6 + 0.0625);
		tessellator->addVertexWithUV(f6, (float)j + 0.015625f, f7, d, d6 + 0.0625);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f7, d, d4 + 0.0625);
		tessellator->addVertexWithUV(f5, (float)j + 0.015625f, f8, d2, d4 + 0.0625);
	}
	if (!blockAccess->isBlockNormalCube(i, j + 1, k))
	{
		double d1 = (float)(j1 + 16) / 256.0f;
		double d3 = ((float)(j1 + 16) + 15.99f) / 256.0f;
		double d5 = (float)k1 / 256.0f;
		double d7 = ((float)k1 + 15.99f) / 256.0f;
		if (blockAccess->isBlockNormalCube(i - 1, j, k) && blockAccess->getBlockId(i - 1, j + 1, k) == Block::redstoneWire->blockID)
		{
			tessellator->setColorOpaque_F(f * f2, f * f3, f * f4);
			tessellator->addVertexWithUV((float)i + 0.015625f, (float)(j + 1) + 0.021875f, k + 1, d3, d5);
			tessellator->addVertexWithUV((float)i + 0.015625f, j + 0, k + 1, d1, d5);
			tessellator->addVertexWithUV((float)i + 0.015625f, j + 0, k + 0, d1, d7);
			tessellator->addVertexWithUV((float)i + 0.015625f, (float)(j + 1) + 0.021875f, k + 0, d3, d7);
			tessellator->setColorOpaque_F(f, f, f);
			tessellator->addVertexWithUV((float)i + 0.015625f, (float)(j + 1) + 0.021875f, k + 1, d3, d5 + 0.0625);
			tessellator->addVertexWithUV((float)i + 0.015625f, j + 0, k + 1, d1, d5 + 0.0625);
			tessellator->addVertexWithUV((float)i + 0.015625f, j + 0, k + 0, d1, d7 + 0.0625);
			tessellator->addVertexWithUV((float)i + 0.015625f, (float)(j + 1) + 0.021875f, k + 0, d3, d7 + 0.0625);
		}
		if (blockAccess->isBlockNormalCube(i + 1, j, k) && blockAccess->getBlockId(i + 1, j + 1, k) == Block::redstoneWire->blockID)
		{
			tessellator->setColorOpaque_F(f * f2, f * f3, f * f4);
			tessellator->addVertexWithUV((float)(i + 1) - 0.015625f, j + 0, k + 1, d1, d7);
			tessellator->addVertexWithUV((float)(i + 1) - 0.015625f, (float)(j + 1) + 0.021875f, k + 1, d3, d7);
			tessellator->addVertexWithUV((float)(i + 1) - 0.015625f, (float)(j + 1) + 0.021875f, k + 0, d3, d5);
			tessellator->addVertexWithUV((float)(i + 1) - 0.015625f, j + 0, k + 0, d1, d5);
			tessellator->setColorOpaque_F(f, f, f);
			tessellator->addVertexWithUV((float)(i + 1) - 0.015625f, j + 0, k + 1, d1, d7 + 0.0625);
			tessellator->addVertexWithUV((float)(i + 1) - 0.015625f, (float)(j + 1) + 0.021875f, k + 1, d3, d7 + 0.0625);
			tessellator->addVertexWithUV((float)(i + 1) - 0.015625f, (float)(j + 1) + 0.021875f, k + 0, d3, d5 + 0.0625);
			tessellator->addVertexWithUV((float)(i + 1) - 0.015625f, j + 0, k + 0, d1, d5 + 0.0625);
		}
		if (blockAccess->isBlockNormalCube(i, j, k - 1) && blockAccess->getBlockId(i, j + 1, k - 1) == Block::redstoneWire->blockID)
		{
			tessellator->setColorOpaque_F(f * f2, f * f3, f * f4);
			tessellator->addVertexWithUV(i + 1, j + 0, (float)k + 0.015625f, d1, d7);
			tessellator->addVertexWithUV(i + 1, (float)(j + 1) + 0.021875f, (float)k + 0.015625f, d3, d7);
			tessellator->addVertexWithUV(i + 0, (float)(j + 1) + 0.021875f, (float)k + 0.015625f, d3, d5);
			tessellator->addVertexWithUV(i + 0, j + 0, (float)k + 0.015625f, d1, d5);
			tessellator->setColorOpaque_F(f, f, f);
			tessellator->addVertexWithUV(i + 1, j + 0, (float)k + 0.015625f, d1, d7 + 0.0625);
			tessellator->addVertexWithUV(i + 1, (float)(j + 1) + 0.021875f, (float)k + 0.015625f, d3, d7 + 0.0625);
			tessellator->addVertexWithUV(i + 0, (float)(j + 1) + 0.021875f, (float)k + 0.015625f, d3, d5 + 0.0625);
			tessellator->addVertexWithUV(i + 0, j + 0, (float)k + 0.015625f, d1, d5 + 0.0625);
		}
		if (blockAccess->isBlockNormalCube(i, j, k + 1) && blockAccess->getBlockId(i, j + 1, k + 1) == Block::redstoneWire->blockID)
		{
			tessellator->setColorOpaque_F(f * f2, f * f3, f * f4);
			tessellator->addVertexWithUV(i + 1, (float)(j + 1) + 0.021875f, (float)(k + 1) - 0.015625f, d3, d5);
			tessellator->addVertexWithUV(i + 1, j + 0, (float)(k + 1) - 0.015625f, d1, d5);
			tessellator->addVertexWithUV(i + 0, j + 0, (float)(k + 1) - 0.015625f, d1, d7);
			tessellator->addVertexWithUV(i + 0, (float)(j + 1) + 0.021875f, (float)(k + 1) - 0.015625f, d3, d7);
			tessellator->setColorOpaque_F(f, f, f);
			tessellator->addVertexWithUV(i + 1, (float)(j + 1) + 0.021875f, (float)(k + 1) - 0.015625f, d3, d5 + 0.0625);
			tessellator->addVertexWithUV(i + 1, j + 0, (float)(k + 1) - 0.015625f, d1, d5 + 0.0625);
			tessellator->addVertexWithUV(i + 0, j + 0, (float)(k + 1) - 0.015625f, d1, d7 + 0.0625);
			tessellator->addVertexWithUV(i + 0, (float)(j + 1) + 0.021875f, (float)(k + 1) - 0.015625f, d3, d7 + 0.0625);
		}
	}
	return true;
}

bool RenderBlocks::renderBlockMinecartTrack(BlockRail *blockrail, int_t i, int_t j, int_t k)
{
	Tessellator *tessellator = &Tessellator::instance;
	int_t l = blockAccess->getBlockMetadata(i, j, k);
	int_t i1 = blockrail->getBlockTextureFromSideAndMetadata(0, l);
	if (overrideBlockTexture >= 0)
	{
		i1 = overrideBlockTexture;
	}
	if (blockrail->getIsPowered())
	{
		l &= 7;
	}
	float f = blockrail->getBlockBrightness(blockAccess, i, j, k);
	tessellator->setColorOpaque_F(f, f, f);
	int_t j1 = (i1 & 0xf) << 4;
	int_t k1 = i1 & 0xf0;
	double d = (float)j1 / 256.0f;
	double d1 = ((float)j1 + 15.99f) / 256.0f;
	double d2 = (float)k1 / 256.0f;
	double d3 = ((float)k1 + 15.99f) / 256.0f;
	float f1 = 0.0625f;
	float f2 = i + 1;
	float f3 = i + 1;
	float f4 = i + 0;
	float f5 = i + 0;
	float f6 = k + 0;
	float f7 = k + 1;
	float f8 = k + 1;
	float f9 = k + 0;
	float f10 = (float)j + f1;
	float f11 = (float)j + f1;
	float f12 = (float)j + f1;
	float f13 = (float)j + f1;
	if (l == 1 || l == 2 || l == 3 || l == 7)
	{
		f2 = f5 = i + 1;
		f3 = f4 = i + 0;
		f6 = f7 = k + 1;
		f8 = f9 = k + 0;
	}
	else if (l == 8)
	{
		f2 = f3 = i + 0;
		f4 = f5 = i + 1;
		f6 = f9 = k + 1;
		f7 = f8 = k + 0;
	}
	else if (l == 9)
	{
		f2 = f5 = i + 0;
		f3 = f4 = i + 1;
		f6 = f7 = k + 0;
		f8 = f9 = k + 1;
	}
	if (l == 2 || l == 4)
	{
		f10++;
		f13++;
	}
	else if (l == 3 || l == 5)
	{
		f11++;
		f12++;
	}
	tessellator->addVertexWithUV(f2, f10, f6, d1, d2);
	tessellator->addVertexWithUV(f3, f11, f7, d1, d3);
	tessellator->addVertexWithUV(f4, f12, f8, d, d3);
	tessellator->addVertexWithUV(f5, f13, f9, d, d2);
	tessellator->addVertexWithUV(f5, f13, f9, d, d2);
	tessellator->addVertexWithUV(f4, f12, f8, d, d3);
	tessellator->addVertexWithUV(f3, f11, f7, d1, d3);
	tessellator->addVertexWithUV(f2, f10, f6, d1, d2);
	return true;
}

bool RenderBlocks::renderBlockLadder(Block *block, int_t i, int_t j, int_t k)
{
	Tessellator *tessellator = &Tessellator::instance;
	int_t l = block->getBlockTextureFromSide(0);
	if (overrideBlockTexture >= 0)
	{
		l = overrideBlockTexture;
	}
	float f = block->getBlockBrightness(blockAccess, i, j, k);
	tessellator->setColorOpaque_F(f, f, f);
	int_t i1 = (l & 0xf) << 4;
	int_t j1 = l & 0xf0;
	double d = (float)i1 / 256.0f;
	double d1 = ((float)i1 + 15.99f) / 256.0f;
	double d2 = (float)j1 / 256.0f;
	double d3 = ((float)j1 + 15.99f) / 256.0f;
	int_t k1 = blockAccess->getBlockMetadata(i, j, k);
	float f1 = 0.0f;
	float f2 = 0.05f;
	if (k1 == 5)
	{
		tessellator->addVertexWithUV((float)i + f2, (float)(j + 1) + f1, (float)(k + 1) + f1, d, d2);
		tessellator->addVertexWithUV((float)i + f2, (float)(j + 0) - f1, (float)(k + 1) + f1, d, d3);
		tessellator->addVertexWithUV((float)i + f2, (float)(j + 0) - f1, (float)(k + 0) - f1, d1, d3);
		tessellator->addVertexWithUV((float)i + f2, (float)(j + 1) + f1, (float)(k + 0) - f1, d1, d2);
	}
	if (k1 == 4)
	{
		tessellator->addVertexWithUV((float)(i + 1) - f2, (float)(j + 0) - f1, (float)(k + 1) + f1, d1, d3);
		tessellator->addVertexWithUV((float)(i + 1) - f2, (float)(j + 1) + f1, (float)(k + 1) + f1, d1, d2);
		tessellator->addVertexWithUV((float)(i + 1) - f2, (float)(j + 1) + f1, (float)(k + 0) - f1, d, d2);
		tessellator->addVertexWithUV((float)(i + 1) - f2, (float)(j + 0) - f1, (float)(k + 0) - f1, d, d3);
	}
	if (k1 == 3)
	{
		tessellator->addVertexWithUV((float)(i + 1) + f1, (float)(j + 0) - f1, (float)k + f2, d1, d3);
		tessellator->addVertexWithUV((float)(i + 1) + f1, (float)(j + 1) + f1, (float)k + f2, d1, d2);
		tessellator->addVertexWithUV((float)(i + 0) - f1, (float)(j + 1) + f1, (float)k + f2, d, d2);
		tessellator->addVertexWithUV((float)(i + 0) - f1, (float)(j + 0) - f1, (float)k + f2, d, d3);
	}
	if (k1 == 2)
	{
		tessellator->addVertexWithUV((float)(i + 1) + f1, (float)(j + 1) + f1, (float)(k + 1) - f2, d, d2);
		tessellator->addVertexWithUV((float)(i + 1) + f1, (float)(j + 0) - f1, (float)(k + 1) - f2, d, d3);
		tessellator->addVertexWithUV((float)(i + 0) - f1, (float)(j + 0) - f1, (float)(k + 1) - f2, d1, d3);
		tessellator->addVertexWithUV((float)(i + 0) - f1, (float)(j + 1) + f1, (float)(k + 1) - f2, d1, d2);
	}
	return true;
}

bool RenderBlocks::renderBlockReed(Block *block, int_t i, int_t j, int_t k)
{
	Tessellator *tessellator = &Tessellator::instance;
	float f = block->getBlockBrightness(blockAccess, i, j, k);
	int_t l = block->colorMultiplier(blockAccess, i, j, k);
	float f1 = (float)(l >> 16 & 0xff) / 255.0f;
	float f2 = (float)(l >> 8 & 0xff) / 255.0f;
	float f3 = (float)(l & 0xff) / 255.0f;
	if (Minecraft::isAmbientOcclusionEnabled())
	{
		float f4 = (f1 * 30.0f + f2 * 59.0f + f3 * 11.0f) / 100.0f;
		float f5 = (f1 * 30.0f + f2 * 70.0f) / 100.0f;
		float f6 = (f1 * 30.0f + f3 * 70.0f) / 100.0f;
		f1 = f4;
		f2 = f5;
		f3 = f6;
	}
	tessellator->setColorOpaque_F(f * f1, f * f2, f * f3);
	double d = i;
	double d1 = j;
	double d2 = k;
	if (block == static_cast<Block*>(Block::tallGrass))
	{
		long_t l1 = (long_t)(i * 0x2fc20f) ^ (long_t)k * 0x6ebfff5L ^ (long_t)j;
		l1 = l1 * l1 * 0x285b825L + l1 * 11LL;
		d += ((double)((float)(l1 >> 16 & 15LL) / 15.0f) - 0.5) * 0.5;
		d1 += ((double)((float)(l1 >> 20 & 15LL) / 15.0f) - 1.0) * 0.20000000000000001;
		d2 += ((double)((float)(l1 >> 24 & 15LL) / 15.0f) - 0.5) * 0.5;
	}
	renderCrossedSquares(block, blockAccess->getBlockMetadata(i, j, k), d, d1, d2);
	return true;
}

bool RenderBlocks::renderBlockCrops(Block *block, int_t i, int_t j, int_t k)
{
	Tessellator *tessellator = &Tessellator::instance;
	float f = block->getBlockBrightness(blockAccess, i, j, k);
	tessellator->setColorOpaque_F(f, f, f);
	renderCropsCrossed(block, blockAccess->getBlockMetadata(i, j, k), i, (float)j - 0.0625f, k);
	return true;
}

void RenderBlocks::renderTorchAtAngle(Block *block, double d, double d1, double d2, double d3, double d4)
{
	Tessellator *tessellator = &Tessellator::instance;
	int_t i = block->getBlockTextureFromSide(0);
	if (overrideBlockTexture >= 0)
	{
		i = overrideBlockTexture;
	}
	int_t j = (i & 0xf) << 4;
	int_t k = i & 0xf0;
	float f = (float)j / 256.0f;
	float f1 = ((float)j + 15.99f) / 256.0f;
	float f2 = (float)k / 256.0f;
	float f3 = ((float)k + 15.99f) / 256.0f;
	double d5 = (double)f + 0.02734375;
	double d6 = (double)f2 + 0.0234375;
	double d7 = (double)f + 0.03515625;
	double d8 = (double)f2 + 0.03125;
	d += 0.5;
	d2 += 0.5;
	double d9 = d - 0.5;
	double d10 = d + 0.5;
	double d11 = d2 - 0.5;
	double d12 = d2 + 0.5;
	double d13 = 0.0625;
	double d14 = 0.625;
	tessellator->addVertexWithUV((d + d3 * (1.0 - d14)) - d13, d1 + d14, (d2 + d4 * (1.0 - d14)) - d13, d5, d6);
	tessellator->addVertexWithUV((d + d3 * (1.0 - d14)) - d13, d1 + d14, d2 + d4 * (1.0 - d14) + d13, d5, d8);
	tessellator->addVertexWithUV(d + d3 * (1.0 - d14) + d13, d1 + d14, d2 + d4 * (1.0 - d14) + d13, d7, d8);
	tessellator->addVertexWithUV(d + d3 * (1.0 - d14) + d13, d1 + d14, (d2 + d4 * (1.0 - d14)) - d13, d7, d6);
	tessellator->addVertexWithUV(d - d13, d1 + 1.0, d11, f, f2);
	tessellator->addVertexWithUV((d - d13) + d3, d1 + 0.0, d11 + d4, f, f3);
	tessellator->addVertexWithUV((d - d13) + d3, d1 + 0.0, d12 + d4, f1, f3);
	tessellator->addVertexWithUV(d - d13, d1 + 1.0, d12, f1, f2);
	tessellator->addVertexWithUV(d + d13, d1 + 1.0, d12, f, f2);
	tessellator->addVertexWithUV(d + d3 + d13, d1 + 0.0, d12 + d4, f, f3);
	tessellator->addVertexWithUV(d + d3 + d13, d1 + 0.0, d11 + d4, f1, f3);
	tessellator->addVertexWithUV(d + d13, d1 + 1.0, d11, f1, f2);
	tessellator->addVertexWithUV(d9, d1 + 1.0, d2 + d13, f, f2);
	tessellator->addVertexWithUV(d9 + d3, d1 + 0.0, d2 + d13 + d4, f, f3);
	tessellator->addVertexWithUV(d10 + d3, d1 + 0.0, d2 + d13 + d4, f1, f3);
	tessellator->addVertexWithUV(d10, d1 + 1.0, d2 + d13, f1, f2);
	tessellator->addVertexWithUV(d10, d1 + 1.0, d2 - d13, f, f2);
	tessellator->addVertexWithUV(d10 + d3, d1 + 0.0, (d2 - d13) + d4, f, f3);
	tessellator->addVertexWithUV(d9 + d3, d1 + 0.0, (d2 - d13) + d4, f1, f3);
	tessellator->addVertexWithUV(d9, d1 + 1.0, d2 - d13, f1, f2);
}

void RenderBlocks::renderCrossedSquares(Block *block, int_t i, double d, double d1, double d2)
{
	Tessellator *tessellator = &Tessellator::instance;
	int_t j = block->getBlockTextureFromSideAndMetadata(0, i);
	if (overrideBlockTexture >= 0)
	{
		j = overrideBlockTexture;
	}
	int_t k = (j & 0xf) << 4;
	int_t l = j & 0xf0;
	double d3 = (float)k / 256.0f;
	double d4 = ((float)k + 15.99f) / 256.0f;
	double d5 = (float)l / 256.0f;
	double d6 = ((float)l + 15.99f) / 256.0f;
	double d7 = (d + 0.5) - 0.44999998807907104;
	double d8 = d + 0.5 + 0.44999998807907104;
	double d9 = (d2 + 0.5) - 0.44999998807907104;
	double d10 = d2 + 0.5 + 0.44999998807907104;
	tessellator->addVertexWithUV(d7, d1 + 1.0, d9, d3, d5);
	tessellator->addVertexWithUV(d7, d1 + 0.0, d9, d3, d6);
	tessellator->addVertexWithUV(d8, d1 + 0.0, d10, d4, d6);
	tessellator->addVertexWithUV(d8, d1 + 1.0, d10, d4, d5);
	tessellator->addVertexWithUV(d8, d1 + 1.0, d10, d3, d5);
	tessellator->addVertexWithUV(d8, d1 + 0.0, d10, d3, d6);
	tessellator->addVertexWithUV(d7, d1 + 0.0, d9, d4, d6);
	tessellator->addVertexWithUV(d7, d1 + 1.0, d9, d4, d5);
	tessellator->addVertexWithUV(d7, d1 + 1.0, d10, d3, d5);
	tessellator->addVertexWithUV(d7, d1 + 0.0, d10, d3, d6);
	tessellator->addVertexWithUV(d8, d1 + 0.0, d9, d4, d6);
	tessellator->addVertexWithUV(d8, d1 + 1.0, d9, d4, d5);
	tessellator->addVertexWithUV(d8, d1 + 1.0, d9, d3, d5);
	tessellator->addVertexWithUV(d8, d1 + 0.0, d9, d3, d6);
	tessellator->addVertexWithUV(d7, d1 + 0.0, d10, d4, d6);
	tessellator->addVertexWithUV(d7, d1 + 1.0, d10, d4, d5);
}

void RenderBlocks::renderCropsCrossed(Block *block, int_t i, double d, double d1, double d2)
{
	Tessellator *tessellator = &Tessellator::instance;
	int_t j = block->getBlockTextureFromSideAndMetadata(0, i);
	if (overrideBlockTexture >= 0)
	{
		j = overrideBlockTexture;
	}
	int_t k = (j & 0xf) << 4;
	int_t l = j & 0xf0;
	double d3 = (float)k / 256.0f;
	double d4 = ((float)k + 15.99f) / 256.0f;
	double d5 = (float)l / 256.0f;
	double d6 = ((float)l + 15.99f) / 256.0f;
	double d7 = (d + 0.5) - 0.25;
	double d8 = d + 0.5 + 0.25;
	double d9 = (d2 + 0.5) - 0.5;
	double d10 = d2 + 0.5 + 0.5;
	tessellator->addVertexWithUV(d7, d1 + 1.0, d9, d3, d5);
	tessellator->addVertexWithUV(d7, d1 + 0.0, d9, d3, d6);
	tessellator->addVertexWithUV(d7, d1 + 0.0, d10, d4, d6);
	tessellator->addVertexWithUV(d7, d1 + 1.0, d10, d4, d5);
	tessellator->addVertexWithUV(d7, d1 + 1.0, d10, d3, d5);
	tessellator->addVertexWithUV(d7, d1 + 0.0, d10, d3, d6);
	tessellator->addVertexWithUV(d7, d1 + 0.0, d9, d4, d6);
	tessellator->addVertexWithUV(d7, d1 + 1.0, d9, d4, d5);
	tessellator->addVertexWithUV(d8, d1 + 1.0, d10, d3, d5);
	tessellator->addVertexWithUV(d8, d1 + 0.0, d10, d3, d6);
	tessellator->addVertexWithUV(d8, d1 + 0.0, d9, d4, d6);
	tessellator->addVertexWithUV(d8, d1 + 1.0, d9, d4, d5);
	tessellator->addVertexWithUV(d8, d1 + 1.0, d9, d3, d5);
	tessellator->addVertexWithUV(d8, d1 + 0.0, d9, d3, d6);
	tessellator->addVertexWithUV(d8, d1 + 0.0, d10, d4, d6);
	tessellator->addVertexWithUV(d8, d1 + 1.0, d10, d4, d5);
	d7 = (d + 0.5) - 0.5;
	d8 = d + 0.5 + 0.5;
	d9 = (d2 + 0.5) - 0.25;
	d10 = d2 + 0.5 + 0.25;
	tessellator->addVertexWithUV(d7, d1 + 1.0, d9, d3, d5);
	tessellator->addVertexWithUV(d7, d1 + 0.0, d9, d3, d6);
	tessellator->addVertexWithUV(d8, d1 + 0.0, d9, d4, d6);
	tessellator->addVertexWithUV(d8, d1 + 1.0, d9, d4, d5);
	tessellator->addVertexWithUV(d8, d1 + 1.0, d9, d3, d5);
	tessellator->addVertexWithUV(d8, d1 + 0.0, d9, d3, d6);
	tessellator->addVertexWithUV(d7, d1 + 0.0, d9, d4, d6);
	tessellator->addVertexWithUV(d7, d1 + 1.0, d9, d4, d5);
	tessellator->addVertexWithUV(d8, d1 + 1.0, d10, d3, d5);
	tessellator->addVertexWithUV(d8, d1 + 0.0, d10, d3, d6);
	tessellator->addVertexWithUV(d7, d1 + 0.0, d10, d4, d6);
	tessellator->addVertexWithUV(d7, d1 + 1.0, d10, d4, d5);
	tessellator->addVertexWithUV(d7, d1 + 1.0, d10, d3, d5);
	tessellator->addVertexWithUV(d7, d1 + 0.0, d10, d3, d6);
	tessellator->addVertexWithUV(d8, d1 + 0.0, d10, d4, d6);
	tessellator->addVertexWithUV(d8, d1 + 1.0, d10, d4, d5);
}

bool RenderBlocks::renderBlockFluids(Block *block, int_t i, int_t j, int_t k)
{
	Tessellator *tessellator = &Tessellator::instance;
	int_t l = block->colorMultiplier(blockAccess, i, j, k);
	float f = (float)(l >> 16 & 0xff) / 255.0f;
	float f1 = (float)(l >> 8 & 0xff) / 255.0f;
	float f2 = (float)(l & 0xff) / 255.0f;
	bool flag = block->shouldSideBeRendered(blockAccess, i, j + 1, k, 1);
	bool flag1 = block->shouldSideBeRendered(blockAccess, i, j - 1, k, 0);
	bool aflag[4];
	aflag[0] = block->shouldSideBeRendered(blockAccess, i, j, k - 1, 2);
	aflag[1] = block->shouldSideBeRendered(blockAccess, i, j, k + 1, 3);
	aflag[2] = block->shouldSideBeRendered(blockAccess, i - 1, j, k, 4);
	aflag[3] = block->shouldSideBeRendered(blockAccess, i + 1, j, k, 5);
	if (!flag && !flag1 && !aflag[0] && !aflag[1] && !aflag[2] && !aflag[3])
	{
		return false;
	}
	bool flag2 = false;
	float f3 = 0.5f;
	float f4 = 1.0f;
	float f5 = 0.8f;
	float f6 = 0.6f;
	double d = 0.0;
	double d1 = 1.0;
	Material *material = block->blockMaterial;
	int_t i1 = blockAccess->getBlockMetadata(i, j, k);
	float f7 = getFluidHeight(i, j, k, material);
	float f8 = getFluidHeight(i, j, k + 1, material);
	float f9 = getFluidHeight(i + 1, j, k + 1, material);
	float f10 = getFluidHeight(i + 1, j, k, material);
	if (renderAllFaces || flag)
	{
		flag2 = true;
		int_t j1 = block->getBlockTextureFromSideAndMetadata(1, i1);
		float f12 = (float)BlockFluid::func_293_a(blockAccess, i, j, k, material);
		if (f12 > -999.0f)
		{
			j1 = block->getBlockTextureFromSideAndMetadata(2, i1);
		}
		int_t i2 = (j1 & 0xf) << 4;
		int_t k2 = j1 & 0xf0;
		double d2 = ((double)i2 + 8.0) / 256.0;
		double d3 = ((double)k2 + 8.0) / 256.0;
		if (f12 < -999.0f)
		{
			f12 = 0.0f;
		}
		else
		{
			d2 = (float)(i2 + 16) / 256.0f;
			d3 = (float)(k2 + 16) / 256.0f;
		}
		float f14 = (MathHelper::sin(f12) * 8.0f) / 256.0f;
		float f16 = (MathHelper::cos(f12) * 8.0f) / 256.0f;
		float f18 = block->getBlockBrightness(blockAccess, i, j, k);
		tessellator->setColorOpaque_F(f4 * f18 * f, f4 * f18 * f1, f4 * f18 * f2);
		tessellator->addVertexWithUV(i + 0, (float)j + f7, k + 0, d2 - (double)f16 - (double)f14, (d3 - (double)f16) + (double)f14);
		tessellator->addVertexWithUV(i + 0, (float)j + f8, k + 1, (d2 - (double)f16) + (double)f14, d3 + (double)f16 + (double)f14);
		tessellator->addVertexWithUV(i + 1, (float)j + f9, k + 1, d2 + (double)f16 + (double)f14, (d3 + (double)f16) - (double)f14);
		tessellator->addVertexWithUV(i + 1, (float)j + f10, k + 0, (d2 + (double)f16) - (double)f14, d3 - (double)f16 - (double)f14);
	}
	if (renderAllFaces || flag1)
	{
		float f11 = block->getBlockBrightness(blockAccess, i, j - 1, k);
		tessellator->setColorOpaque_F(f3 * f11, f3 * f11, f3 * f11);
		renderBottomFace(block, i, j, k, block->getBlockTextureFromSide(0));
		flag2 = true;
	}
	for (int_t k1 = 0; k1 < 4; k1++)
	{
		int_t l1 = i;
		int_t j2 = j;
		int_t l2 = k;
		if (k1 == 0)
		{
			l2--;
		}
		if (k1 == 1)
		{
			l2++;
		}
		if (k1 == 2)
		{
			l1--;
		}
		if (k1 == 3)
		{
			l1++;
		}
		int_t i3 = block->getBlockTextureFromSideAndMetadata(k1 + 2, i1);
		int_t j3 = (i3 & 0xf) << 4;
		int_t k3 = i3 & 0xf0;
		if (!renderAllFaces && !aflag[k1])
		{
			continue;
		}
		float f13;
		float f15;
		float f17;
		float f19;
		float f20;
		float f21;
		if (k1 == 0)
		{
			f13 = f7;
			f15 = f10;
			f17 = i;
			f20 = i + 1;
			f19 = k;
			f21 = k;
		}
		else if (k1 == 1)
		{
			f13 = f9;
			f15 = f8;
			f17 = i + 1;
			f20 = i;
			f19 = k + 1;
			f21 = k + 1;
		}
		else if (k1 == 2)
		{
			f13 = f8;
			f15 = f7;
			f17 = i;
			f20 = i;
			f19 = k + 1;
			f21 = k;
		}
		else
		{
			f13 = f10;
			f15 = f9;
			f17 = i + 1;
			f20 = i + 1;
			f19 = k;
			f21 = k + 1;
		}
		flag2 = true;
		double d4 = (float)(j3 + 0) / 256.0f;
		double d5 = ((double)(j3 + 16) - 0.01) / 256.0;
		double d6 = ((float)k3 + (1.0f - f13) * 16.0f) / 256.0f;
		double d7 = ((float)k3 + (1.0f - f15) * 16.0f) / 256.0f;
		double d8 = ((double)(k3 + 16) - 0.01) / 256.0;
		float f22 = block->getBlockBrightness(blockAccess, l1, j2, l2);
		if (k1 < 2)
		{
			f22 *= f5;
		}
		else
		{
			f22 *= f6;
		}
		tessellator->setColorOpaque_F(f4 * f22 * f, f4 * f22 * f1, f4 * f22 * f2);
		tessellator->addVertexWithUV(f17, (float)j + f13, f19, d4, d6);
		tessellator->addVertexWithUV(f20, (float)j + f15, f21, d5, d7);
		tessellator->addVertexWithUV(f20, j + 0, f21, d5, d8);
		tessellator->addVertexWithUV(f17, j + 0, f19, d4, d8);
	}

	block->minY = d;
	block->maxY = d1;
	return flag2;
}

float RenderBlocks::getFluidHeight(int_t i, int_t j, int_t k, Material *material)
{
	int_t l = 0;
	float f = 0.0f;
	for (int_t i1 = 0; i1 < 4; i1++)
	{
		int_t j1 = i - (i1 & 1);
		int_t k1 = j;
		int_t l1 = k - (i1 >> 1 & 1);
		if (blockAccess->getBlockMaterial(j1, k1 + 1, l1) == material)
		{
			return 1.0f;
		}
		Material *material1 = blockAccess->getBlockMaterial(j1, k1, l1);
		if (material1 == material)
		{
			int_t i2 = blockAccess->getBlockMetadata(j1, k1, l1);
			if (i2 >= 8 || i2 == 0)
			{
				f += BlockFluid::getPercentAir(i2) * 10.0f;
				l += 10;
			}
			f += BlockFluid::getPercentAir(i2);
			l++;
			continue;
		}
		if (!material1->isSolid())
		{
			f++;
			l++;
		}
	}

	return 1.0f - f / (float)l;
}

void RenderBlocks::renderBlockFallingSand(Block *block, World *world, int_t i, int_t j, int_t k)
{
	float f = 0.5f;
	float f1 = 1.0f;
	float f2 = 0.8f;
	float f3 = 0.6f;
	Tessellator *tessellator = &Tessellator::instance;
	tessellator->startDrawingQuads();
	float f4 = block->getBlockBrightness(world, i, j, k);
	float f5 = block->getBlockBrightness(world, i, j - 1, k);
	if (f5 < f4)
	{
		f5 = f4;
	}
	tessellator->setColorOpaque_F(f * f5, f * f5, f * f5);
	renderBottomFace(block, -0.5, -0.5, -0.5, block->getBlockTextureFromSide(0));
	f5 = block->getBlockBrightness(world, i, j + 1, k);
	if (f5 < f4)
	{
		f5 = f4;
	}
	tessellator->setColorOpaque_F(f1 * f5, f1 * f5, f1 * f5);
	renderTopFace(block, -0.5, -0.5, -0.5, block->getBlockTextureFromSide(1));
	f5 = block->getBlockBrightness(world, i, j, k - 1);
	if (f5 < f4)
	{
		f5 = f4;
	}
	tessellator->setColorOpaque_F(f2 * f5, f2 * f5, f2 * f5);
	renderEastFace(block, -0.5, -0.5, -0.5, block->getBlockTextureFromSide(2));
	f5 = block->getBlockBrightness(world, i, j, k + 1);
	if (f5 < f4)
	{
		f5 = f4;
	}
	tessellator->setColorOpaque_F(f2 * f5, f2 * f5, f2 * f5);
	renderWestFace(block, -0.5, -0.5, -0.5, block->getBlockTextureFromSide(3));
	f5 = block->getBlockBrightness(world, i - 1, j, k);
	if (f5 < f4)
	{
		f5 = f4;
	}
	tessellator->setColorOpaque_F(f3 * f5, f3 * f5, f3 * f5);
	renderNorthFace(block, -0.5, -0.5, -0.5, block->getBlockTextureFromSide(4));
	f5 = block->getBlockBrightness(world, i + 1, j, k);
	if (f5 < f4)
	{
		f5 = f4;
	}
	tessellator->setColorOpaque_F(f3 * f5, f3 * f5, f3 * f5);
	renderSouthFace(block, -0.5, -0.5, -0.5, block->getBlockTextureFromSide(5));
	tessellator->draw();
}

bool RenderBlocks::renderStandardBlock(Block *block, int_t i, int_t j, int_t k)
{
	int_t l = block->colorMultiplier(blockAccess, i, j, k);
	float f = (float)(l >> 16 & 0xff) / 255.0f;
		float f1 = (float)(l >> 8 & 0xff) / 255.0f;
	float f2 = (float)(l & 0xff) / 255.0f;
	if (Minecraft::isAmbientOcclusionEnabled())
	{
		float f3 = (f * 30.0f + f1 * 59.0f + f2 * 11.0f) / 100.0f;
		float f4 = (f * 30.0f + f1 * 70.0f) / 100.0f;
		float f5 = (f * 30.0f + f2 * 70.0f) / 100.0f;
		f = f3;
		f1 = f4;
		f2 = f5;
	}
	if (Minecraft::isAmbientOcclusionEnabled())
	{
		return renderStandardBlockWithAmbientOcclusion(block, i, j, k, f, f1, f2);
	}
	else
	{
		return renderStandardBlockWithColorMultiplier(block, i, j, k, f, f1, f2);
	}
}

bool RenderBlocks::renderStandardBlockWithAmbientOcclusion(Block *block, int_t i, int_t j, int_t k, float f, float f1, float f2)
{
	enableAO = true;
	bool flag = false;
	float f3 = lightValueOwn;
	float f10 = lightValueOwn;
	float f17 = lightValueOwn;
	float f24 = lightValueOwn;
	bool flag1 = true;
	bool flag2 = true;
	bool flag3 = true;
	bool flag4 = true;
	bool flag5 = true;
	bool flag6 = true;
	lightValueOwn = block->getBlockBrightness(blockAccess, i, j, k);
	aoLightValueXNeg = block->getBlockBrightness(blockAccess, i - 1, j, k);
	aoLightValueYNeg = block->getBlockBrightness(blockAccess, i, j - 1, k);
	aoLightValueZNeg = block->getBlockBrightness(blockAccess, i, j, k - 1);
	aoLightValueXPos = block->getBlockBrightness(blockAccess, i + 1, j, k);
	aoLightValueYPos = block->getBlockBrightness(blockAccess, i, j + 1, k);
	aoLightValueZPos = block->getBlockBrightness(blockAccess, i, j, k + 1);
	canBlockGrassAboveEast = Block::canBlockGrass[blockAccess->getBlockId(i + 1, j + 1, k)];
	canBlockGrassBelowEastSouth = Block::canBlockGrass[blockAccess->getBlockId(i + 1, j - 1, k)];
	canBlockGrassBelowEast = Block::canBlockGrass[blockAccess->getBlockId(i + 1, j, k + 1)];
	canBlockGrassBelowWest = Block::canBlockGrass[blockAccess->getBlockId(i + 1, j, k - 1)];
	canBlockGrassAboveWest = Block::canBlockGrass[blockAccess->getBlockId(i - 1, j + 1, k)];
	canBlockGrassBelowWestSouth = Block::canBlockGrass[blockAccess->getBlockId(i - 1, j - 1, k)];
	canBlockGrassBelowNorth = Block::canBlockGrass[blockAccess->getBlockId(i - 1, j, k - 1)];
	canBlockGrassBelowSouth = Block::canBlockGrass[blockAccess->getBlockId(i - 1, j, k + 1)];
	canBlockGrassAboveSouth = Block::canBlockGrass[blockAccess->getBlockId(i, j + 1, k + 1)];
	canBlockGrassAboveNorth = Block::canBlockGrass[blockAccess->getBlockId(i, j + 1, k - 1)];
	canBlockGrassBelowSouthEast = Block::canBlockGrass[blockAccess->getBlockId(i, j - 1, k + 1)];
	canBlockGrassBelowNorthWest = Block::canBlockGrass[blockAccess->getBlockId(i, j - 1, k - 1)];
	if (block->blockIndexInTexture == 3)
	{
		flag1 = flag3 = flag4 = flag5 = flag6 = false;
	}
	if (overrideBlockTexture >= 0)
	{
		flag1 = flag3 = flag4 = flag5 = flag6 = false;
	}
	if (renderAllFaces || block->shouldSideBeRendered(blockAccess, i, j - 1, k, 0))
	{
		float f4;
		float f11;
		float f18;
		float f25;
		if (lightingQuality > 0)
		{
			j--;
			aoLightValueXNeg2 = block->getBlockBrightness(blockAccess, i - 1, j, k);
			aoLightValueYNeg2 = block->getBlockBrightness(blockAccess, i, j, k - 1);
			aoLightValueXYZPNN = block->getBlockBrightness(blockAccess, i, j, k + 1);
			aoLightValueXPos2 = block->getBlockBrightness(blockAccess, i + 1, j, k);
			if (canBlockGrassBelowNorthWest || canBlockGrassBelowWestSouth)
			{
				aoLightValueXYZNNN = block->getBlockBrightness(blockAccess, i - 1, j, k - 1);
			}
			else
			{
				aoLightValueXYZNNN = aoLightValueXNeg2;
			}
			if (canBlockGrassBelowSouthEast || canBlockGrassBelowWestSouth)
			{
				aoLightValueXYZNNP = block->getBlockBrightness(blockAccess, i - 1, j, k + 1);
			}
			else
			{
				aoLightValueXYZNNP = aoLightValueXNeg2;
			}
			if (canBlockGrassBelowNorthWest || canBlockGrassBelowEastSouth)
			{
				aoLightValueXYZPNP = block->getBlockBrightness(blockAccess, i + 1, j, k - 1);
			}
			else
			{
				aoLightValueXYZPNP = aoLightValueXPos2;
			}
			if (canBlockGrassBelowSouthEast || canBlockGrassBelowEastSouth)
			{
				aoLightValueXYZPPP = block->getBlockBrightness(blockAccess, i + 1, j, k + 1);
			}
			else
			{
				aoLightValueXYZPPP = aoLightValueXPos2;
			}
			j++;
			f4 = (aoLightValueXYZNNP + aoLightValueXNeg2 + aoLightValueXYZPNN + aoLightValueYNeg) / 4.0f;
			f25 = (aoLightValueXYZPNN + aoLightValueYNeg + aoLightValueXYZPPP + aoLightValueXPos2) / 4.0f;
			f18 = (aoLightValueYNeg + aoLightValueYNeg2 + aoLightValueXPos2 + aoLightValueXYZPNP) / 4.0f;
			f11 = (aoLightValueXNeg2 + aoLightValueXYZNNN + aoLightValueYNeg + aoLightValueYNeg2) / 4.0f;
		}
		else
		{
			f4 = f11 = f18 = f25 = aoLightValueYNeg;
		}
		colorRedTopLeft = colorRedBottomLeft = colorRedBottomRight = colorRedTopRight = (flag1 ? f : 1.0f) * 0.5f;
		colorGreenTopLeft = colorGreenBottomLeft = colorGreenBottomRight = colorGreenTopRight = (flag1 ? f1 : 1.0f) * 0.5f;
		colorBlueTopLeft = colorBlueBottomLeft = colorBlueBottomRight = colorBlueTopRight = (flag1 ? f2 : 1.0f) * 0.5f;
		colorRedTopLeft *= f4;
		colorGreenTopLeft *= f4;
		colorBlueTopLeft *= f4;
		colorRedBottomLeft *= f11;
		colorGreenBottomLeft *= f11;
		colorBlueBottomLeft *= f11;
		colorRedBottomRight *= f18;
		colorGreenBottomRight *= f18;
		colorBlueBottomRight *= f18;
		colorRedTopRight *= f25;
		colorGreenTopRight *= f25;
		colorBlueTopRight *= f25;
		renderBottomFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 0));
		flag = true;
	}
	if (renderAllFaces || block->shouldSideBeRendered(blockAccess, i, j + 1, k, 1))
	{
		float f5;
		float f12;
		float f19;
		float f26;
		if (lightingQuality > 0)
		{
			j++;
			aoLightValueXPosUp = block->getBlockBrightness(blockAccess, i - 1, j, k);
			aoLightValueXPosUp2 = block->getBlockBrightness(blockAccess, i + 1, j, k);
			aoLightValueZNegUp = block->getBlockBrightness(blockAccess, i, j, k - 1);
			aoLightValueZPosUp = block->getBlockBrightness(blockAccess, i, j, k + 1);
			if (canBlockGrassAboveNorth || canBlockGrassAboveWest)
			{
				aoLightValueXYZNNP2 = block->getBlockBrightness(blockAccess, i - 1, j, k - 1);
			}
			else
			{
				aoLightValueXYZNNP2 = aoLightValueXPosUp;
			}
			if (canBlockGrassAboveNorth || canBlockGrassAboveEast)
			{
				aoLightValueXYZPPN = block->getBlockBrightness(blockAccess, i + 1, j, k - 1);
			}
			else
			{
				aoLightValueXYZPPN = aoLightValueXPosUp2;
			}
			if (canBlockGrassAboveSouth || canBlockGrassAboveWest)
			{
				aoLightValueXYZPNP2 = block->getBlockBrightness(blockAccess, i - 1, j, k + 1);
			}
			else
			{
				aoLightValueXYZPNP2 = aoLightValueXPosUp;
			}
			if (canBlockGrassAboveSouth || canBlockGrassAboveEast)
			{
				aoLightValueXYZPPP2 = block->getBlockBrightness(blockAccess, i + 1, j, k + 1);
			}
			else
			{
				aoLightValueXYZPPP2 = aoLightValueXPosUp2;
			}
			j--;
			f26 = (aoLightValueXYZPNP2 + aoLightValueXPosUp + aoLightValueZPosUp + aoLightValueYPos) / 4.0f;
			f5 = (aoLightValueZPosUp + aoLightValueYPos + aoLightValueXYZPPP2 + aoLightValueXPosUp2) / 4.0f;
			f12 = (aoLightValueYPos + aoLightValueZNegUp + aoLightValueXPosUp2 + aoLightValueXYZPPN) / 4.0f;
			f19 = (aoLightValueXPosUp + aoLightValueXYZNNP2 + aoLightValueYPos + aoLightValueZNegUp) / 4.0f;
		}
		else
		{
			f5 = f12 = f19 = f26 = aoLightValueYPos;
		}
		colorRedTopLeft = colorRedBottomLeft = colorRedBottomRight = colorRedTopRight = flag2 ? f : 1.0f;
		colorGreenTopLeft = colorGreenBottomLeft = colorGreenBottomRight = colorGreenTopRight = flag2 ? f1 : 1.0f;
		colorBlueTopLeft = colorBlueBottomLeft = colorBlueBottomRight = colorBlueTopRight = flag2 ? f2 : 1.0f;
		colorRedTopLeft *= f5;
		colorGreenTopLeft *= f5;
		colorBlueTopLeft *= f5;
		colorRedBottomLeft *= f12;
		colorGreenBottomLeft *= f12;
		colorBlueBottomLeft *= f12;
		colorRedBottomRight *= f19;
		colorGreenBottomRight *= f19;
		colorBlueBottomRight *= f19;
		colorRedTopRight *= f26;
		colorGreenTopRight *= f26;
		colorBlueTopRight *= f26;
		renderTopFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 1));
		flag = true;
	}
	if (renderAllFaces || block->shouldSideBeRendered(blockAccess, i, j, k - 1, 2))
	{
		float f6;
		float f13;
		float f20;
		float f27;
		if (lightingQuality > 0)
		{
			k--;
			aoLightValueXNegNorth = block->getBlockBrightness(blockAccess, i - 1, j, k);
			aoLightValueYNeg2 = block->getBlockBrightness(blockAccess, i, j - 1, k);
			aoLightValueZNegUp = block->getBlockBrightness(blockAccess, i, j + 1, k);
			aoLightValueXPosNorth = block->getBlockBrightness(blockAccess, i + 1, j, k);
			if (canBlockGrassBelowNorth || canBlockGrassBelowNorthWest)
			{
				aoLightValueXYZNNN = block->getBlockBrightness(blockAccess, i - 1, j - 1, k);
			}
			else
			{
				aoLightValueXYZNNN = aoLightValueXNegNorth;
			}
			if (canBlockGrassBelowNorth || canBlockGrassAboveNorth)
			{
				aoLightValueXYZNNP2 = block->getBlockBrightness(blockAccess, i - 1, j + 1, k);
			}
			else
			{
				aoLightValueXYZNNP2 = aoLightValueXNegNorth;
			}
			if (canBlockGrassBelowWest || canBlockGrassBelowNorthWest)
			{
				aoLightValueXYZPNP = block->getBlockBrightness(blockAccess, i + 1, j - 1, k);
			}
			else
			{
				aoLightValueXYZPNP = aoLightValueXPosNorth;
			}
			if (canBlockGrassBelowWest || canBlockGrassAboveNorth)
			{
				aoLightValueXYZPPN = block->getBlockBrightness(blockAccess, i + 1, j + 1, k);
			}
			else
			{
				aoLightValueXYZPPN = aoLightValueXPosNorth;
			}
			k++;
			f6 = (aoLightValueXNegNorth + aoLightValueXYZNNP2 + aoLightValueZNeg + aoLightValueZNegUp) / 4.0f;
			f13 = (aoLightValueZNeg + aoLightValueZNegUp + aoLightValueXPosNorth + aoLightValueXYZPPN) / 4.0f;
			f20 = (aoLightValueYNeg2 + aoLightValueZNeg + aoLightValueXYZPNP + aoLightValueXPosNorth) / 4.0f;
			f27 = (aoLightValueXYZNNN + aoLightValueXNegNorth + aoLightValueYNeg2 + aoLightValueZNeg) / 4.0f;
		}
		else
		{
			f6 = f13 = f20 = f27 = aoLightValueZNeg;
		}
		colorRedTopLeft = colorRedBottomLeft = colorRedBottomRight = colorRedTopRight = (flag3 ? f : 1.0f) * 0.8f;
		colorGreenTopLeft = colorGreenBottomLeft = colorGreenBottomRight = colorGreenTopRight = (flag3 ? f1 : 1.0f) * 0.8f;
		colorBlueTopLeft = colorBlueBottomLeft = colorBlueBottomRight = colorBlueTopRight = (flag3 ? f2 : 1.0f) * 0.8f;
		colorRedTopLeft *= f6;
		colorGreenTopLeft *= f6;
		colorBlueTopLeft *= f6;
		colorRedBottomLeft *= f13;
		colorGreenBottomLeft *= f13;
		colorBlueBottomLeft *= f13;
		colorRedBottomRight *= f20;
		colorGreenBottomRight *= f20;
		colorBlueBottomRight *= f20;
		colorRedTopRight *= f27;
		colorGreenTopRight *= f27;
		colorBlueTopRight *= f27;
		int_t l = block->getBlockTexture(blockAccess, i, j, k, 2);
		renderEastFace(block, i, j, k, l);
		if (fancyGrass && l == 3 && overrideBlockTexture < 0)
		{
			colorRedTopLeft *= f;
			colorRedBottomLeft *= f;
			colorRedBottomRight *= f;
			colorRedTopRight *= f;
			colorGreenTopLeft *= f1;
			colorGreenBottomLeft *= f1;
			colorGreenBottomRight *= f1;
			colorGreenTopRight *= f1;
			colorBlueTopLeft *= f2;
			colorBlueBottomLeft *= f2;
			colorBlueBottomRight *= f2;
			colorBlueTopRight *= f2;
			renderEastFace(block, i, j, k, 38);
		}
		flag = true;
	}
	if (renderAllFaces || block->shouldSideBeRendered(blockAccess, i, j, k + 1, 3))
	{
		float f7;
		float f14;
		float f21;
		float f28;
		if (lightingQuality > 0)
		{
			k++;
			aoLightValueXNegSouth = block->getBlockBrightness(blockAccess, i - 1, j, k);
			aoLightValueXPosSouth = block->getBlockBrightness(blockAccess, i + 1, j, k);
			aoLightValueXYZPNN = block->getBlockBrightness(blockAccess, i, j - 1, k);
			aoLightValueZPosUp = block->getBlockBrightness(blockAccess, i, j + 1, k);
			if (canBlockGrassBelowSouth || canBlockGrassBelowSouthEast)
			{
				aoLightValueXYZNNP = block->getBlockBrightness(blockAccess, i - 1, j - 1, k);
			}
			else
			{
				aoLightValueXYZNNP = aoLightValueXNegSouth;
			}
			if (canBlockGrassBelowSouth || canBlockGrassAboveSouth)
			{
				aoLightValueXYZPNP2 = block->getBlockBrightness(blockAccess, i - 1, j + 1, k);
			}
			else
			{
				aoLightValueXYZPNP2 = aoLightValueXNegSouth;
			}
			if (canBlockGrassBelowEast || canBlockGrassBelowSouthEast)
			{
				aoLightValueXYZPPP = block->getBlockBrightness(blockAccess, i + 1, j - 1, k);
			}
			else
			{
				aoLightValueXYZPPP = aoLightValueXPosSouth;
			}
			if (canBlockGrassBelowEast || canBlockGrassAboveSouth)
			{
				aoLightValueXYZPPP2 = block->getBlockBrightness(blockAccess, i + 1, j + 1, k);
			}
			else
			{
				aoLightValueXYZPPP2 = aoLightValueXPosSouth;
			}
			k--;
			f7 = (aoLightValueXNegSouth + aoLightValueXYZPNP2 + aoLightValueZPos + aoLightValueZPosUp) / 4.0f;
			f28 = (aoLightValueZPos + aoLightValueZPosUp + aoLightValueXPosSouth + aoLightValueXYZPPP2) / 4.0f;
			f21 = (aoLightValueXYZPNN + aoLightValueZPos + aoLightValueXYZPPP + aoLightValueXPosSouth) / 4.0f;
			f14 = (aoLightValueXYZNNP + aoLightValueXNegSouth + aoLightValueXYZPNN + aoLightValueZPos) / 4.0f;
		}
		else
		{
			f7 = f14 = f21 = f28 = aoLightValueZPos;
		}
		colorRedTopLeft = colorRedBottomLeft = colorRedBottomRight = colorRedTopRight = (flag4 ? f : 1.0f) * 0.8f;
		colorGreenTopLeft = colorGreenBottomLeft = colorGreenBottomRight = colorGreenTopRight = (flag4 ? f1 : 1.0f) * 0.8f;
		colorBlueTopLeft = colorBlueBottomLeft = colorBlueBottomRight = colorBlueTopRight = (flag4 ? f2 : 1.0f) * 0.8f;
		colorRedTopLeft *= f7;
		colorGreenTopLeft *= f7;
		colorBlueTopLeft *= f7;
		colorRedBottomLeft *= f14;
		colorGreenBottomLeft *= f14;
		colorBlueBottomLeft *= f14;
		colorRedBottomRight *= f21;
		colorGreenBottomRight *= f21;
		colorBlueBottomRight *= f21;
		colorRedTopRight *= f28;
		colorGreenTopRight *= f28;
		colorBlueTopRight *= f28;
		int_t i1 = block->getBlockTexture(blockAccess, i, j, k, 3);
		renderWestFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 3));
		if (fancyGrass && i1 == 3 && overrideBlockTexture < 0)
		{
			colorRedTopLeft *= f;
			colorRedBottomLeft *= f;
			colorRedBottomRight *= f;
			colorRedTopRight *= f;
			colorGreenTopLeft *= f1;
			colorGreenBottomLeft *= f1;
			colorGreenBottomRight *= f1;
			colorGreenTopRight *= f1;
			colorBlueTopLeft *= f2;
			colorBlueBottomLeft *= f2;
			colorBlueBottomRight *= f2;
			colorBlueTopRight *= f2;
			renderWestFace(block, i, j, k, 38);
		}
		flag = true;
	}
	if (renderAllFaces || block->shouldSideBeRendered(blockAccess, i - 1, j, k, 4))
	{
		float f8;
		float f15;
		float f22;
		float f29;
		if (lightingQuality > 0)
		{
			i--;
			aoLightValueXNeg2 = block->getBlockBrightness(blockAccess, i, j - 1, k);
			aoLightValueXNegNorth = block->getBlockBrightness(blockAccess, i, j, k - 1);
			aoLightValueXNegSouth = block->getBlockBrightness(blockAccess, i, j, k + 1);
			aoLightValueXPosUp = block->getBlockBrightness(blockAccess, i, j + 1, k);
			if (canBlockGrassBelowNorth || canBlockGrassBelowWestSouth)
			{
				aoLightValueXYZNNN = block->getBlockBrightness(blockAccess, i, j - 1, k - 1);
			}
			else
			{
				aoLightValueXYZNNN = aoLightValueXNegNorth;
			}
			if (canBlockGrassBelowSouth || canBlockGrassBelowWestSouth)
			{
				aoLightValueXYZNNP = block->getBlockBrightness(blockAccess, i, j - 1, k + 1);
			}
			else
			{
				aoLightValueXYZNNP = aoLightValueXNegSouth;
			}
			if (canBlockGrassBelowNorth || canBlockGrassAboveWest)
			{
				aoLightValueXYZNNP2 = block->getBlockBrightness(blockAccess, i, j + 1, k - 1);
			}
			else
			{
				aoLightValueXYZNNP2 = aoLightValueXNegNorth;
			}
			if (canBlockGrassBelowSouth || canBlockGrassAboveWest)
			{
				aoLightValueXYZPNP2 = block->getBlockBrightness(blockAccess, i, j + 1, k + 1);
			}
			else
			{
				aoLightValueXYZPNP2 = aoLightValueXNegSouth;
			}
			i++;
			f29 = (aoLightValueXNeg2 + aoLightValueXYZNNP + aoLightValueXNeg + aoLightValueXNegSouth) / 4.0f;
			f8 = (aoLightValueXNeg + aoLightValueXNegSouth + aoLightValueXPosUp + aoLightValueXYZPNP2) / 4.0f;
			f15 = (aoLightValueXNegNorth + aoLightValueXNeg + aoLightValueXYZNNP2 + aoLightValueXPosUp) / 4.0f;
			f22 = (aoLightValueXYZNNN + aoLightValueXNeg2 + aoLightValueXNegNorth + aoLightValueXNeg) / 4.0f;
		}
		else
		{
			f8 = f15 = f22 = f29 = aoLightValueXNeg;
		}
		colorRedTopLeft = colorRedBottomLeft = colorRedBottomRight = colorRedTopRight = (flag5 ? f : 1.0f) * 0.6f;
		colorGreenTopLeft = colorGreenBottomLeft = colorGreenBottomRight = colorGreenTopRight = (flag5 ? f1 : 1.0f) * 0.6f;
		colorBlueTopLeft = colorBlueBottomLeft = colorBlueBottomRight = colorBlueTopRight = (flag5 ? f2 : 1.0f) * 0.6f;
		colorRedTopLeft *= f8;
		colorGreenTopLeft *= f8;
		colorBlueTopLeft *= f8;
		colorRedBottomLeft *= f15;
		colorGreenBottomLeft *= f15;
		colorBlueBottomLeft *= f15;
		colorRedBottomRight *= f22;
		colorGreenBottomRight *= f22;
		colorBlueBottomRight *= f22;
		colorRedTopRight *= f29;
		colorGreenTopRight *= f29;
		colorBlueTopRight *= f29;
		int_t j1 = block->getBlockTexture(blockAccess, i, j, k, 4);
		renderNorthFace(block, i, j, k, j1);
		if (fancyGrass && j1 == 3 && overrideBlockTexture < 0)
		{
			colorRedTopLeft *= f;
			colorRedBottomLeft *= f;
			colorRedBottomRight *= f;
			colorRedTopRight *= f;
			colorGreenTopLeft *= f1;
			colorGreenBottomLeft *= f1;
			colorGreenBottomRight *= f1;
			colorGreenTopRight *= f1;
			colorBlueTopLeft *= f2;
			colorBlueBottomLeft *= f2;
			colorBlueBottomRight *= f2;
			colorBlueTopRight *= f2;
			renderNorthFace(block, i, j, k, 38);
		}
		flag = true;
	}
	if (renderAllFaces || block->shouldSideBeRendered(blockAccess, i + 1, j, k, 5))
	{
		float f9;
		float f16;
		float f23;
		float f30;
		if (lightingQuality > 0)
		{
			i++;
			aoLightValueXPos2 = block->getBlockBrightness(blockAccess, i, j - 1, k);
			aoLightValueXPosNorth = block->getBlockBrightness(blockAccess, i, j, k - 1);
			aoLightValueXPosSouth = block->getBlockBrightness(blockAccess, i, j, k + 1);
			aoLightValueXPosUp2 = block->getBlockBrightness(blockAccess, i, j + 1, k);
			if (canBlockGrassBelowEastSouth || canBlockGrassBelowWest)
			{
				aoLightValueXYZPNP = block->getBlockBrightness(blockAccess, i, j - 1, k - 1);
			}
			else
			{
				aoLightValueXYZPNP = aoLightValueXPosNorth;
			}
			if (canBlockGrassBelowEastSouth || canBlockGrassBelowEast)
			{
				aoLightValueXYZPPP = block->getBlockBrightness(blockAccess, i, j - 1, k + 1);
			}
			else
			{
				aoLightValueXYZPPP = aoLightValueXPosSouth;
			}
			if (canBlockGrassAboveEast || canBlockGrassBelowWest)
			{
				aoLightValueXYZPPN = block->getBlockBrightness(blockAccess, i, j + 1, k - 1);
			}
			else
			{
				aoLightValueXYZPPN = aoLightValueXPosNorth;
			}
			if (canBlockGrassAboveEast || canBlockGrassBelowEast)
			{
				aoLightValueXYZPPP2 = block->getBlockBrightness(blockAccess, i, j + 1, k + 1);
			}
			else
			{
				aoLightValueXYZPPP2 = aoLightValueXPosSouth;
			}
			i--;
			f9 = (aoLightValueXPos2 + aoLightValueXYZPPP + aoLightValueXPos + aoLightValueXPosSouth) / 4.0f;
			f30 = (aoLightValueXPos + aoLightValueXPosSouth + aoLightValueXPosUp2 + aoLightValueXYZPPP2) / 4.0f;
			f23 = (aoLightValueXPosNorth + aoLightValueXPos + aoLightValueXYZPPN + aoLightValueXPosUp2) / 4.0f;
			f16 = (aoLightValueXYZPNP + aoLightValueXPos2 + aoLightValueXPosNorth + aoLightValueXPos) / 4.0f;
		}
		else
		{
			f9 = f16 = f23 = f30 = aoLightValueXPos;
		}
		colorRedTopLeft = colorRedBottomLeft = colorRedBottomRight = colorRedTopRight = (flag6 ? f : 1.0f) * 0.6f;
		colorGreenTopLeft = colorGreenBottomLeft = colorGreenBottomRight = colorGreenTopRight = (flag6 ? f1 : 1.0f) * 0.6f;
		colorBlueTopLeft = colorBlueBottomLeft = colorBlueBottomRight = colorBlueTopRight = (flag6 ? f2 : 1.0f) * 0.6f;
		colorRedTopLeft *= f9;
		colorGreenTopLeft *= f9;
		colorBlueTopLeft *= f9;
		colorRedBottomLeft *= f16;
		colorGreenBottomLeft *= f16;
		colorBlueBottomLeft *= f16;
		colorRedBottomRight *= f23;
		colorGreenBottomRight *= f23;
		colorBlueBottomRight *= f23;
		colorRedTopRight *= f30;
		colorGreenTopRight *= f30;
		colorBlueTopRight *= f30;
		int_t k1 = block->getBlockTexture(blockAccess, i, j, k, 5);
		renderSouthFace(block, i, j, k, k1);
		if (fancyGrass && k1 == 3 && overrideBlockTexture < 0)
		{
			colorRedTopLeft *= f;
			colorRedBottomLeft *= f;
			colorRedBottomRight *= f;
			colorRedTopRight *= f;
			colorGreenTopLeft *= f1;
			colorGreenBottomLeft *= f1;
			colorGreenBottomRight *= f1;
			colorGreenTopRight *= f1;
			colorBlueTopLeft *= f2;
			colorBlueBottomLeft *= f2;
			colorBlueBottomRight *= f2;
			colorBlueTopRight *= f2;
			renderSouthFace(block, i, j, k, 38);
		}
		flag = true;
	}
	enableAO = false;
	return flag;
}

bool RenderBlocks::renderStandardBlockWithColorMultiplier(Block *block, int_t i, int_t j, int_t k, float f, float f1, float f2)
{
	enableAO = false;
	Tessellator *tessellator = &Tessellator::instance;
	bool flag = false;
	float f3 = 0.5f;
	float f4 = 1.0f;
	float f5 = 0.8f;
	float f6 = 0.6f;
	float f7 = f4 * f;
	float f8 = f4 * f1;
	float f9 = f4 * f2;
	float f10 = f3;
	float f11 = f5;
	float f12 = f6;
	float f13 = f3;
	float f14 = f5;
	float f15 = f6;
	float f16 = f3;
	float f17 = f5;
	float f18 = f6;
	if (block != static_cast<Block*>(Block::grass))
	{
		f10 *= f;
		f11 *= f;
		f12 *= f;
		f13 *= f1;
		f14 *= f1;
		f15 *= f1;
		f16 *= f2;
		f17 *= f2;
		f18 *= f2;
	}
	float f19 = block->getBlockBrightness(blockAccess, i, j, k);
	if (renderAllFaces || block->shouldSideBeRendered(blockAccess, i, j - 1, k, 0))
	{
		float f20 = block->getBlockBrightness(blockAccess, i, j - 1, k);
		tessellator->setColorOpaque_F(f10 * f20, f13 * f20, f16 * f20);
		renderBottomFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 0));
		flag = true;
	}
	if (renderAllFaces || block->shouldSideBeRendered(blockAccess, i, j + 1, k, 1))
	{
		float f21 = block->getBlockBrightness(blockAccess, i, j + 1, k);
		if (block->maxY != 1.0 && !block->blockMaterial->getIsLiquid())
		{
			f21 = f19;
		}
		tessellator->setColorOpaque_F(f7 * f21, f8 * f21, f9 * f21);
		renderTopFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 1));
		flag = true;
	}
	if (renderAllFaces || block->shouldSideBeRendered(blockAccess, i, j, k - 1, 2))
	{
		float f22 = block->getBlockBrightness(blockAccess, i, j, k - 1);
		if (block->minZ > 0.0)
		{
			f22 = f19;
		}
		tessellator->setColorOpaque_F(f11 * f22, f14 * f22, f17 * f22);
		int_t l = block->getBlockTexture(blockAccess, i, j, k, 2);
		renderEastFace(block, i, j, k, l);
		if (fancyGrass && l == 3 && overrideBlockTexture < 0)
		{
			tessellator->setColorOpaque_F(f11 * f22 * f, f14 * f22 * f1, f17 * f22 * f2);
			renderEastFace(block, i, j, k, 38);
		}
		flag = true;
	}
	if (renderAllFaces || block->shouldSideBeRendered(blockAccess, i, j, k + 1, 3))
	{
		float f23 = block->getBlockBrightness(blockAccess, i, j, k + 1);
		if (block->maxZ < 1.0)
		{
			f23 = f19;
		}
		tessellator->setColorOpaque_F(f11 * f23, f14 * f23, f17 * f23);
		int_t i1 = block->getBlockTexture(blockAccess, i, j, k, 3);
		renderWestFace(block, i, j, k, i1);
		if (fancyGrass && i1 == 3 && overrideBlockTexture < 0)
		{
			tessellator->setColorOpaque_F(f11 * f23 * f, f14 * f23 * f1, f17 * f23 * f2);
			renderWestFace(block, i, j, k, 38);
		}
		flag = true;
	}
	if (renderAllFaces || block->shouldSideBeRendered(blockAccess, i - 1, j, k, 4))
	{
		float f24 = block->getBlockBrightness(blockAccess, i - 1, j, k);
		if (block->minX > 0.0)
		{
			f24 = f19;
		}
		tessellator->setColorOpaque_F(f12 * f24, f15 * f24, f18 * f24);
		int_t j1 = block->getBlockTexture(blockAccess, i, j, k, 4);
		renderNorthFace(block, i, j, k, j1);
		if (fancyGrass && j1 == 3 && overrideBlockTexture < 0)
		{
			tessellator->setColorOpaque_F(f12 * f24 * f, f15 * f24 * f1, f18 * f24 * f2);
			renderNorthFace(block, i, j, k, 38);
		}
		flag = true;
	}
	if (renderAllFaces || block->shouldSideBeRendered(blockAccess, i + 1, j, k, 5))
	{
		float f25 = block->getBlockBrightness(blockAccess, i + 1, j, k);
		if (block->maxX < 1.0)
		{
			f25 = f19;
		}
		tessellator->setColorOpaque_F(f12 * f25, f15 * f25, f18 * f25);
		int_t k1 = block->getBlockTexture(blockAccess, i, j, k, 5);
		renderSouthFace(block, i, j, k, k1);
		if (fancyGrass && k1 == 3 && overrideBlockTexture < 0)
		{
			tessellator->setColorOpaque_F(f12 * f25 * f, f15 * f25 * f1, f18 * f25 * f2);
			renderSouthFace(block, i, j, k, 38);
		}
		flag = true;
	}
	return flag;
}

bool RenderBlocks::renderBlockCactus(Block *block, int_t i, int_t j, int_t k)
{
	int_t l = block->colorMultiplier(blockAccess, i, j, k);
	float f = (float)(l >> 16 & 0xff) / 255.0f;
	float f1 = (float)(l >> 8 & 0xff) / 255.0f;
	float f2 = (float)(l & 0xff) / 255.0f;
	if (Minecraft::isAmbientOcclusionEnabled())
	{
		float f3 = (f * 30.0f + f1 * 59.0f + f2 * 11.0f) / 100.0f;
		float f4 = (f * 30.0f + f1 * 70.0f) / 100.0f;
		float f5 = (f * 30.0f + f2 * 70.0f) / 100.0f;
		f = f3;
		f1 = f4;
		f2 = f5;
	}
	return renderCactusWithColorMultiplier(block, i, j, k, f, f1, f2);
}

bool RenderBlocks::renderCactusWithColorMultiplier(Block *block, int_t i, int_t j, int_t k, float f, float f1, float f2)
{
	Tessellator *tessellator = &Tessellator::instance;
	bool flag = false;
	float f3 = 0.5f;
	float f4 = 1.0f;
	float f5 = 0.8f;
	float f6 = 0.6f;
	float f7 = f3 * f;
	float f8 = f4 * f;
	float f9 = f5 * f;
	float f10 = f6 * f;
	float f11 = f3 * f1;
	float f12 = f4 * f1;
	float f13 = f5 * f1;
	float f14 = f6 * f1;
	float f15 = f3 * f2;
	float f16 = f4 * f2;
	float f17 = f5 * f2;
	float f18 = f6 * f2;
	float f19 = 0.0625f;
	float f20 = block->getBlockBrightness(blockAccess, i, j, k);
	if (renderAllFaces || block->shouldSideBeRendered(blockAccess, i, j - 1, k, 0))
	{
		float f21 = block->getBlockBrightness(blockAccess, i, j - 1, k);
		tessellator->setColorOpaque_F(f7 * f21, f11 * f21, f15 * f21);
		renderBottomFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 0));
		flag = true;
	}
	if (renderAllFaces || block->shouldSideBeRendered(blockAccess, i, j + 1, k, 1))
	{
		float f22 = block->getBlockBrightness(blockAccess, i, j + 1, k);
		if (block->maxY != 1.0 && !block->blockMaterial->getIsLiquid())
		{
			f22 = f20;
		}
		tessellator->setColorOpaque_F(f8 * f22, f12 * f22, f16 * f22);
		renderTopFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 1));
		flag = true;
	}
	if (renderAllFaces || block->shouldSideBeRendered(blockAccess, i, j, k - 1, 2))
	{
		float f23 = block->getBlockBrightness(blockAccess, i, j, k - 1);
		if (block->minZ > 0.0)
		{
			f23 = f20;
		}
		tessellator->setColorOpaque_F(f9 * f23, f13 * f23, f17 * f23);
		tessellator->setTranslationF(0.0f, 0.0f, f19);
		renderEastFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 2));
		tessellator->setTranslationF(0.0f, 0.0f, -f19);
		flag = true;
	}
	if (renderAllFaces || block->shouldSideBeRendered(blockAccess, i, j, k + 1, 3))
	{
		float f24 = block->getBlockBrightness(blockAccess, i, j, k + 1);
		if (block->maxZ < 1.0)
		{
			f24 = f20;
		}
		tessellator->setColorOpaque_F(f9 * f24, f13 * f24, f17 * f24);
		tessellator->setTranslationF(0.0f, 0.0f, -f19);
		renderWestFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 3));
		tessellator->setTranslationF(0.0f, 0.0f, f19);
		flag = true;
	}
	if (renderAllFaces || block->shouldSideBeRendered(blockAccess, i - 1, j, k, 4))
	{
		float f25 = block->getBlockBrightness(blockAccess, i - 1, j, k);
		if (block->minX > 0.0)
		{
			f25 = f20;
		}
		tessellator->setColorOpaque_F(f10 * f25, f14 * f25, f18 * f25);
		tessellator->setTranslationF(f19, 0.0f, 0.0f);
		renderNorthFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 4));
		tessellator->setTranslationF(-f19, 0.0f, 0.0f);
		flag = true;
	}
	if (renderAllFaces || block->shouldSideBeRendered(blockAccess, i + 1, j, k, 5))
	{
		float f26 = block->getBlockBrightness(blockAccess, i + 1, j, k);
		if (block->maxX < 1.0)
		{
			f26 = f20;
		}
		tessellator->setColorOpaque_F(f10 * f26, f14 * f26, f18 * f26);
		tessellator->setTranslationF(-f19, 0.0f, 0.0f);
		renderSouthFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 5));
		tessellator->setTranslationF(f19, 0.0f, 0.0f);
		flag = true;
	}
	return flag;
}

bool RenderBlocks::renderBlockFence(Block *block, int_t i, int_t j, int_t k)
{
	bool flag = false;
	float f = 0.375f;
	float f1 = 0.625f;
	block->setBlockBounds(f, 0.0f, f, f1, 1.0f, f1);
	renderStandardBlock(block, i, j, k);
	flag = true;
	bool flag1 = false;
	bool flag2 = false;
	if (blockAccess->getBlockId(i - 1, j, k) == block->blockID || blockAccess->getBlockId(i + 1, j, k) == block->blockID)
	{
		flag1 = true;
	}
	if (blockAccess->getBlockId(i, j, k - 1) == block->blockID || blockAccess->getBlockId(i, j, k + 1) == block->blockID)
	{
		flag2 = true;
	}
	bool flag3 = blockAccess->getBlockId(i - 1, j, k) == block->blockID;
	bool flag4 = blockAccess->getBlockId(i + 1, j, k) == block->blockID;
	bool flag5 = blockAccess->getBlockId(i, j, k - 1) == block->blockID;
	bool flag6 = blockAccess->getBlockId(i, j, k + 1) == block->blockID;
	if (!flag1 && !flag2)
	{
		flag1 = true;
	}
	f = 0.4375f;
	f1 = 0.5625f;
	float f2 = 0.75f;
	float f3 = 0.9375f;
	float f4 = flag3 ? 0.0f : f;
	float f5 = flag4 ? 1.0f : f1;
	float f6 = flag5 ? 0.0f : f;
	float f7 = flag6 ? 1.0f : f1;
	if (flag1)
	{
		block->setBlockBounds(f4, f2, f, f5, f3, f1);
		renderStandardBlock(block, i, j, k);
		flag = true;
	}
	if (flag2)
	{
		block->setBlockBounds(f, f2, f6, f1, f3, f7);
		renderStandardBlock(block, i, j, k);
		flag = true;
	}
	f2 = 0.375f;
	f3 = 0.5625f;
	if (flag1)
	{
		block->setBlockBounds(f4, f2, f, f5, f3, f1);
		renderStandardBlock(block, i, j, k);
		flag = true;
	}
	if (flag2)
	{
		block->setBlockBounds(f, f2, f6, f1, f3, f7);
		renderStandardBlock(block, i, j, k);
		flag = true;
	}
	block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	return flag;
}

bool RenderBlocks::renderBlockStairs(Block *block, int_t i, int_t j, int_t k)
{
	bool flag = false;
	int_t l = blockAccess->getBlockMetadata(i, j, k);
	if (l == 0)
	{
		block->setBlockBounds(0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 1.0f);
		renderStandardBlock(block, i, j, k);
		block->setBlockBounds(0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
		renderStandardBlock(block, i, j, k);
		flag = true;
	}
	else if (l == 1)
	{
		block->setBlockBounds(0.0f, 0.0f, 0.0f, 0.5f, 1.0f, 1.0f);
		renderStandardBlock(block, i, j, k);
		block->setBlockBounds(0.5f, 0.0f, 0.0f, 1.0f, 0.5f, 1.0f);
		renderStandardBlock(block, i, j, k);
		flag = true;
	}
	else if (l == 2)
	{
		block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f);
		renderStandardBlock(block, i, j, k);
		block->setBlockBounds(0.0f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f);
		renderStandardBlock(block, i, j, k);
		flag = true;
	}
	else if (l == 3)
	{
		block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
		renderStandardBlock(block, i, j, k);
		block->setBlockBounds(0.0f, 0.0f, 0.5f, 1.0f, 0.5f, 1.0f);
		renderStandardBlock(block, i, j, k);
		flag = true;
	}
	block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	return flag;
}

bool RenderBlocks::renderBlockDoor(Block *block, int_t i, int_t j, int_t k)
{
	Tessellator *tessellator = &Tessellator::instance;
	BlockDoor *blockdoor = (BlockDoor *)block;
	bool flag = false;
	float f = 0.5f;
	float f1 = 1.0f;
	float f2 = 0.8f;
	float f3 = 0.6f;
	float f4 = block->getBlockBrightness(blockAccess, i, j, k);
	float f5 = block->getBlockBrightness(blockAccess, i, j - 1, k);
	if (blockdoor->minY > 0.0)
	{
		f5 = f4;
	}
	if (Block::lightValue[block->blockID] > 0)
	{
		f5 = 1.0f;
	}
	tessellator->setColorOpaque_F(f * f5, f * f5, f * f5);
	renderBottomFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 0));
	flag = true;
	f5 = block->getBlockBrightness(blockAccess, i, j + 1, k);
	if (blockdoor->maxY < 1.0)
	{
		f5 = f4;
	}
	if (Block::lightValue[block->blockID] > 0)
	{
		f5 = 1.0f;
	}
	tessellator->setColorOpaque_F(f1 * f5, f1 * f5, f1 * f5);
	renderTopFace(block, i, j, k, block->getBlockTexture(blockAccess, i, j, k, 1));
	flag = true;
	f5 = block->getBlockBrightness(blockAccess, i, j, k - 1);
	if (blockdoor->minZ > 0.0)
	{
		f5 = f4;
	}
	if (Block::lightValue[block->blockID] > 0)
	{
		f5 = 1.0f;
	}
	tessellator->setColorOpaque_F(f2 * f5, f2 * f5, f2 * f5);
	int_t l = block->getBlockTexture(blockAccess, i, j, k, 2);
	if (l < 0)
	{
		flipTexture = true;
		l = -l;
	}
	renderEastFace(block, i, j, k, l);
	flag = true;
	flipTexture = false;
	f5 = block->getBlockBrightness(blockAccess, i, j, k + 1);
	if (blockdoor->maxZ < 1.0)
	{
		f5 = f4;
	}
	if (Block::lightValue[block->blockID] > 0)
	{
		f5 = 1.0f;
	}
	tessellator->setColorOpaque_F(f2 * f5, f2 * f5, f2 * f5);
	l = block->getBlockTexture(blockAccess, i, j, k, 3);
	if (l < 0)
	{
		flipTexture = true;
		l = -l;
	}
	renderWestFace(block, i, j, k, l);
	flag = true;
	flipTexture = false;
	f5 = block->getBlockBrightness(blockAccess, i - 1, j, k);
	if (blockdoor->minX > 0.0)
	{
		f5 = f4;
	}
	if (Block::lightValue[block->blockID] > 0)
	{
		f5 = 1.0f;
	}
	tessellator->setColorOpaque_F(f3 * f5, f3 * f5, f3 * f5);
	l = block->getBlockTexture(blockAccess, i, j, k, 4);
	if (l < 0)
	{
		flipTexture = true;
		l = -l;
	}
	renderNorthFace(block, i, j, k, l);
	flag = true;
	flipTexture = false;
	f5 = block->getBlockBrightness(blockAccess, i + 1, j, k);
	if (blockdoor->maxX < 1.0)
	{
		f5 = f4;
	}
	if (Block::lightValue[block->blockID] > 0)
	{
		f5 = 1.0f;
	}
	tessellator->setColorOpaque_F(f3 * f5, f3 * f5, f3 * f5);
	l = block->getBlockTexture(blockAccess, i, j, k, 5);
	if (l < 0)
	{
		flipTexture = true;
		l = -l;
	}
	renderSouthFace(block, i, j, k, l);
	flag = true;
	flipTexture = false;
	return flag;
}

void RenderBlocks::renderBottomFace(Block *block, double d, double d1, double d2, int_t i)
{
	Tessellator *tessellator = &Tessellator::instance;
	if (overrideBlockTexture >= 0)
	{
		i = overrideBlockTexture;
	}
	int_t j = (i & 0xf) << 4;
	int_t k = i & 0xf0;
	double d3 = ((double)j + block->minX * 16.0) / 256.0;
	double d4 = (((double)j + block->maxX * 16.0) - 0.01) / 256.0;
	double d5 = ((double)k + block->minZ * 16.0) / 256.0;
	double d6 = (((double)k + block->maxZ * 16.0) - 0.01) / 256.0;
	if (block->minX < 0.0 || block->maxX > 1.0)
	{
		d3 = ((float)j + 0.0f) / 256.0f;
		d4 = ((float)j + 15.99f) / 256.0f;
	}
	if (block->minZ < 0.0 || block->maxZ > 1.0)
	{
		d5 = ((float)k + 0.0f) / 256.0f;
		d6 = ((float)k + 15.99f) / 256.0f;
	}
	double d7 = d4;
	double d8 = d3;
	double d9 = d5;
	double d10 = d6;
	if (bottomFaceRotation == 2)
	{
		d3 = ((double)j + block->minZ * 16.0) / 256.0;
		d5 = ((double)(k + 16) - block->maxX * 16.0) / 256.0;
		d4 = ((double)j + block->maxZ * 16.0) / 256.0;
		d6 = ((double)(k + 16) - block->minX * 16.0) / 256.0;
		d7 = d4;
		d8 = d3;
		d9 = d5;
		d10 = d6;
		d7 = d3;
		d8 = d4;
		d5 = d6;
		d6 = d9;
	}
	else if (bottomFaceRotation == 1)
	{
		d3 = ((double)(j + 16) - block->maxZ * 16.0) / 256.0;
		d5 = ((double)k + block->minX * 16.0) / 256.0;
		d4 = ((double)(j + 16) - block->minZ * 16.0) / 256.0;
		d6 = ((double)k + block->maxX * 16.0) / 256.0;
		d7 = d4;
		d8 = d3;
		d9 = d5;
		d10 = d6;
		d3 = d7;
		d4 = d8;
		d9 = d6;
		d10 = d5;
	}
	else if (bottomFaceRotation == 3)
	{
		d3 = ((double)(j + 16) - block->minX * 16.0) / 256.0;
		d4 = ((double)(j + 16) - block->maxX * 16.0 - 0.01) / 256.0;
		d5 = ((double)(k + 16) - block->minZ * 16.0) / 256.0;
		d6 = ((double)(k + 16) - block->maxZ * 16.0 - 0.01) / 256.0;
		d7 = d4;
		d8 = d3;
		d9 = d5;
		d10 = d6;
	}
	double d11 = d + block->minX;
	double d12 = d + block->maxX;
	double d13 = d1 + block->minY;
	double d14 = d2 + block->minZ;
	double d15 = d2 + block->maxZ;
	if (enableAO)
	{
		tessellator->setColorOpaque_F(colorRedTopLeft, colorGreenTopLeft, colorBlueTopLeft);
		tessellator->addVertexWithUV(d11, d13, d15, d8, d10);
		tessellator->setColorOpaque_F(colorRedBottomLeft, colorGreenBottomLeft, colorBlueBottomLeft);
		tessellator->addVertexWithUV(d11, d13, d14, d3, d5);
		tessellator->setColorOpaque_F(colorRedBottomRight, colorGreenBottomRight, colorBlueBottomRight);
		tessellator->addVertexWithUV(d12, d13, d14, d7, d9);
		tessellator->setColorOpaque_F(colorRedTopRight, colorGreenTopRight, colorBlueTopRight);
		tessellator->addVertexWithUV(d12, d13, d15, d4, d6);
	}
	else
	{
		tessellator->addVertexWithUV(d11, d13, d15, d8, d10);
		tessellator->addVertexWithUV(d11, d13, d14, d3, d5);
		tessellator->addVertexWithUV(d12, d13, d14, d7, d9);
		tessellator->addVertexWithUV(d12, d13, d15, d4, d6);
	}
}

void RenderBlocks::renderTopFace(Block *block, double d, double d1, double d2, int_t i)
{
	Tessellator *tessellator = &Tessellator::instance;
	if (overrideBlockTexture >= 0)
	{
		i = overrideBlockTexture;
	}
	int_t j = (i & 0xf) << 4;
	int_t k = i & 0xf0;
	double d3 = ((double)j + block->minX * 16.0) / 256.0;
	double d4 = (((double)j + block->maxX * 16.0) - 0.01) / 256.0;
	double d5 = ((double)k + block->minZ * 16.0) / 256.0;
	double d6 = (((double)k + block->maxZ * 16.0) - 0.01) / 256.0;
	if (block->minX < 0.0 || block->maxX > 1.0)
	{
		d3 = ((float)j + 0.0f) / 256.0f;
		d4 = ((float)j + 15.99f) / 256.0f;
	}
	if (block->minZ < 0.0 || block->maxZ > 1.0)
	{
		d5 = ((float)k + 0.0f) / 256.0f;
		d6 = ((float)k + 15.99f) / 256.0f;
	}
	double d7 = d4;
	double d8 = d3;
	double d9 = d5;
	double d10 = d6;
	if (topFaceRotation == 1)
	{
		d3 = ((double)j + block->minZ * 16.0) / 256.0;
		d5 = ((double)(k + 16) - block->maxX * 16.0) / 256.0;
		d4 = ((double)j + block->maxZ * 16.0) / 256.0;
		d6 = ((double)(k + 16) - block->minX * 16.0) / 256.0;
		d7 = d4;
		d8 = d3;
		d9 = d5;
		d10 = d6;
		d7 = d3;
		d8 = d4;
		d5 = d6;
		d6 = d9;
	}
	else if (topFaceRotation == 2)
	{
		d3 = ((double)(j + 16) - block->maxZ * 16.0) / 256.0;
		d5 = ((double)k + block->minX * 16.0) / 256.0;
		d4 = ((double)(j + 16) - block->minZ * 16.0) / 256.0;
		d6 = ((double)k + block->maxX * 16.0) / 256.0;
		d7 = d4;
		d8 = d3;
		d9 = d5;
		d10 = d6;
		d3 = d7;
		d4 = d8;
		d9 = d6;
		d10 = d5;
	}
	else if (topFaceRotation == 3)
	{
		d3 = ((double)(j + 16) - block->minX * 16.0) / 256.0;
		d4 = ((double)(j + 16) - block->maxX * 16.0 - 0.01) / 256.0;
		d5 = ((double)(k + 16) - block->minZ * 16.0) / 256.0;
		d6 = ((double)(k + 16) - block->maxZ * 16.0 - 0.01) / 256.0;
		d7 = d4;
		d8 = d3;
		d9 = d5;
		d10 = d6;
	}
	double d11 = d + block->minX;
	double d12 = d + block->maxX;
	double d13 = d1 + block->maxY;
	double d14 = d2 + block->minZ;
	double d15 = d2 + block->maxZ;
	if (enableAO)
	{
		tessellator->setColorOpaque_F(colorRedTopLeft, colorGreenTopLeft, colorBlueTopLeft);
		tessellator->addVertexWithUV(d12, d13, d15, d4, d6);
		tessellator->setColorOpaque_F(colorRedBottomLeft, colorGreenBottomLeft, colorBlueBottomLeft);
		tessellator->addVertexWithUV(d12, d13, d14, d7, d9);
		tessellator->setColorOpaque_F(colorRedBottomRight, colorGreenBottomRight, colorBlueBottomRight);
		tessellator->addVertexWithUV(d11, d13, d14, d3, d5);
		tessellator->setColorOpaque_F(colorRedTopRight, colorGreenTopRight, colorBlueTopRight);
		tessellator->addVertexWithUV(d11, d13, d15, d8, d10);
	}
	else
	{
		tessellator->addVertexWithUV(d12, d13, d15, d4, d6);
		tessellator->addVertexWithUV(d12, d13, d14, d7, d9);
		tessellator->addVertexWithUV(d11, d13, d14, d3, d5);
		tessellator->addVertexWithUV(d11, d13, d15, d8, d10);
	}
}

void RenderBlocks::renderEastFace(Block *block, double d, double d1, double d2, int_t i)
{
	Tessellator *tessellator = &Tessellator::instance;
	if (overrideBlockTexture >= 0)
	{
		i = overrideBlockTexture;
	}
	int_t j = (i & 0xf) << 4;
	int_t k = i & 0xf0;
	double d3 = ((double)j + block->minX * 16.0) / 256.0;
	double d4 = (((double)j + block->maxX * 16.0) - 0.01) / 256.0;
	double d5 = ((double)(k + 16) - block->maxY * 16.0) / 256.0;
	double d6 = ((double)(k + 16) - block->minY * 16.0 - 0.01) / 256.0;
	if (flipTexture)
	{
		double d7 = d3;
		d3 = d4;
		d4 = d7;
	}
	if (block->minX < 0.0 || block->maxX > 1.0)
	{
		d3 = ((float)j + 0.0f) / 256.0f;
		d4 = ((float)j + 15.99f) / 256.0f;
	}
	if (block->minY < 0.0 || block->maxY > 1.0)
	{
		d5 = ((float)k + 0.0f) / 256.0f;
		d6 = ((float)k + 15.99f) / 256.0f;
	}
	double d8 = d4;
	double d9 = d3;
	double d10 = d5;
	double d11 = d6;
	if (eastFaceRotation == 2)
	{
		d3 = ((double)j + block->minY * 16.0) / 256.0;
		d5 = ((double)(k + 16) - block->minX * 16.0) / 256.0;
		d4 = ((double)j + block->maxY * 16.0) / 256.0;
		d6 = ((double)(k + 16) - block->maxX * 16.0) / 256.0;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
		d8 = d3;
		d9 = d4;
		d5 = d6;
		d6 = d10;
	}
	else if (eastFaceRotation == 1)
	{
		d3 = ((double)(j + 16) - block->maxY * 16.0) / 256.0;
		d5 = ((double)k + block->maxX * 16.0) / 256.0;
		d4 = ((double)(j + 16) - block->minY * 16.0) / 256.0;
		d6 = ((double)k + block->minX * 16.0) / 256.0;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
		d3 = d8;
		d4 = d9;
		d10 = d6;
		d11 = d5;
	}
	else if (eastFaceRotation == 3)
	{
		d3 = ((double)(j + 16) - block->minX * 16.0) / 256.0;
		d4 = ((double)(j + 16) - block->maxX * 16.0 - 0.01) / 256.0;
		d5 = ((double)k + block->maxY * 16.0) / 256.0;
		d6 = (((double)k + block->minY * 16.0) - 0.01) / 256.0;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
	}
	double d12 = d + block->minX;
	double d13 = d + block->maxX;
	double d14 = d1 + block->minY;
	double d15 = d1 + block->maxY;
	double d16 = d2 + block->minZ;
	if (enableAO)
	{
		tessellator->setColorOpaque_F(colorRedTopLeft, colorGreenTopLeft, colorBlueTopLeft);
		tessellator->addVertexWithUV(d12, d15, d16, d8, d10);
		tessellator->setColorOpaque_F(colorRedBottomLeft, colorGreenBottomLeft, colorBlueBottomLeft);
		tessellator->addVertexWithUV(d13, d15, d16, d3, d5);
		tessellator->setColorOpaque_F(colorRedBottomRight, colorGreenBottomRight, colorBlueBottomRight);
		tessellator->addVertexWithUV(d13, d14, d16, d9, d11);
		tessellator->setColorOpaque_F(colorRedTopRight, colorGreenTopRight, colorBlueTopRight);
		tessellator->addVertexWithUV(d12, d14, d16, d4, d6);
	}
	else
	{
		tessellator->addVertexWithUV(d12, d15, d16, d8, d10);
		tessellator->addVertexWithUV(d13, d15, d16, d3, d5);
		tessellator->addVertexWithUV(d13, d14, d16, d9, d11);
		tessellator->addVertexWithUV(d12, d14, d16, d4, d6);
	}
}

void RenderBlocks::renderWestFace(Block *block, double d, double d1, double d2, int_t i)
{
	Tessellator *tessellator = &Tessellator::instance;
	if (overrideBlockTexture >= 0)
	{
		i = overrideBlockTexture;
	}
	int_t j = (i & 0xf) << 4;
	int_t k = i & 0xf0;
	double d3 = ((double)j + block->minX * 16.0) / 256.0;
	double d4 = (((double)j + block->maxX * 16.0) - 0.01) / 256.0;
	double d5 = ((double)(k + 16) - block->maxY * 16.0) / 256.0;
	double d6 = ((double)(k + 16) - block->minY * 16.0 - 0.01) / 256.0;
	if (flipTexture)
	{
		double d7 = d3;
		d3 = d4;
		d4 = d7;
	}
	if (block->minX < 0.0 || block->maxX > 1.0)
	{
		d3 = ((float)j + 0.0f) / 256.0f;
		d4 = ((float)j + 15.99f) / 256.0f;
	}
	if (block->minY < 0.0 || block->maxY > 1.0)
	{
		d5 = ((float)k + 0.0f) / 256.0f;
		d6 = ((float)k + 15.99f) / 256.0f;
	}
	double d8 = d4;
	double d9 = d3;
	double d10 = d5;
	double d11 = d6;
	if (westFaceRotation == 1)
	{
		d3 = ((double)j + block->minY * 16.0) / 256.0;
		d6 = ((double)(k + 16) - block->minX * 16.0) / 256.0;
		d4 = ((double)j + block->maxY * 16.0) / 256.0;
		d5 = ((double)(k + 16) - block->maxX * 16.0) / 256.0;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
		d8 = d3;
		d9 = d4;
		d5 = d6;
		d6 = d10;
	}
	else if (westFaceRotation == 2)
	{
		d3 = ((double)(j + 16) - block->maxY * 16.0) / 256.0;
		d5 = ((double)k + block->minX * 16.0) / 256.0;
		d4 = ((double)(j + 16) - block->minY * 16.0) / 256.0;
		d6 = ((double)k + block->maxX * 16.0) / 256.0;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
		d3 = d8;
		d4 = d9;
		d10 = d6;
		d11 = d5;
	}
	else if (westFaceRotation == 3)
	{
		d3 = ((double)(j + 16) - block->minX * 16.0) / 256.0;
		d4 = ((double)(j + 16) - block->maxX * 16.0 - 0.01) / 256.0;
		d5 = ((double)k + block->maxY * 16.0) / 256.0;
		d6 = (((double)k + block->minY * 16.0) - 0.01) / 256.0;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
	}
	double d12 = d + block->minX;
	double d13 = d + block->maxX;
	double d14 = d1 + block->minY;
	double d15 = d1 + block->maxY;
	double d16 = d2 + block->maxZ;
	if (enableAO)
	{
		tessellator->setColorOpaque_F(colorRedTopLeft, colorGreenTopLeft, colorBlueTopLeft);
		tessellator->addVertexWithUV(d12, d15, d16, d3, d5);
		tessellator->setColorOpaque_F(colorRedBottomLeft, colorGreenBottomLeft, colorBlueBottomLeft);
		tessellator->addVertexWithUV(d12, d14, d16, d9, d11);
		tessellator->setColorOpaque_F(colorRedBottomRight, colorGreenBottomRight, colorBlueBottomRight);
		tessellator->addVertexWithUV(d13, d14, d16, d4, d6);
		tessellator->setColorOpaque_F(colorRedTopRight, colorGreenTopRight, colorBlueTopRight);
		tessellator->addVertexWithUV(d13, d15, d16, d8, d10);
	}
	else
	{
		tessellator->addVertexWithUV(d12, d15, d16, d3, d5);
		tessellator->addVertexWithUV(d12, d14, d16, d9, d11);
		tessellator->addVertexWithUV(d13, d14, d16, d4, d6);
		tessellator->addVertexWithUV(d13, d15, d16, d8, d10);
	}
}

void RenderBlocks::renderNorthFace(Block *block, double d, double d1, double d2, int_t i)
{
	Tessellator *tessellator = &Tessellator::instance;
	if (overrideBlockTexture >= 0)
	{
		i = overrideBlockTexture;
	}
	int_t j = (i & 0xf) << 4;
	int_t k = i & 0xf0;
	double d3 = ((double)j + block->minZ * 16.0) / 256.0;
	double d4 = (((double)j + block->maxZ * 16.0) - 0.01) / 256.0;
	double d5 = ((double)(k + 16) - block->maxY * 16.0) / 256.0;
	double d6 = ((double)(k + 16) - block->minY * 16.0 - 0.01) / 256.0;
	if (flipTexture)
	{
		double d7 = d3;
		d3 = d4;
		d4 = d7;
	}
	if (block->minZ < 0.0 || block->maxZ > 1.0)
	{
		d3 = ((float)j + 0.0f) / 256.0f;
		d4 = ((float)j + 15.99f) / 256.0f;
	}
	if (block->minY < 0.0 || block->maxY > 1.0)
	{
		d5 = ((float)k + 0.0f) / 256.0f;
		d6 = ((float)k + 15.99f) / 256.0f;
	}
	double d8 = d4;
	double d9 = d3;
	double d10 = d5;
	double d11 = d6;
	if (northFaceRotation == 1)
	{
		d3 = ((double)j + block->minY * 16.0) / 256.0;
		d5 = ((double)(k + 16) - block->maxZ * 16.0) / 256.0;
		d4 = ((double)j + block->maxY * 16.0) / 256.0;
		d6 = ((double)(k + 16) - block->minZ * 16.0) / 256.0;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
		d8 = d3;
		d9 = d4;
		d5 = d6;
		d6 = d10;
	}
	else if (northFaceRotation == 2)
	{
		d3 = ((double)(j + 16) - block->maxY * 16.0) / 256.0;
		d5 = ((double)k + block->minZ * 16.0) / 256.0;
		d4 = ((double)(j + 16) - block->minY * 16.0) / 256.0;
		d6 = ((double)k + block->maxZ * 16.0) / 256.0;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
		d3 = d8;
		d4 = d9;
		d10 = d6;
		d11 = d5;
	}
	else if (northFaceRotation == 3)
	{
		d3 = ((double)(j + 16) - block->minZ * 16.0) / 256.0;
		d4 = ((double)(j + 16) - block->maxZ * 16.0 - 0.01) / 256.0;
		d5 = ((double)k + block->maxY * 16.0) / 256.0;
		d6 = (((double)k + block->minY * 16.0) - 0.01) / 256.0;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
	}
	double d12 = d + block->minX;
	double d13 = d1 + block->minY;
	double d14 = d1 + block->maxY;
	double d15 = d2 + block->minZ;
	double d16 = d2 + block->maxZ;
	if (enableAO)
	{
		tessellator->setColorOpaque_F(colorRedTopLeft, colorGreenTopLeft, colorBlueTopLeft);
		tessellator->addVertexWithUV(d12, d14, d16, d8, d10);
		tessellator->setColorOpaque_F(colorRedBottomLeft, colorGreenBottomLeft, colorBlueBottomLeft);
		tessellator->addVertexWithUV(d12, d14, d15, d3, d5);
		tessellator->setColorOpaque_F(colorRedBottomRight, colorGreenBottomRight, colorBlueBottomRight);
		tessellator->addVertexWithUV(d12, d13, d15, d9, d11);
		tessellator->setColorOpaque_F(colorRedTopRight, colorGreenTopRight, colorBlueTopRight);
		tessellator->addVertexWithUV(d12, d13, d16, d4, d6);
	}
	else
	{
		tessellator->addVertexWithUV(d12, d14, d16, d8, d10);
		tessellator->addVertexWithUV(d12, d14, d15, d3, d5);
		tessellator->addVertexWithUV(d12, d13, d15, d9, d11);
		tessellator->addVertexWithUV(d12, d13, d16, d4, d6);
	}
}

void RenderBlocks::renderSouthFace(Block *block, double d, double d1, double d2, int_t i)
{
	Tessellator *tessellator = &Tessellator::instance;
	if (overrideBlockTexture >= 0)
	{
		i = overrideBlockTexture;
	}
	int_t j = (i & 0xf) << 4;
	int_t k = i & 0xf0;
	double d3 = ((double)j + block->minZ * 16.0) / 256.0;
	double d4 = (((double)j + block->maxZ * 16.0) - 0.01) / 256.0;
	double d5 = ((double)(k + 16) - block->maxY * 16.0) / 256.0;
	double d6 = ((double)(k + 16) - block->minY * 16.0 - 0.01) / 256.0;
	if (flipTexture)
	{
		double d7 = d3;
		d3 = d4;
		d4 = d7;
	}
	if (block->minZ < 0.0 || block->maxZ > 1.0)
	{
		d3 = ((float)j + 0.0f) / 256.0f;
		d4 = ((float)j + 15.99f) / 256.0f;
	}
	if (block->minY < 0.0 || block->maxY > 1.0)
	{
		d5 = ((float)k + 0.0f) / 256.0f;
		d6 = ((float)k + 15.99f) / 256.0f;
	}
	double d8 = d4;
	double d9 = d3;
	double d10 = d5;
	double d11 = d6;
	if (southFaceRotation == 2)
	{
		d3 = ((double)j + block->minY * 16.0) / 256.0;
		d5 = ((double)(k + 16) - block->minZ * 16.0) / 256.0;
		d4 = ((double)j + block->maxY * 16.0) / 256.0;
		d6 = ((double)(k + 16) - block->maxZ * 16.0) / 256.0;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
		d8 = d3;
		d9 = d4;
		d5 = d6;
		d6 = d10;
	}
	else if (southFaceRotation == 1)
	{
		d3 = ((double)(j + 16) - block->maxY * 16.0) / 256.0;
		d5 = ((double)k + block->maxZ * 16.0) / 256.0;
		d4 = ((double)(j + 16) - block->minY * 16.0) / 256.0;
		d6 = ((double)k + block->minZ * 16.0) / 256.0;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
		d3 = d8;
		d4 = d9;
		d10 = d6;
		d11 = d5;
	}
	else if (southFaceRotation == 3)
	{
		d3 = ((double)(j + 16) - block->minZ * 16.0) / 256.0;
		d4 = ((double)(j + 16) - block->maxZ * 16.0 - 0.01) / 256.0;
		d5 = ((double)k + block->maxY * 16.0) / 256.0;
		d6 = (((double)k + block->minY * 16.0) - 0.01) / 256.0;
		d8 = d4;
		d9 = d3;
		d10 = d5;
		d11 = d6;
	}
	double d12 = d + block->maxX;
	double d13 = d1 + block->minY;
	double d14 = d1 + block->maxY;
	double d15 = d2 + block->minZ;
	double d16 = d2 + block->maxZ;
	if (enableAO)
	{
		tessellator->setColorOpaque_F(colorRedTopLeft, colorGreenTopLeft, colorBlueTopLeft);
		tessellator->addVertexWithUV(d12, d13, d16, d9, d11);
		tessellator->setColorOpaque_F(colorRedBottomLeft, colorGreenBottomLeft, colorBlueBottomLeft);
		tessellator->addVertexWithUV(d12, d13, d15, d4, d6);
		tessellator->setColorOpaque_F(colorRedBottomRight, colorGreenBottomRight, colorBlueBottomRight);
		tessellator->addVertexWithUV(d12, d14, d15, d8, d10);
		tessellator->setColorOpaque_F(colorRedTopRight, colorGreenTopRight, colorBlueTopRight);
		tessellator->addVertexWithUV(d12, d14, d16, d3, d5);
	}
	else
	{
		tessellator->addVertexWithUV(d12, d13, d16, d9, d11);
		tessellator->addVertexWithUV(d12, d13, d15, d4, d6);
		tessellator->addVertexWithUV(d12, d14, d15, d8, d10);
		tessellator->addVertexWithUV(d12, d14, d16, d3, d5);
	}
}

void RenderBlocks::renderBlockOnInventory(Block *block, int_t i, float f)
{
	Tessellator *tessellator = &Tessellator::instance;
	if (field_31088_b)
	{
		int_t j = block->getRenderColor(i);
		float f1 = (float)(j >> 16 & 0xff) / 255.0f;
		float f3 = (float)(j >> 8 & 0xff) / 255.0f;
		float f5 = (float)(j & 0xff) / 255.0f;
		glColor4f(f1 * f, f3 * f, f5 * f, 1.0f);
	}
	int_t k = block->getRenderType();
	if (k == 0 || k == 16)
	{
		if (k == 16)
		{
			i = 1;
		}
		block->setBlockBoundsForItemRender();
		glTranslatef(-0.5f, -0.5f, -0.5f);
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, -1.0f, 0.0f);
		renderBottomFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSideAndMetadata(0, i));
		tessellator->draw();
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, 1.0f, 0.0f);
		renderTopFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSideAndMetadata(1, i));
		tessellator->draw();
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, 0.0f, -1.0f);
		renderEastFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSideAndMetadata(2, i));
		tessellator->draw();
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, 0.0f, 1.0f);
		renderWestFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSideAndMetadata(3, i));
		tessellator->draw();
		tessellator->startDrawingQuads();
		tessellator->setNormal(-1.0f, 0.0f, 0.0f);
		renderNorthFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSideAndMetadata(4, i));
		tessellator->draw();
		tessellator->startDrawingQuads();
		tessellator->setNormal(1.0f, 0.0f, 0.0f);
		renderSouthFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSideAndMetadata(5, i));
		tessellator->draw();
		glTranslatef(0.5f, 0.5f, 0.5f);
	}
	else if (k == 1)
	{
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, -1.0f, 0.0f);
		renderCrossedSquares(block, i, -0.5, -0.5, -0.5);
		tessellator->draw();
	}
	else if (k == 13)
	{
		block->setBlockBoundsForItemRender();
		glTranslatef(-0.5f, -0.5f, -0.5f);
		float f2 = 0.0625f;
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, -1.0f, 0.0f);
		renderBottomFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(0));
		tessellator->draw();
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, 1.0f, 0.0f);
		renderTopFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(1));
		tessellator->draw();
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, 0.0f, -1.0f);
		tessellator->setTranslationF(0.0f, 0.0f, f2);
		renderEastFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(2));
		tessellator->setTranslationF(0.0f, 0.0f, -f2);
		tessellator->draw();
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, 0.0f, 1.0f);
		tessellator->setTranslationF(0.0f, 0.0f, -f2);
		renderWestFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(3));
		tessellator->setTranslationF(0.0f, 0.0f, f2);
		tessellator->draw();
		tessellator->startDrawingQuads();
		tessellator->setNormal(-1.0f, 0.0f, 0.0f);
		tessellator->setTranslationF(f2, 0.0f, 0.0f);
		renderNorthFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(4));
		tessellator->setTranslationF(-f2, 0.0f, 0.0f);
		tessellator->draw();
		tessellator->startDrawingQuads();
		tessellator->setNormal(1.0f, 0.0f, 0.0f);
		tessellator->setTranslationF(-f2, 0.0f, 0.0f);
		renderSouthFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(5));
		tessellator->setTranslationF(f2, 0.0f, 0.0f);
		tessellator->draw();
		glTranslatef(0.5f, 0.5f, 0.5f);
	}
	else if (k == 6)
	{
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, -1.0f, 0.0f);
		renderCropsCrossed(block, i, -0.5, -0.5, -0.5);
		tessellator->draw();
	}
	else if (k == 2)
	{
		tessellator->startDrawingQuads();
		tessellator->setNormal(0.0f, -1.0f, 0.0f);
		renderTorchAtAngle(block, -0.5, -0.5, -0.5, 0.0, 0.0);
		tessellator->draw();
	}
	else if (k == 10)
	{
		for (int_t l = 0; l < 2; l++)
		{
			if (l == 0)
			{
				block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
			}
			if (l == 1)
			{
				block->setBlockBounds(0.0f, 0.0f, 0.5f, 1.0f, 0.5f, 1.0f);
			}
			glTranslatef(-0.5f, -0.5f, -0.5f);
			tessellator->startDrawingQuads();
			tessellator->setNormal(0.0f, -1.0f, 0.0f);
			renderBottomFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(0));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(0.0f, 1.0f, 0.0f);
			renderTopFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(1));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(0.0f, 0.0f, -1.0f);
			renderEastFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(2));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(0.0f, 0.0f, 1.0f);
			renderWestFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(3));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(-1.0f, 0.0f, 0.0f);
			renderNorthFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(4));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(1.0f, 0.0f, 0.0f);
			renderSouthFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(5));
			tessellator->draw();
			glTranslatef(0.5f, 0.5f, 0.5f);
		}
	}
	else if (k == 11)
	{
		for (int_t i1 = 0; i1 < 4; i1++)
		{
			float f4 = 0.125f;
			if (i1 == 0)
			{
				block->setBlockBounds(0.5f - f4, 0.0f, 0.0f, 0.5f + f4, 1.0f, f4 * 2.0f);
			}
			if (i1 == 1)
			{
				block->setBlockBounds(0.5f - f4, 0.0f, 1.0f - f4 * 2.0f, 0.5f + f4, 1.0f, 1.0f);
			}
			f4 = 0.0625f;
			if (i1 == 2)
			{
				block->setBlockBounds(0.5f - f4, 1.0f - f4 * 3.0f, -f4 * 2.0f, 0.5f + f4, 1.0f - f4, 1.0f + f4 * 2.0f);
			}
			if (i1 == 3)
			{
				block->setBlockBounds(0.5f - f4, 0.5f - f4 * 3.0f, -f4 * 2.0f, 0.5f + f4, 0.5f - f4, 1.0f + f4 * 2.0f);
			}
			glTranslatef(-0.5f, -0.5f, -0.5f);
			tessellator->startDrawingQuads();
			tessellator->setNormal(0.0f, -1.0f, 0.0f);
			renderBottomFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(0));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(0.0f, 1.0f, 0.0f);
			renderTopFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(1));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(0.0f, 0.0f, -1.0f);
			renderEastFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(2));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(0.0f, 0.0f, 1.0f);
			renderWestFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(3));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(-1.0f, 0.0f, 0.0f);
			renderNorthFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(4));
			tessellator->draw();
			tessellator->startDrawingQuads();
			tessellator->setNormal(1.0f, 0.0f, 0.0f);
			renderSouthFace(block, 0.0, 0.0, 0.0, block->getBlockTextureFromSide(5));
			tessellator->draw();
			glTranslatef(0.5f, 0.5f, 0.5f);
		}
		block->setBlockBounds(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
	}
}

bool RenderBlocks::renderItemIn3d(int_t i)
{
	if (i == 0)
	{
		return true;
	}
	if (i == 13)
	{
		return true;
	}
	if (i == 10)
	{
		return true;
	}
	if (i == 11)
	{
		return true;
	}
	return i == 16;
}