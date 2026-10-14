#pragma once

#include <vector>

#include "java/Type.h"
#include "java/String.h"

class Random;

class Material;
class IBlockAccess;
class AxisAlignedBB;
class World;
class Vec3D;
class MovingObjectPosition;
class EntityPlayer;
class EntityLiving;
class Entity;
class ItemStack;
class StepSound;

class BlockGrass;
class BlockLeaves;
class BlockTallGrass;
class BlockDeadBush;
class BlockPistonExtension;
class BlockPistonMoving;
class BlockFlower;
class BlockFire;
class BlockPortal;

// net.minecraft.src.Block
class Block
{
protected:
	Block(int_t i, Material *material);
	Block(int_t i, int_t j, Material *material);

	virtual Block *disableNeighborNotifyOnMetadataChange();

	virtual void initializeBlock();

	virtual Block *setStepSound(StepSound *stepsound);
	virtual Block *setLightOpacity(int_t i);
	virtual Block *setLightValue(float f);
	virtual Block *setResistance(float f);

public:
	virtual bool renderAsNormalBlock();
	virtual int_t getRenderType();

protected:
	virtual Block *setHardness(float f);
	virtual Block *setBlockUnbreakable();

public:
	virtual float getHardness();

protected:
	virtual Block *setTickOnLoad(bool flag);

public:
	virtual void setBlockBounds(float f, float f1, float f2, float f3, float f4, float f5);

	virtual float getBlockBrightness(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k);
	virtual bool shouldSideBeRendered(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l);
	virtual bool getIsBlockSolid(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l);
	virtual int_t getBlockTexture(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l);
	virtual int_t getBlockTextureFromSideAndMetadata(int_t i, int_t j);
	virtual int_t getBlockTextureFromSide(int_t i);

	virtual AxisAlignedBB *getSelectedBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k);
	virtual void getCollidingBoundingBoxes(World *world, int_t i, int_t j, int_t k, AxisAlignedBB *axisalignedbb, std::vector<AxisAlignedBB *> &arraylist);
	virtual AxisAlignedBB *getCollisionBoundingBoxFromPool(World *world, int_t i, int_t j, int_t k);

	virtual bool isOpaqueCube();
	virtual bool canCollideCheck(int_t i, bool flag);
	virtual bool isCollidable();

	virtual void updateTick(World *world, int_t i, int_t j, int_t k, Random &random);
	virtual void randomDisplayTick(World *world, int_t i, int_t j, int_t k, Random &random);
	virtual void onBlockDestroyedByPlayer(World *world, int_t i, int_t j, int_t k, int_t l);
	virtual void onNeighborBlockChange(World *world, int_t i, int_t j, int_t k, int_t l);
	virtual int_t tickRate();
	virtual void onBlockAdded(World *world, int_t i, int_t j, int_t k);
	virtual void onBlockRemoval(World *world, int_t i, int_t j, int_t k);
	virtual int_t quantityDropped(Random &random);
	virtual int_t idDropped(int_t i, Random &random);

	virtual float blockStrength(EntityPlayer *entityplayer);

	void dropBlockAsItem(World *world, int_t i, int_t j, int_t k, int_t l);
	virtual void dropBlockAsItemWithChance(World *world, int_t i, int_t j, int_t k, int_t l, float f);

protected:
	virtual void dropBlockAsItem_do(World *world, int_t i, int_t j, int_t k, ItemStack *itemstack);
	virtual int_t damageDropped(int_t i);

public:
	virtual float getExplosionResistance(Entity *entity);
	virtual MovingObjectPosition *collisionRayTrace(World *world, int_t i, int_t j, int_t k, Vec3D *vec3d, Vec3D *vec3d1);

private:
	bool isVecInsideYZBounds(Vec3D *vec3d);
	bool isVecInsideXZBounds(Vec3D *vec3d);
	bool isVecInsideXYBounds(Vec3D *vec3d);

public:
	virtual void onBlockDestroyedByExplosion(World *world, int_t i, int_t j, int_t k);
	virtual int_t getRenderBlockPass();
	virtual bool canPlaceBlockOnSide(World *world, int_t i, int_t j, int_t k, int_t l);
	virtual bool canPlaceBlockAt(World *world, int_t i, int_t j, int_t k);
	virtual bool blockActivated(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer);
	virtual void onEntityWalking(World *world, int_t i, int_t j, int_t k, Entity *entity);
	virtual void onBlockPlaced(World *world, int_t i, int_t j, int_t k, int_t l);
	virtual void onBlockClicked(World *world, int_t i, int_t j, int_t k, EntityPlayer *entityplayer);
	virtual void velocityToAddToEntity(World *world, int_t i, int_t j, int_t k, Entity *entity, Vec3D *vec3d);
	virtual void setBlockBoundsBasedOnState(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k);
	virtual int_t getRenderColor(int_t i);
	virtual int_t colorMultiplier(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k);
	virtual bool isPoweringTo(IBlockAccess *iblockaccess, int_t i, int_t j, int_t k, int_t l);
	virtual bool canProvidePower();
	virtual void onEntityCollidedWithBlock(World *world, int_t i, int_t j, int_t k, Entity *entity);
	virtual bool isIndirectlyPoweringTo(World *world, int_t i, int_t j, int_t k, int_t l);
	virtual void setBlockBoundsForItemRender();
	virtual void harvestBlock(World *world, EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t l);
	virtual bool canBlockStay(World *world, int_t i, int_t j, int_t k);
	virtual void onBlockPlacedBy(World *world, int_t i, int_t j, int_t k, EntityLiving *entityliving);
	virtual Block *setBlockName(const jstring &s);
	virtual jstring translateBlockName();
	virtual jstring getBlockName();
	virtual void playBlock(World *world, int_t i, int_t j, int_t k, int_t l, int_t i1);
	virtual bool getEnableStats();

protected:
	virtual Block *disableStats();

public:
	virtual int_t getMobilityFlag();

	virtual ~Block() = default;

	static StepSound *soundPowderFootstep;
	static StepSound *soundWoodFootstep;
	static StepSound *soundGravelFootstep;
	static StepSound *soundGrassFootstep;
	static StepSound *soundStoneFootstep;
	static StepSound *soundMetalFootstep;
	static StepSound *soundGlassFootstep;
	static StepSound *soundClothFootstep;
	static StepSound *soundSandFootstep;

	static Block *blocksList[256];
	static bool tickOnLoad[256];
	static bool opaqueCubeLookup[256];
	static bool isBlockContainer[256];
	static int_t lightOpacity[256];
	static bool lightOpacityExplicit[256]; // not in Java: marks blocks whose setLightOpacity() was called, so the post-init fixup won't clobber them
	static bool canBlockGrass[256];
	static int_t lightValue[256];
	static bool disableNeighborNotifyOnMetadataChangeLookup[256];

	static Block *stone;
	static BlockGrass *grass;
	static Block *dirt;
	static Block *cobblestone;
	static Block *planks;
	static Block *sapling;
	static Block *bedrock;
	static Block *waterMoving;
	static Block *waterStill;
	static Block *lavaMoving;
	static Block *lavaStill;
	static Block *sand;
	static Block *gravel;
	static Block *oreGold;
	static Block *oreIron;
	static Block *oreCoal;
	static Block *wood;
	static BlockLeaves *leaves;
	static Block *sponge;
	static Block *glass;
	static Block *oreLapis;
	static Block *blockLapis;
	static Block *dispenser;
	static Block *sandStone;
	static Block *musicBlock;
	static Block *blockBed;
	static Block *railPowered;
	static Block *railDetector;
	static Block *pistonStickyBase;
	static Block *web;
	static BlockTallGrass *tallGrass;
	static BlockDeadBush *deadBush;
	static Block *pistonBase;
	static BlockPistonExtension *pistonExtension;
	static Block *cloth;
	static BlockPistonMoving *pistonMoving;
	static BlockFlower *plantYellow;
	static BlockFlower *plantRed;
	static BlockFlower *mushroomBrown;
	static BlockFlower *mushroomRed;
	static Block *blockGold;
	static Block *blockSteel;
	static Block *stairDouble;
	static Block *stairSingle;
	static Block *brick;
	static Block *tnt;
	static Block *bookShelf;
	static Block *cobblestoneMossy;
	static Block *obsidian;
	static Block *torchWood;
	static BlockFire *fire;
	static Block *mobSpawner;
	static Block *stairCompactPlanks;
	static Block *chest;
	static Block *redstoneWire;
	static Block *oreDiamond;
	static Block *blockDiamond;
	static Block *workbench;
	static Block *crops;
	static Block *tilledField;
	static Block *stoneOvenIdle;
	static Block *stoneOvenActive;
	static Block *signPost;
	static Block *doorWood;
	static Block *ladder;
	static Block *rail;
	static Block *stairCompactCobblestone;
	static Block *signWall;
	static Block *lever;
	static Block *pressurePlateStone;
	static Block *doorSteel;
	static Block *pressurePlatePlanks;
	static Block *oreRedstone;
	static Block *oreRedstoneGlowing;
	static Block *torchRedstoneIdle;
	static Block *torchRedstoneActive;
	static Block *button;
	static Block *snow;
	static Block *ice;
	static Block *blockSnow;
	static Block *cactus;
	static Block *blockClay;
	static Block *reed;
	static Block *jukebox;
	static Block *fence;
	static Block *pumpkin;
	static Block *netherrack;
	static Block *slowSand;
	static Block *glowStone;
	static BlockPortal *portal;
	static Block *pumpkinLantern;
	static Block *cake;
	static Block *redstoneRepeaterIdle;
	static Block *redstoneRepeaterActive;
	static Block *lockedChest;
	static Block *trapdoor;

	int_t blockIndexInTexture = 0;
	const int_t blockID;

	// Java keeps these `protected` but every Block subclass reads them via
	// `block.blockHardness` / `block.blockResistance` (package-level access),
	// which is effectively public from the call sites. Expose them as public
	// here so the 1:1 ports compile.
	float blockHardness = 0.0f;
	float blockResistance = 0.0f;

protected:
	bool blockConstructorCalled;
	bool enableStats;

public:
	double minX = 0.0;
	double minY = 0.0;
	double minZ = 0.0;
	double maxX = 0.0;
	double maxY = 0.0;
	double maxZ = 0.0;
	StepSound *stepSound;
	float blockParticleGravity;
	Material *const blockMaterial;
	float slipperiness;

private:
	jstring blockName;

public:
	// Constructs the static Block registry. Must be called once at startup,
	// after Material::initialize(). Java does this in a static {} block; C++ has
	// no defined cross-translation-unit static init order, so it is explicit.
	static void initialize();
};
