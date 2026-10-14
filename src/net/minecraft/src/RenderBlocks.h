#pragma once

#include "java/Type.h"

class IBlockAccess;
class Block;
class BlockRail;
class World;
class Tessellator;
class Material;
class Vec3D;

// net.minecraft.src.RenderBlocks
class RenderBlocks
{
public:
	RenderBlocks(IBlockAccess *iblockaccess);
	RenderBlocks();

	void renderBlockUsingTexture(Block *block, int_t i, int_t j, int_t k, int_t l);
	void renderBlockAllFaces(Block *block, int_t i, int_t j, int_t k); // func_31075_a
	bool renderBlockByRenderType(Block *block, int_t i, int_t j, int_t k);

	// Renombrado de func_31078_d: renderiza base de pistón con todas las caras
	void renderPistonBaseAllFaces(Block *block, int_t i, int_t j, int_t k);
	// Renombrado de func_31079_a: renderiza extensión de pistón con todas las caras
	void renderPistonExtensionAllFaces(Block *block, int_t i, int_t j, int_t k, bool flag);

private:
	bool renderBlockTorch(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockBed(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockRepeater(Block *block, int_t i, int_t j, int_t k);
	// Renombrado de func_31074_b: renderiza base de pistón (no extendido)
	bool renderPistonBase(Block *block, int_t i, int_t j, int_t k, bool flag);
	// Renombrado de func_31076_a: renderiza brazo de pistón en X
	void renderPistonArmX(double d, double d1, double d2, double d3, double d4, double d5, float f, double d6);
	// Renombrado de func_31081_b: renderiza brazo de pistón en Y
	void renderPistonArmY(double d, double d1, double d2, double d3, double d4, double d5, float f, double d6);
	// Renombrado de func_31077_c: renderiza brazo de pistón en Z
	void renderPistonArmZ(double d, double d1, double d2, double d3, double d4, double d5, float f, double d6);
	// Renombrado de func_31080_c: renderiza extensión de pistón
	bool renderPistonExtension(Block *block, int_t i, int_t j, int_t k, bool flag);

public:
	bool renderBlockLever(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockFire(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockRedstoneWire(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockMinecartTrack(BlockRail *blockrail, int_t i, int_t j, int_t k);
	bool renderBlockLadder(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockReed(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockCrops(Block *block, int_t i, int_t j, int_t k);
	void renderTorchAtAngle(Block *block, double d, double d1, double d2, double d3, double d4);
	void renderCrossedSquares(Block *block, int_t i, double d, double d1, double d2);
	// Renombrado de func_1245_b: renderiza cultivos cruzados (crops)
	void renderCropsCrossed(Block *block, int_t i, double d, double d1, double d2);
	bool renderBlockFluids(Block *block, int_t i, int_t j, int_t k);
	// Renombrado de func_1224_a: calcula altura de fluido
	float getFluidHeight(int_t i, int_t j, int_t k, Material *material);
	void renderBlockFallingSand(Block *block, World *world, int_t i, int_t j, int_t k);
	bool renderStandardBlock(Block *block, int_t i, int_t j, int_t k);
	bool renderStandardBlockWithAmbientOcclusion(Block *block, int_t i, int_t j, int_t k, float f, float f1, float f2);
	bool renderStandardBlockWithColorMultiplier(Block *block, int_t i, int_t j, int_t k, float f, float f1, float f2);
	bool renderBlockCactus(Block *block, int_t i, int_t j, int_t k);
	// Renombrado de func_1230_b: renderiza cactus con multiplicador de color
	bool renderCactusWithColorMultiplier(Block *block, int_t i, int_t j, int_t k, float f, float f1, float f2);
	bool renderBlockFence(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockStairs(Block *block, int_t i, int_t j, int_t k);
	bool renderBlockDoor(Block *block, int_t i, int_t j, int_t k);
	void renderBottomFace(Block *block, double d, double d1, double d2, int_t i);
	void renderTopFace(Block *block, double d, double d1, double d2, int_t i);
	void renderEastFace(Block *block, double d, double d1, double d2, int_t i);
	void renderWestFace(Block *block, double d, double d1, double d2, int_t i);
	void renderNorthFace(Block *block, double d, double d1, double d2, int_t i);
	void renderSouthFace(Block *block, double d, double d1, double d2, int_t i);
	void renderBlockOnInventory(Block *block, int_t i, float f);

	static bool renderItemIn3d(int_t i);

	IBlockAccess *blockAccess;
	int_t overrideBlockTexture;
	bool flipTexture;
	bool renderAllFaces;
	bool field_31088_b;
	// Renombrado de field_31087_g: rotación de textura para cara este
	int_t eastFaceRotation;
	// Renombrado de field_31086_h: rotación de textura para cara oeste
	int_t westFaceRotation;
	// Renombrado de field_31085_i: rotación de textura para cara sur
	int_t southFaceRotation;
	// Renombrado de field_31084_j: rotación de textura para cara norte
	int_t northFaceRotation;
	// Renombrado de field_31083_k: rotación de textura para cara superior
	int_t topFaceRotation;
	// Renombrado de field_31082_l: rotación de textura para cara inferior
	int_t bottomFaceRotation;
	// Renombrado de field_22352_G: flag de calidad de iluminación (1 = mejor calidad)
	int_t lightingQuality;
	bool enableAO;
	float lightValueOwn;
	float aoLightValueXNeg;
	float aoLightValueYNeg;
	float aoLightValueZNeg;
	float aoLightValueXPos;
	float aoLightValueYPos;
	float aoLightValueZPos;
	// Renombrado de field_22377_m: valor de luz AO esquina inferior-norte-oeste
	float aoLightValueXYZNNN;
	// Renombrado de field_22376_n: valor de luz AO negativo X
	float aoLightValueXNeg2;
	// Renombrado de field_22375_o: valor de luz AO esquina inferior-sur-oeste
	float aoLightValueXYZNNP;
	// Renombrado de field_22374_p: valor de luz AO negativo Y
	float aoLightValueYNeg2;
	// Renombrado de field_22373_q: valor de luz AO esquina inferior-sur-este
	float aoLightValueXYZPNN;
	// Renombrado de field_22372_r: valor de luz AO esquina inferior-norte-este
	float aoLightValueXYZPNP;
	// Renombrado de field_22371_s: valor de luz AO positivo X
	float aoLightValueXPos2;
	// Renombrado de field_22370_t: valor de luz AO esquina superior-sur-este
	float aoLightValueXYZPPP;
	// Renombrado de field_22369_u: valor de luz AO esquina superior-norte-oeste
	float aoLightValueXYZNNP2;
	// Renombrado de field_22368_v: valor de luz AO positivo X (arriba)
	float aoLightValueXPosUp;
	// Renombrado de field_22367_w: valor de luz AO esquina superior-sur-oeste
	float aoLightValueXYZPNP2;
	// Renombrado de field_22366_x: valor de luz AO negativo Z (arriba)
	float aoLightValueZNegUp;
	// Renombrado de field_22365_y: valor de luz AO esquina superior-norte-este
	float aoLightValueXYZPPN;
	// Renombrado de field_22364_z: valor de luz AO positivo X (arriba) 2
	float aoLightValueXPosUp2;
	// Renombrado de field_22362_A: valor de luz AO positivo Z (arriba)
	float aoLightValueZPosUp;
	// Renombrado de field_22360_B: valor de luz AO esquina superior-sur-este 2
	float aoLightValueXYZPPP2;
	// Renombrado de field_22358_C: valor de luz AO negativo X (norte)
	float aoLightValueXNegNorth;
	// Renombrado de field_22356_D: valor de luz AO positivo X (norte)
	float aoLightValueXPosNorth;
	// Renombrado de field_22354_E: valor de luz AO negativo X (sur)
	float aoLightValueXNegSouth;
	// Renombrado de field_22353_F: valor de luz AO positivo X (sur)
	float aoLightValueXPosSouth;
	float colorRedTopLeft;
	float colorRedBottomLeft;
	float colorRedBottomRight;
	float colorRedTopRight;
	float colorGreenTopLeft;
	float colorGreenBottomLeft;
	float colorGreenBottomRight;
	float colorGreenTopRight;
	float colorBlueTopLeft;
	float colorBlueBottomLeft;
	float colorBlueBottomRight;
	float colorBlueTopRight;
	// Renombrado de field_22339_T: puede bloquear hierba arriba-norte
	bool canBlockGrassAboveNorth;
	// Renombrado de field_22338_U: puede bloquear hierba arriba-este
	bool canBlockGrassAboveEast;
	// Renombrado de field_22337_V: puede bloquear hierga arriba-oeste
	bool canBlockGrassAboveWest;
	// Renombrado de field_22336_W: puede bloquear hierga arriba-sur
	bool canBlockGrassAboveSouth;
	// Renombrado de field_22335_X: puede bloquear hierga abajo-norte
	bool canBlockGrassBelowNorth;
	// Renombrado de field_22334_Y: puede bloquear hierga abajo-este
	bool canBlockGrassBelowEast;
	// Renombrado de field_22333_Z: puede bloquear hierga abajo-sur
	bool canBlockGrassBelowSouth;
	// Renombrado de field_22363_aa: puede bloquear hierga abajo-oeste
	bool canBlockGrassBelowWest;
	// Renombrado de field_22361_ab: puede bloquear hierga abajo-norte-oeste
	bool canBlockGrassBelowNorthWest;
	// Renombrado de field_22359_ac: puede bloquear hierga abajo-este-sur
	bool canBlockGrassBelowEastSouth;
	// Renombrado de field_22357_ad: puede bloquear hierga abajo-oeste-sur
	bool canBlockGrassBelowWestSouth;
	// Renombrado de field_22355_ae: puede bloquear hierga abajo-sur-este
	bool canBlockGrassBelowSouthEast;

	static bool fancyGrass;
};