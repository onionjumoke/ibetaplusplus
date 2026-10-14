#pragma once

#include <vector>
#include "java/Type.h"

class World;
class TileEntity;
class ICamera;
class Entity;
class AxisAlignedBB;

// net.minecraft.src.WorldRenderer
class WorldRenderer
{
public:
	WorldRenderer(World *world, std::vector<TileEntity *> *tileEntities, int_t posX, int_t posY, int_t posZ, int_t size, int_t glListId);
	~WorldRenderer();

	void setPosition(int_t x, int_t y, int_t z);
	void updateInFrustrum(ICamera *icamera);
	bool updateRenderer();
	bool isBuilt() const { return isInitialized; }
	void markDirty();
	void callOcclusionQueryList();
	int_t getGLCallListForPass(int_t pass);
	bool skipAllRenderPasses();
	bool skipRenderPass(int_t pass);
	float distanceToEntitySquared(Entity *entity);
	void setDontDraw();
	void func_1204_c();
	void cleanup();

	static int_t chunksUpdated;

	// Public fields matching Java
	World *worldObj;
	bool needsUpdate;
	bool isChunkLit;
	bool isWaitingOnOcclusionQuery;
	bool isVisible;
	bool isInFrustum;
	int_t chunkIndex;
	int_t glOcclusionQuery;

	int_t posX;
	int_t posY;
	int_t posZ;
	int_t sizeWidth;
	int_t sizeHeight;
	int_t sizeDepth;

	// Center positions (posX + size/2) for distance calc
	int_t posXPlus;
	int_t posYPlus;
	int_t posZPlus;

	// Clip positions (i & 0x3ff)
	int_t posXClip;
	int_t posYClip;
	int_t posZClip;

	// Offsets (i - posXClip) — used by RenderGlobal for translation
	int_t posXMinus;
	int_t posYMinus;
	int_t posZMinus;

	float rendererRadius;
	AxisAlignedBB *rendererBoundingBox;

	// Tile entities with special renderers collected during updateRenderer()
	std::vector<TileEntity *> tileEntityRenderers;

private:
	int_t glRenderList;
	bool isInitialized;
	bool _skipRenderPass[2];

	std::vector<TileEntity *> *tileEntities;
};
