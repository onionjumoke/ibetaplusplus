#pragma once

#include <unordered_set>
#include <vector>
#include <string>

#include "java/Type.h"
#include "java/String.h"
#include "java/Random.h"
#include "IWorldAccess.h"

class Minecraft;
class RenderEngine;
class World;
class RenderBlocks;
class WorldRenderer;
class Entity;
class EntityLiving;
class EntityPlayer;
class Vec3D;
class ICamera;
class TileEntity;
class Tessellator;
class MovingObjectPosition;
class ItemStack;
class RenderList;
class AxisAlignedBB;

// net.minecraft.src.RenderGlobal
// Implementa IWorldAccess para recibir callbacks del mundo
class RenderGlobal : public IWorldAccess
{
public:
	RenderGlobal(Minecraft *minecraft, RenderEngine *renderengine);
	~RenderGlobal();

	// IWorldAccess implementation
	void markBlockAndNeighborsNeedsUpdate(int_t i, int_t j, int_t k) override;
	void markBlockRangeNeedsUpdate(int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1) override;
	void playSound(const jstring &s, double d, double d1, double d2, float f, float f1) override;
	void spawnParticle(const jstring &s, double d, double d1, double d2, double d3, double d4, double d5) override;
	void obtainEntitySkin(Entity *entity) override;
	void releaseEntitySkin(Entity *entity) override;
	void updateAllRenderers() override;
	void playRecord(const jstring &s, int_t i, int_t j, int_t k) override;
	void doNothingWithTileEntity(int_t i, int_t j, int_t k, TileEntity *tileentity) override;
	void playAuxSFX(EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t l, int_t i1) override;

	// RenderGlobal methods
	void changeWorld(World *world);
	void loadRenderers();
	void renderEntities(Vec3D *vec3d, ICamera *icamera, float f);
	void drawChunkBorders(float f);
	jstring getDebugInfoRenders();
	jstring getDebugInfoEntities();
	int_t sortAndRender(EntityLiving *entityliving, int_t i, double d);
	void renderSky(float f);
	void renderClouds(float f);
	void renderCloudsFancy(float f);
	bool func_27307_a(double d, double d1, double d2, float f);
	bool updateRenderers(EntityLiving *entityliving, bool flag);
	void drawBlockBreaking(EntityPlayer *entityplayer, MovingObjectPosition *movingobjectposition, int_t i, ItemStack *itemstack, float f);
	void drawSelectionBox(EntityPlayer *entityplayer, MovingObjectPosition *movingobjectposition, int_t i, ItemStack *itemstack, float f);
	void updateClouds();
	void clipRenderersByFrustrum(ICamera *icamera, float f);
	void renderAllRenderLists(int_t i, double d);
	bool isCloudFog(double x, double y, double z, float partialTicks);  // func_27307_a
	// func_28137_f — clears/resets all world renderers (called on shutdown/world change)
	void clearWorldRenderers();

	// Public fields
	std::vector<TileEntity *> tileEntities;
	float damagePartialTime = 0.0f;

private:
	// func_949_a — marks all WorldRenderers whose chunk coords fall in [i..l, j..i1, k..j1] dirty
	void markRenderersInRange(int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1);
	void renderStars();
	void markRenderersForNewPosition(int_t i, int_t j, int_t k);
	void enqueueRendererUpdate(WorldRenderer *worldrenderer);
	void dequeueRendererUpdate(WorldRenderer *worldrenderer);
	void checkOcclusionQueryResult(int_t i, int_t j);
	int_t renderSortedRenderers(int_t i, int_t j, int_t k, double d);
	void drawOutlinedBoundingBox(AxisAlignedBB *axisalignedbb);

	// Private fields
	World *worldObj = nullptr;
	RenderEngine *renderEngine = nullptr;
	std::vector<WorldRenderer *> worldRenderersToUpdate;
	// Plain vector, not unordered_set: the statically-linked libc++ hash
	// table hung/crashed on this iOS5 build (erase never returned).
	std::vector<WorldRenderer *> worldRenderersQueuedForUpdate;
	WorldRenderer **sortedWorldRenderers = nullptr;
	WorldRenderer **worldRenderers = nullptr;
	int_t renderChunksWide = 0;
	int_t renderChunksTall = 0;
	int_t renderChunksDeep = 0;
	int_t glRenderListBase = 0;
	Minecraft *mc = nullptr;
	RenderBlocks *globalRenderBlocks = nullptr;
	std::vector<int_t> glOcclusionQueryBase;
	bool occlusionEnabled = false;
	int_t cloudOffsetX = 0;
	int_t starGLCallList = 0;
	int_t glSkyList = 0;
	int_t glSkyList2 = 0;
	int_t minBlockX = 0;
	int_t minBlockY = 0;
	int_t minBlockZ = 0;
	int_t maxBlockX = 0;
	int_t maxBlockY = 0;
	int_t maxBlockZ = 0;
	int_t renderDistance = -1;
	int_t renderEntitiesStartupCounter = 2;
	int_t countEntitiesTotal = 0;
	int_t countEntitiesRendered = 0;
	int_t countEntitiesHidden = 0;
	std::vector<int_t> dummyBuf50k;
	std::vector<int_t> occlusionResult;
	int_t renderersLoaded = 0;
	int_t renderersBeingClipped = 0;
	int_t renderersBeingOccluded = 0;
	int_t renderersBeingRendered = 0;
	int_t renderersSkippingRenderPass = 0;
	int_t worldRenderersCheckIndex = 0;
	std::vector<WorldRenderer *> glRenderLists;
	RenderList *allRenderLists[4];
	int_t dummyInt0 = 0;
	int_t glDummyList = 0;
	double prevSortX = -9999.0;
	double prevSortY = -9999.0;
	double prevSortZ = -9999.0;
	int_t frustrumCheckOffset = 0;
};
