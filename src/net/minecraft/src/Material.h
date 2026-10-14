#pragma once

#include "java/Type.h"

class MapColor;

// net.minecraft.src.Material
class Material
{
public:
	Material(MapColor *mapcolor);
	virtual ~Material() = default;

	virtual bool getIsLiquid();
	virtual bool isSolid();
	virtual bool getCanBlockGrass();
	virtual bool getIsSolid();

private:
	Material *setIsTranslucent();
	Material *setNoHarvest();
	Material *setBurning();

public:
	bool getBurning();

	Material *setIsGroundCover();
	bool getIsGroundCover();

	bool getIsTranslucent();
	bool getIsHarvestable();

	int_t getMaterialMobility();

protected:
	Material *setNoPushMobility();
	Material *setImmovableMobility();

public:
	static Material *air;
	static Material *grassMaterial;
	static Material *ground;
	static Material *wood;
	static Material *rock;
	static Material *iron;
	static Material *water;
	static Material *lava;
	static Material *leaves;
	static Material *plants;
	static Material *sponge;
	static Material *cloth;
	static Material *fire;
	static Material *sand;
	static Material *circuits;
	static Material *glass;
	static Material *tnt;
	static Material *spawnableCreatureList;
	static Material *ice;
	static Material *snow;
	static Material *builtSnow;
	static Material *cactus;
	static Material *clay;
	static Material *pumpkin;
	static Material *portal;
	static Material *cakeMaterial;
	static Material *pistonBase;
	static Material *pistonSticky;

private:
	bool canBurn = false;
	bool groundCover = false;
	bool isOpaque = false;

public:
	MapColor *const materialMapColor;

private:
	bool canHarvest;
	int_t mobilityFlag = 0;

public:
	// Constructs the static Material instances. Must be called once at startup,
	// after MapColor's statics are initialized. Java does this in a static {} block;
	// C++ has no defined cross-translation-unit static init order, so it is explicit.
	static void initialize();
};
