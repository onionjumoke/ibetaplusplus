#include <unistd.h>
#include <cstdlib>
#include "RenderGlobal.h"
#include <chrono>
#include <atomic>
#include <cstdio>

#ifdef MC_IOS
extern "C" int gles11MatrixMaxDepth(int idx);
extern "C" int gles11MatrixRefused(int idx);
#else
static int gles11MatrixMaxDepth(int) { return 0; }
static int gles11MatrixRefused(int) { return 0; }
#endif

static std::atomic<long> g_dbgMarks{0};
static std::atomic<long> g_dbgEnq{0};

#include <algorithm>
#include <cmath>

#include "pc/OpenGL.h"
#include "client/Minecraft.h"
#include "Config.h"
#include "Block.h"
#include "BlockLeaves.h"
#include "Entity.h"
#include "EntityLiving.h"
#include "EntityPlayer.h"
#include "EntityPlayerSP.h"
#include "EntityRenderer.h"
#include "EffectRenderer.h"
#include "EntityBubbleFX.h"
#include "EntitySmokeFX.h"
#include "EntityNoteFX.h"
#include "EntityPortalFX.h"
#include "EntityExplodeFX.h"
#include "EntityFlameFX.h"
#include "EntityLavaFX.h"
#include "EntityFootStepFX.h"
#include "EntitySplashFX.h"
#include "EntityReddustFX.h"
#include "EntitySlimeFX.h"
#include "EntitySnowShovelFX.h"
#include "EntityHeartFX.h"
#include "EnumMovingObjectType.h"
#include "FontRenderer.h"
#include "Frustrum.h"
#include "GameSettings.h"
#include "GLAllocation.h"
#include "GuiIngame.h"
#include "ImageBufferDownload.h"
#include "Item.h"
#include "ItemRecord.h"
#include "ItemStack.h"
#include "MathHelper.h"
#include "MovingObjectPosition.h"
#include "OpenGlCapsChecker.h"
#include "RenderBlocks.h"
#include "RenderEngine.h"
#include "RenderHelper.h"
#include "RenderList.h"
#include "RenderManager.h"
#include "RenderSorter.h"
#include "EntitySorter.h"
#include "SoundManager.h"
#include "StepSound.h"
#include "Tessellator.h"
#include "TileEntity.h"
#include "TileEntityRenderer.h"
#include "Vec3D.h"
#include "World.h"
#include "WorldProvider.h"
#include "WorldRenderer.h"

RenderGlobal::RenderGlobal(Minecraft *minecraft, RenderEngine *renderengine)
{
	tileEntities = std::vector<TileEntity *>();
	worldRenderersToUpdate = std::vector<WorldRenderer *>();
	occlusionEnabled = false;
	cloudOffsetX = 0;
	renderDistance = -1;
	renderEntitiesStartupCounter = 2;
	dummyBuf50k = std::vector<int_t>(50000);
	occlusionResult = GLAllocation::createDirectIntBuffer(64);
	glRenderLists = std::vector<WorldRenderer *>();
	dummyInt0 = 0;
	glDummyList = GLAllocation::generateDisplayLists(1);
	prevSortX = -9999.0;
	prevSortY = -9999.0;
	prevSortZ = -9999.0;
	frustrumCheckOffset = 0;
	mc = minecraft;
	renderEngine = renderengine;

	// The renderer is clamped to 400 blocks below, so at most 26 * 8 * 26
	// chunk renderers are alive. Reserving the original 64^3 range can fail on
	// some compatibility drivers and return list 0, causing list-ID collisions.
	constexpr int_t maxChunksWide = 400 / 16 + 1;
	constexpr int_t maxChunksTall = 8;
	constexpr int_t maxWorldRenderers = maxChunksWide * maxChunksTall * maxChunksWide;
	glRenderListBase = GLAllocation::generateDisplayLists(maxWorldRenderers * 3);
	// FORCE OFF: ARB occlusion query boxes are chunk-sized cubes drawn with the
	// terrain texture. If the shim's colour-mask handling lets them render, they
	// appear as shattered chunk-sized geometry. Test with them disabled.
	occlusionEnabled = false;

	if (occlusionEnabled)
	{
		glOcclusionQueryBase = GLAllocation::createDirectIntBuffer(maxWorldRenderers);
		glGenQueriesARB((GLsizei)glOcclusionQueryBase.size(), reinterpret_cast<GLuint *>(glOcclusionQueryBase.data()));
	}

	starGLCallList = GLAllocation::generateDisplayLists(3);
	glPushMatrix();
	glNewList(starGLCallList, GL_COMPILE);
	renderStars();
	glEndList();
	glPopMatrix();

	Tessellator *tessellator = &Tessellator::instance;
	glSkyList = starGLCallList + 1;
	glNewList(glSkyList, GL_COMPILE);

	byte_t byte1 = 64;
	int_t i = 256 / byte1 + 2;
	float f = 16.0f;

	for (int_t j = -byte1 * i; j <= byte1 * i; j += byte1)
	{
		for (int_t l = -byte1 * i; l <= byte1 * i; l += byte1)
		{
			tessellator->startDrawingQuads();
			tessellator->addVertex(j + 0, f, l + 0);
			tessellator->addVertex(j + byte1, f, l + 0);
			tessellator->addVertex(j + byte1, f, l + byte1);
			tessellator->addVertex(j + 0, f, l + byte1);
			tessellator->draw();
		}
	}

	glEndList();

	glSkyList2 = starGLCallList + 2;
	glNewList(glSkyList2, GL_COMPILE);
	f = -16.0f;
	tessellator->startDrawingQuads();

	for (int_t k = -byte1 * i; k <= byte1 * i; k += byte1)
	{
		for (int_t i1 = -byte1 * i; i1 <= byte1 * i; i1 += byte1)
		{
			tessellator->addVertex(k + byte1, f, i1 + 0);
			tessellator->addVertex(k + 0, f, i1 + 0);
			tessellator->addVertex(k + 0, f, i1 + byte1);
			tessellator->addVertex(k + byte1, f, i1 + byte1);
		}
	}

	tessellator->draw();
	glEndList();

	// Initialize allRenderLists
	for (int_t i = 0; i < 4; i++)
		allRenderLists[i] = new RenderList();
}

RenderGlobal::~RenderGlobal()
{
	changeWorld(nullptr);
	for (RenderList *&renderList : allRenderLists)
	{
		delete renderList;
		renderList = nullptr;
	}
}

void RenderGlobal::renderStars()
{
	if (!Config::isStarsEnabled()) // OptiFine: Stars OFF
		return;

	Random random(10842LL);
	Tessellator *tessellator = &Tessellator::instance;
	tessellator->startDrawingQuads();

	for (int_t i = 0; i < 1500; i++)
	{
		double d = random.nextFloat() * 2.0f - 1.0f;
		double d1 = random.nextFloat() * 2.0f - 1.0f;
		double d2 = random.nextFloat() * 2.0f - 1.0f;
		double d3 = 0.25f + random.nextFloat() * 0.25f;
		double d4 = d * d + d1 * d1 + d2 * d2;

		if (d4 >= 1.0 || d4 <= 0.01)
			continue;

		d4 = 1.0 / std::sqrt(d4);
		d *= d4;
		d1 *= d4;
		d2 *= d4;

		double d5 = d * 100.0;
		double d6 = d1 * 100.0;
		double d7 = d2 * 100.0;

		double d8 = std::atan2(d, d2);
		double d9 = std::sin(d8);
		double d10 = std::cos(d8);
		double d11 = std::atan2(std::sqrt(d * d + d2 * d2), d1);
		double d12 = std::sin(d11);
		double d13 = std::cos(d11);
		double d14 = random.nextDouble() * 3.1415926535897931 * 2.0;
		double d15 = std::sin(d14);
		double d16 = std::cos(d14);

		for (int_t j = 0; j < 4; j++)
		{
			double d17 = 0.0;
			double d18 = (double)((j & 2) - 1) * d3;
			double d19 = (double)((j + 1 & 2) - 1) * d3;
			double d20 = d17;
			double d21 = d18 * d16 - d19 * d15;
			double d22 = d19 * d16 + d18 * d15;
			double d23 = d22;
			double d24 = d21 * d12 + d20 * d13;
			double d25 = d20 * d12 - d21 * d13;
			double d26 = d25 * d9 - d23 * d10;
			double d27 = d24;
			double d28 = d23 * d9 + d25 * d10;

			tessellator->addVertex(d5 + d26, d6 + d27, d7 + d28);
		}
	}

	tessellator->draw();
}

void RenderGlobal::changeWorld(World *world)
{
	if (worldObj != nullptr)
		worldObj->removeWorldAccess(this);

	prevSortX = -9999.0;
	prevSortY = -9999.0;
	prevSortZ = -9999.0;

	// RenderManager.instance.func_852_a(world); // TODO: implement when RenderManager is available
	clearWorldRenderers();
	delete globalRenderBlocks;
	globalRenderBlocks = nullptr;
	worldObj = world;

	if (world != nullptr)
	{
		globalRenderBlocks = new RenderBlocks(world);
		world->addWorldAccess(this);
		loadRenderers();
	}
}

void RenderGlobal::loadRenderers()
{
	Block::leaves->setGraphicsLevel(mc->gameSettings->fancyGraphics);
	renderDistance = mc->gameSettings->renderDistance;
	worldRenderersToUpdate.clear();
	worldRenderersQueuedForUpdate.clear();
	tileEntities.clear();

	if (worldRenderers != nullptr)
	{
		for (int_t i = 0; i < renderChunksWide * renderChunksTall * renderChunksDeep; i++)
		{
			if (worldRenderers[i] != nullptr)
				delete worldRenderers[i];
		}
		delete[] worldRenderers;
		delete[] sortedWorldRenderers;
	}

	int_t j = 64 << (3 - renderDistance);
	if (j > 400)
		j = 400;

	renderChunksWide = j / 16 + 1;
	renderChunksTall = 8;
	renderChunksDeep = j / 16 + 1;

	int_t totalRenderers = renderChunksWide * renderChunksTall * renderChunksDeep;
	worldRenderersToUpdate.reserve(totalRenderers);
	worldRenderersQueuedForUpdate.reserve(totalRenderers);
	worldRenderers = new WorldRenderer *[totalRenderers];
	sortedWorldRenderers = new WorldRenderer *[totalRenderers];

	int_t k = 0;
	int_t l = 0;

	minBlockX = 0;
	minBlockY = 0;
	minBlockZ = 0;
	maxBlockX = renderChunksWide;
	maxBlockY = renderChunksTall;
	maxBlockZ = renderChunksDeep;

	for (int_t j1 = 0; j1 < renderChunksWide; j1++)
	{
		for (int_t k1 = 0; k1 < renderChunksTall; k1++)
		{
			for (int_t l1 = 0; l1 < renderChunksDeep; l1++)
			{
				int_t index = (l1 * renderChunksTall + k1) * renderChunksWide + j1;
				worldRenderers[index] = new WorldRenderer(worldObj, &tileEntities, j1 * 16, k1 * 16, l1 * 16, 16, glRenderListBase + k);

				if (occlusionEnabled)
					worldRenderers[index]->glOcclusionQuery = glOcclusionQueryBase[l];

				worldRenderers[index]->isWaitingOnOcclusionQuery = false;
				worldRenderers[index]->isVisible = true;
				worldRenderers[index]->isInFrustum = true;
				worldRenderers[index]->chunkIndex = l++;
				worldRenderers[index]->markDirty();
				sortedWorldRenderers[index] = worldRenderers[index];
				enqueueRendererUpdate(worldRenderers[index]);
				k += 3;
			}
		}
	}

	if (worldObj != nullptr)
	{
		EntityLiving *entityliving = mc->renderViewEntity;
		if (entityliving != nullptr)
		{
			markRenderersForNewPosition(MathHelper::floor_double(entityliving->posX), MathHelper::floor_double(entityliving->posY), MathHelper::floor_double(entityliving->posZ));
			// Arrays.sort(sortedWorldRenderers, new EntitySorter(entityliving)); // TODO: implement sorting
		}
	}

	renderEntitiesStartupCounter = 2;
}

void RenderGlobal::renderEntities(Vec3D *vec3d, ICamera *icamera, float f)
{
	if (renderEntitiesStartupCounter > 0)
	{
		renderEntitiesStartupCounter--;
		return;
	}

	TileEntityRenderer::instance.cacheActiveRenderInfo(worldObj, renderEngine, mc->fontRenderer, mc->renderViewEntity, f);
	RenderManager::instance->cacheActiveRenderInfo(worldObj, renderEngine, mc->fontRenderer, mc->renderViewEntity, mc->gameSettings, f);

	countEntitiesTotal = 0;
	countEntitiesRendered = 0;
	countEntitiesHidden = 0;

	EntityLiving *entityliving = mc->renderViewEntity;

	RenderManager::renderPosX = entityliving->lastTickPosX + (entityliving->posX - entityliving->lastTickPosX) * (double)f;
	RenderManager::renderPosY = entityliving->lastTickPosY + (entityliving->posY - entityliving->lastTickPosY) * (double)f;
	RenderManager::renderPosZ = entityliving->lastTickPosZ + (entityliving->posZ - entityliving->lastTickPosZ) * (double)f;

	TileEntityRenderer::staticPlayerX = entityliving->lastTickPosX + (entityliving->posX - entityliving->lastTickPosX) * (double)f;
	TileEntityRenderer::staticPlayerY = entityliving->lastTickPosY + (entityliving->posY - entityliving->lastTickPosY) * (double)f;
	TileEntityRenderer::staticPlayerZ = entityliving->lastTickPosZ + (entityliving->posZ - entityliving->lastTickPosZ) * (double)f;

	std::vector<Entity *> &list = worldObj->getLoadedEntityList();
	countEntitiesTotal = (int_t)list.size();

	for (int_t i = 0; i < (int_t)worldObj->weatherEffects.size(); i++)
	{
		Entity *entity = worldObj->weatherEffects[i];
		countEntitiesRendered++;
		if (entity->isInRangeToRenderVec3D(vec3d))
			RenderManager::instance->renderEntity(entity, f);
	}

	for (int_t j = 0; j < (int_t)list.size(); j++)
	{
		Entity *entity1 = list[j];
		if (!entity1->isInRangeToRenderVec3D(vec3d) || !entity1->ignoreFrustumCheck && !icamera->isBoundingBoxInFrustum(entity1->boundingBox) || entity1 == mc->renderViewEntity && !mc->gameSettings->thirdPersonView && !mc->renderViewEntity->isPlayerSleeping())
			continue;

		int_t l = MathHelper::floor_double(entity1->posY);
		if (l < 0)
			l = 0;
		if (l >= 128)
			l = 127;

		if (worldObj->blockExists(MathHelper::floor_double(entity1->posX), l, MathHelper::floor_double(entity1->posZ)))
		{
			countEntitiesRendered++;
			RenderManager::instance->renderEntity(entity1, f);
		}
	}

	// Tile entities (signs, spawners, pistons, ...) were never collected into
	// this list, so nothing that uses a special renderer ever drew. Populate it
	// from the world's loaded tile-entity list before rendering.
	tileEntities.clear();
	for (TileEntity *te : worldObj->loadedTileEntityList)
	{
		if (te == nullptr || te->isInvalid())
			continue;
		// Only tile entities with a special renderer are drawn (signs, spawners,
		// pistons). Chests/furnaces/etc. have none, so skip them before the
		// per-entity light lookup and typeid dispatch.
		if (!TileEntityRenderer::instance.hasSpecialRenderer(te))
			continue;
		// Frustum-cull: vanilla collects tile entities per visible WorldRenderer,
		// so off-screen ones (very costly for text-heavy signs) were never drawn.
		AxisAlignedBB *teBox = AxisAlignedBB::getBoundingBoxFromPool(
			te->xCoord, te->yCoord, te->zCoord, te->xCoord + 1, te->yCoord + 1, te->zCoord + 1);
		if (icamera->isBoundingBoxInFrustum(teBox))
			tileEntities.push_back(te);
	}

	const long_t teStartNs = System::nanoTime();
	// Diagnostic bisect: <TMPDIR>/bp.noTE disables tile-entity rendering live.
	{
		const char *tmp = std::getenv("TMPDIR");
		std::string noTE = std::string(tmp ? tmp : "/tmp") + "/bp.noTE";
		if (access(noTE.c_str(), F_OK) == 0)
			tileEntities.clear();
	}
	for (size_t k = 0; k < tileEntities.size(); k++)
		TileEntityRenderer::instance.renderTileEntity(tileEntities[k], f);
	const long_t teMs = (System::nanoTime() - teStartNs) / 1000000LL;
	{
		static long_t s_lastTeLog = 0;
		static long_t s_teWorst = 0;
		if (teMs > s_teWorst) s_teWorst = teMs;
		const long_t nowTE = System::currentTimeMillis();
		if (nowTE - s_lastTeLog >= 2000)
		{
			s_lastTeLog = nowTE;
			(void)0;
			s_teWorst = 0;
		}
	}
}

jstring RenderGlobal::getDebugInfoRenders()
{
	return "C: " + std::to_string(renderersBeingRendered) + "/" + std::to_string(renderersLoaded) + ". F: " + std::to_string(renderersBeingClipped) + ", O: " + std::to_string(renderersBeingOccluded) + ", E: " + std::to_string(renderersSkippingRenderPass);
}

jstring RenderGlobal::getDebugInfoEntities()
{
	return "E: " + std::to_string(countEntitiesRendered) + "/" + std::to_string(countEntitiesTotal) + ". B: " + std::to_string(countEntitiesHidden) + ", I: " + std::to_string(countEntitiesTotal - countEntitiesHidden - countEntitiesRendered);
}

void RenderGlobal::markRenderersForNewPosition(int_t i, int_t j, int_t k)
{
	{
		static long_t mrfpLast = 0;
		long_t mrfpNow = System::currentTimeMillis();
		if (mrfpNow - mrfpLast > 2000)
		{
			mrfpLast = mrfpNow;
			(void)0;
		}
	}
	i -= 8;
	j -= 8;
	k -= 8;

	minBlockX = 0x7fffffff;
	minBlockY = 0x7fffffff;
	minBlockZ = 0x7fffffff;
	maxBlockX = 0x80000000;
	maxBlockY = 0x80000000;
	maxBlockZ = 0x80000000;

	int_t l = renderChunksWide * 16;
	int_t i1 = l / 2;

	for (int_t j1 = 0; j1 < renderChunksWide; j1++)
	{
		int_t k1 = j1 * 16;
		int_t l1 = (k1 + i1) - i;

		if (l1 < 0)
			l1 -= l - 1;

		l1 /= l;
		k1 -= l1 * l;

		if (k1 < minBlockX)
			minBlockX = k1;
		if (k1 > maxBlockX)
			maxBlockX = k1;

		for (int_t i2 = 0; i2 < renderChunksDeep; i2++)
		{
			int_t j2 = i2 * 16;
			int_t k2 = (j2 + i1) - k;

			if (k2 < 0)
				k2 -= l - 1;

			k2 /= l;
			j2 -= k2 * l;

			if (j2 < minBlockZ)
				minBlockZ = j2;
			if (j2 > maxBlockZ)
				maxBlockZ = j2;

			for (int_t l2 = 0; l2 < renderChunksTall; l2++)
			{
				int_t i3 = l2 * 16;

				if (i3 < minBlockY)
					minBlockY = i3;
				if (i3 > maxBlockY)
					maxBlockY = i3;

				WorldRenderer *worldrenderer = worldRenderers[(i2 * renderChunksTall + l2) * renderChunksWide + j1];
				bool flag = worldrenderer->needsUpdate;
				worldrenderer->setPosition(k1, i3, j2);

				if (!flag && worldrenderer->needsUpdate)
					enqueueRendererUpdate(worldrenderer);
			}
		}
	}
}

void RenderGlobal::enqueueRendererUpdate(WorldRenderer *worldrenderer)
{
	if (worldrenderer == nullptr)
		return;
	for (WorldRenderer *w : worldRenderersQueuedForUpdate)
		if (w == worldrenderer)
			return;
	worldRenderersQueuedForUpdate.push_back(worldrenderer);
	worldRenderersToUpdate.push_back(worldrenderer);
	g_dbgEnq.fetch_add(1, std::memory_order_relaxed);
}

void RenderGlobal::dequeueRendererUpdate(WorldRenderer *worldrenderer)
{
	if (worldrenderer == nullptr)
		return;
	for (size_t i = 0; i < worldRenderersQueuedForUpdate.size(); i++)
		if (worldRenderersQueuedForUpdate[i] == worldrenderer)
		{
			worldRenderersQueuedForUpdate.erase(worldRenderersQueuedForUpdate.begin() + i);
			break;
		}
}

int_t RenderGlobal::sortAndRender(EntityLiving *entityliving, int_t i, double d)
{
	for (int_t j = 0; j < 10; j++)
	{
		worldRenderersCheckIndex = (worldRenderersCheckIndex + 1) % (renderChunksWide * renderChunksTall * renderChunksDeep);
		WorldRenderer *worldrenderer = worldRenderers[worldRenderersCheckIndex];

		if (worldrenderer->needsUpdate)
			enqueueRendererUpdate(worldrenderer);
	}

	if (mc->gameSettings->renderDistance != renderDistance)
		loadRenderers();

	if (i == 0)
	{
		renderersLoaded = 0;
		renderersBeingClipped = 0;
		renderersBeingOccluded = 0;
		renderersBeingRendered = 0;
		renderersSkippingRenderPass = 0;
	}

	double d1 = entityliving->lastTickPosX + (entityliving->posX - entityliving->lastTickPosX) * d;
	double d2 = entityliving->lastTickPosY + (entityliving->posY - entityliving->lastTickPosY) * d;
	double d3 = entityliving->lastTickPosZ + (entityliving->posZ - entityliving->lastTickPosZ) * d;

	double d4 = entityliving->posX - prevSortX;
	double d5 = entityliving->posY - prevSortY;
	double d6 = entityliving->posZ - prevSortZ;

	if (d4 * d4 + d5 * d5 + d6 * d6 > 16.0)
	{
		prevSortX = entityliving->posX;
		prevSortY = entityliving->posY;
		prevSortZ = entityliving->posZ;
		markRenderersForNewPosition(MathHelper::floor_double(entityliving->posX), MathHelper::floor_double(entityliving->posY), MathHelper::floor_double(entityliving->posZ));
		// Sort renderers front-to-back. Required by Advanced OpenGL occlusion
		// culling: the nearest chunks must be rendered first so the depth buffer
		// holds the closest occluders before farther chunks' boxes are queried.
		int_t totalRenderers = renderChunksWide * renderChunksTall * renderChunksDeep;
		std::sort(sortedWorldRenderers, sortedWorldRenderers + totalRenderers, EntitySorter(entityliving));
	}

	RenderHelper::disableStandardItemLighting();

	int_t k = 0;

	if (occlusionEnabled && mc->gameSettings->advancedOpengl && !mc->gameSettings->anaglyph && i == 0)
	{
		int_t l = 0;
		int_t i1 = 16;
		checkOcclusionQueryResult(l, i1);

		for (int_t j1 = l; j1 < i1; j1++)
			sortedWorldRenderers[j1]->isVisible = true;

		k += renderSortedRenderers(l, i1, i, d);

		do
		{
			int_t byte0 = i1;
			i1 *= 2;
			if (i1 > renderChunksWide * renderChunksTall * renderChunksDeep)
				i1 = renderChunksWide * renderChunksTall * renderChunksDeep;

			glDisable(GL_TEXTURE_2D);
			glDisable(GL_LIGHTING);
			glDisable(GL_ALPHA_TEST);
			glDisable(GL_FOG);
			glColorMask(false, false, false, false);
			glDepthMask(false);

			checkOcclusionQueryResult(byte0, i1);

			glPushMatrix();
			float f = 0.0f;
			float f1 = 0.0f;
			float f2 = 0.0f;

			for (int_t k1 = byte0; k1 < i1; k1++)
			{
				if (sortedWorldRenderers[k1]->skipAllRenderPasses())
				{
					sortedWorldRenderers[k1]->isInFrustum = false;
					continue;
				}

				if (!sortedWorldRenderers[k1]->isInFrustum)
				{
					sortedWorldRenderers[k1]->isVisible = true;
				}

				if (!sortedWorldRenderers[k1]->isInFrustum || sortedWorldRenderers[k1]->isWaitingOnOcclusionQuery)
					continue;

				float f3 = MathHelper::sqrt_float(sortedWorldRenderers[k1]->distanceToEntitySquared(entityliving));
				int_t l1 = (int_t)(1.0f + f3 / 128.0f);

				if (cloudOffsetX % l1 != k1 % l1)
					continue;

				WorldRenderer *worldrenderer1 = sortedWorldRenderers[k1];
				float f4 = (float)((double)worldrenderer1->posXMinus - d1);
				float f5 = (float)((double)worldrenderer1->posYMinus - d2);
				float f6 = (float)((double)worldrenderer1->posZMinus - d3);
				float f7 = f4 - f;
				float f8 = f5 - f1;
				float f9 = f6 - f2;

				if (f7 != 0.0f || f8 != 0.0f || f9 != 0.0f)
				{
					glTranslatef(f7, f8, f9);
					f += f7;
					f1 += f8;
					f2 += f9;
				}

				glBeginQueryARB(GL_SAMPLES_PASSED_ARB, sortedWorldRenderers[k1]->glOcclusionQuery);
				sortedWorldRenderers[k1]->callOcclusionQueryList();
				glEndQueryARB(GL_SAMPLES_PASSED_ARB);
				sortedWorldRenderers[k1]->isWaitingOnOcclusionQuery = true;
			}

			glPopMatrix();

			if (mc->gameSettings->anaglyph)
			{
				if (EntityRenderer::anaglyphField == 0)
					glColorMask(false, true, true, true);
				else
					glColorMask(true, false, false, true);
			}
			else
			{
				glColorMask(true, true, true, true);
			}

			glDepthMask(true);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_ALPHA_TEST);
			glEnable(GL_FOG);

			k += renderSortedRenderers(byte0, i1, i, d);
		} while (i1 < renderChunksWide * renderChunksTall * renderChunksDeep);
	}
	else
	{
		k += renderSortedRenderers(0, renderChunksWide * renderChunksTall * renderChunksDeep, i, d);
	}

	return k;
}

void RenderGlobal::checkOcclusionQueryResult(int_t i, int_t j)
{
	for (int_t k = i; k < j; k++)
	{
		if (!sortedWorldRenderers[k]->isWaitingOnOcclusionQuery)
			continue;

		glGetQueryObjectuivARB(sortedWorldRenderers[k]->glOcclusionQuery, GL_QUERY_RESULT_AVAILABLE_ARB, reinterpret_cast<GLuint *>(occlusionResult.data()));

		if (occlusionResult[0] != 0)
		{
			sortedWorldRenderers[k]->isWaitingOnOcclusionQuery = false;
			glGetQueryObjectuivARB(sortedWorldRenderers[k]->glOcclusionQuery, GL_QUERY_RESULT_ARB, reinterpret_cast<GLuint *>(occlusionResult.data()));
			sortedWorldRenderers[k]->isVisible = occlusionResult[0] != 0;
		}
	}
}

int_t RenderGlobal::renderSortedRenderers(int_t i, int_t j, int_t k, double d)
{
	glRenderLists.clear();

	int_t l = 0;
	for (int_t i1 = i; i1 < j; i1++)
	{
		if (k == 0)
		{
			renderersLoaded++;
			if (sortedWorldRenderers[i1]->skipRenderPass(k))
			{
				renderersSkippingRenderPass++;
			}
			else if (!sortedWorldRenderers[i1]->isInFrustum)
			{
				renderersBeingClipped++;
			}
			else if (occlusionEnabled && !sortedWorldRenderers[i1]->isVisible)
			{
				renderersBeingOccluded++;
			}
			else
			{
				renderersBeingRendered++;
			}
		}

		if (sortedWorldRenderers[i1]->skipRenderPass(k))
			continue;

		int_t j1 = sortedWorldRenderers[i1]->getGLCallListForPass(k);
		if (j1 >= 0)
		{
			glRenderLists.push_back(sortedWorldRenderers[i1]);
			l++;
		}
	}

	EntityLiving *entityliving = mc->renderViewEntity;
	double d1 = entityliving->lastTickPosX + (entityliving->posX - entityliving->lastTickPosX) * d;
	double d2 = entityliving->lastTickPosY + (entityliving->posY - entityliving->lastTickPosY) * d;
	double d3 = entityliving->lastTickPosZ + (entityliving->posZ - entityliving->lastTickPosZ) * d;

	int_t k1 = 0;
	for (int_t l1 = 0; l1 < 4; l1++)
		allRenderLists[l1]->reset();

	for (size_t i2 = 0; i2 < glRenderLists.size(); i2++)
	{
		WorldRenderer *worldrenderer = glRenderLists[i2];
		if (k == 0 && i2 < 4)
		{
			static long_t dwLast = 0;
			long_t dwNow = System::currentTimeMillis();
			if (dwNow - dwLast > 3000)
			{
				dwLast = dwNow;
				(void)0;
			}
		}
		int_t j2 = -1;

		for (int_t k2 = 0; k2 < k1; k2++)
		{
			if (allRenderLists[k2]->matchesPos(worldrenderer->posXMinus, worldrenderer->posYMinus, worldrenderer->posZMinus))
				j2 = k2;
		}

		if (j2 < 0)
		{
			j2 = k1++;
			allRenderLists[j2]->setup(worldrenderer->posXMinus, worldrenderer->posYMinus, worldrenderer->posZMinus, d1, d2, d3);
		}

		allRenderLists[j2]->addGLRenderList(worldrenderer->getGLCallListForPass(k));
	}

	renderAllRenderLists(k, d);
	{
		static long_t cwLastLog = 0;
		long_t cwNow = System::currentTimeMillis();
		if (k == 0 && cwNow - cwLastLog > 3000)
		{
			cwLastLog = cwNow;
			if (worldObj != nullptr && worldObj->getIChunkProvider() != nullptr && k == 0)
			{
				static long_t ccLast = 0;
				long_t ccNow = System::currentTimeMillis();
				if (ccNow - ccLast > 4000)
				{
					ccLast = ccNow;
					jstring ms = worldObj->getIChunkProvider()->makeString();
					(void)0;
				}
			}
			(void)0;
			long yBuilt[8] = {0}, yDrawn[8] = {0};
			int_t totalR = renderChunksWide * renderChunksTall * renderChunksDeep;
			int unbuiltLogged = 0;
			for (int_t q2 = 0; q2 < totalR; q2++)
			{
				int_t yl = (worldRenderers[q2]->posY / 16) & 7;
				if (worldRenderers[q2]->isBuilt()) yBuilt[yl]++;
				else if (unbuiltLogged < 8 && System::currentTimeMillis() % 4000 < 1200)
				{
					unbuiltLogged++;
					(void)0;
				}
				if (!worldRenderers[q2]->skipRenderPass(0)) yDrawn[yl]++;
			}
			(void)0;
		}
	}
	return l;
}

void RenderGlobal::updateClouds()
{
	cloudOffsetX++;
}

void RenderGlobal::renderSky(float f)
{
	if (mc->theWorld->worldProvider->isNether)
		return;

	glDisable(GL_TEXTURE_2D);
	Vec3D *vec3d = worldObj->getSkyColor(mc->renderViewEntity, f);
	float f1 = (float)vec3d->xCoord;
	float f2 = (float)vec3d->yCoord;
	float f3 = (float)vec3d->zCoord;

	{
		static long_t s_lastSkyLog = 0;
		const long_t nowL = System::currentTimeMillis();
		if (nowL - s_lastSkyLog >= 1000)
		{
			s_lastSkyLog = nowL;
			(void)0;
		}
	}

	if (mc->gameSettings->anaglyph)
	{
		float f4 = (f1 * 30.0f + f2 * 59.0f + f3 * 11.0f) / 100.0f;
		float f5 = (f1 * 30.0f + f2 * 70.0f) / 100.0f;
		float f7 = (f1 * 30.0f + f3 * 70.0f) / 100.0f;
		f1 = f4;
		f2 = f5;
		f3 = f7;
	}

	glColor3f(f1, f2, f3);

	Tessellator *tessellator = &Tessellator::instance;

	glDepthMask(false);
	glEnable(GL_FOG);
	glColor3f(f1, f2, f3);
	if (Config::isSkyEnabled()) // OptiFine: Sky OFF (sol/luna/estrellas siguen visibles)
		glCallList(glSkyList);
	glDisable(GL_FOG);
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	RenderHelper::disableStandardItemLighting();

	float *af = worldObj->worldProvider->calcSunriseSunsetColors(worldObj->getCelestialAngle(f), f);

	if (af != nullptr)
	{
		glDisable(GL_TEXTURE_2D);
		glShadeModel(GL_SMOOTH);
		glPushMatrix();
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

		float f8 = worldObj->getCelestialAngle(f);
		glRotatef(f8 <= 0.5f ? 0.0f : 180.0f, 0.0f, 0.0f, 1.0f);

		float f10 = af[0];
		float f12 = af[1];
		float f14 = af[2];

		if (mc->gameSettings->anaglyph)
		{
			float f16 = (f10 * 30.0f + f12 * 59.0f + f14 * 11.0f) / 100.0f;
			float f18 = (f10 * 30.0f + f12 * 70.0f) / 100.0f;
			float f19 = (f10 * 30.0f + f14 * 70.0f) / 100.0f;
			f10 = f16;
			f12 = f18;
			f14 = f19;
		}

		tessellator->startDrawing(6);
		tessellator->setColorRGBA_F(f10, f12, f14, af[3]);
		tessellator->addVertex(0.0, 100.0, 0.0);

		int_t i = 16;
		tessellator->setColorRGBA_F(af[0], af[1], af[2], 0.0f);

		for (int_t j = 0; j <= i; j++)
		{
			float f20 = ((float)j * 3.141593f * 2.0f) / (float)i;
			float f21 = MathHelper::sin(f20);
			float f22 = MathHelper::cos(f20);
			tessellator->addVertex(f21 * 120.0f, f22 * 120.0f, -f22 * 40.0f * af[3]);
		}

		tessellator->draw();
		glPopMatrix();
		glShadeModel(GL_FLAT);
	}

	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glPushMatrix();

	float f6 = 1.0f - worldObj->func_27162_g(f);
	float f9 = 0.0f;
	float f11 = 0.0f;
	float f13 = 0.0f;

	glColor4f(1.0f, 1.0f, 1.0f, f6);
	glTranslatef(f9, f11, f13);
	glRotatef(0.0f, 0.0f, 0.0f, 1.0f);
	glRotatef(worldObj->getCelestialAngle(f) * 360.0f, 1.0f, 0.0f, 0.0f);

	float f15 = 30.0f;
	glBindTexture(GL_TEXTURE_2D, renderEngine->getTexture("/terrain/sun.png"));
	tessellator->startDrawingQuads();
	tessellator->addVertexWithUV(-f15, 100.0, -f15, 0.0, 0.0);
	tessellator->addVertexWithUV(f15, 100.0, -f15, 1.0, 0.0);
	tessellator->addVertexWithUV(f15, 100.0, f15, 1.0, 1.0);
	tessellator->addVertexWithUV(-f15, 100.0, f15, 0.0, 1.0);
	tessellator->draw();

	f15 = 20.0f;
	glBindTexture(GL_TEXTURE_2D, renderEngine->getTexture("/terrain/moon.png"));
	tessellator->startDrawingQuads();
	tessellator->addVertexWithUV(-f15, -100.0, f15, 1.0, 1.0);
	tessellator->addVertexWithUV(f15, -100.0, f15, 0.0, 1.0);
	tessellator->addVertexWithUV(f15, -100.0, -f15, 0.0, 0.0);
	tessellator->addVertexWithUV(-f15, -100.0, -f15, 1.0, 0.0);
	tessellator->draw();

	glDisable(GL_TEXTURE_2D);

	float f17 = worldObj->getStarBrightness(f) * f6;
	if (f17 > 0.0f)
	{
		glColor4f(f17, f17, f17, f17);
		glCallList(starGLCallList);
	}

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_FOG);
	glPopMatrix();

	if (worldObj->worldProvider->hasSkyColorBlend())
		glColor3f(f1 * 0.2f + 0.04f, f2 * 0.2f + 0.04f, f3 * 0.6f + 0.1f);
	else
		glColor3f(f1, f2, f3);

	glDisable(GL_TEXTURE_2D);
	if (Config::isSkyEnabled()) // OptiFine: Sky OFF (plano del horizonte/void)
		glCallList(glSkyList2);
	glEnable(GL_TEXTURE_2D);
	glDepthMask(true);
}

void RenderGlobal::renderClouds(float f)
{
	if (mc->theWorld->worldProvider->isNether)
		return;

	if (Config::isCloudsOff()) // OptiFine: Clouds OFF
		return;

	if (Config::isCloudsFancy()) // OptiFine: Clouds Fast/Fancy/Default
	{
		renderCloudsFancy(f);
		return;
	}

	glDisable(GL_CULL_FACE);

	float f1 = (float)(mc->renderViewEntity->lastTickPosY + (mc->renderViewEntity->posY - mc->renderViewEntity->lastTickPosY) * (double)f);

	byte_t byte0 = 32;
	int_t i = 256 / byte0;

	Tessellator *tessellator = &Tessellator::instance;

	glBindTexture(GL_TEXTURE_2D, renderEngine->getTexture("/environment/clouds.png"));
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Vec3D *vec3d = worldObj->getCloudFogColor(f);
	float f2 = (float)vec3d->xCoord;
	float f3 = (float)vec3d->yCoord;
	float f4 = (float)vec3d->zCoord;

	if (mc->gameSettings->anaglyph)
	{
		float f5 = (f2 * 30.0f + f3 * 59.0f + f4 * 11.0f) / 100.0f;
		float f7 = (f2 * 30.0f + f3 * 70.0f) / 100.0f;
		float f8 = (f2 * 30.0f + f4 * 70.0f) / 100.0f;
		f2 = f5;
		f3 = f7;
		f4 = f8;
	}

	float f6 = 0.0004882813f;

	double d = mc->renderViewEntity->prevPosX + (mc->renderViewEntity->posX - mc->renderViewEntity->prevPosX) * (double)f + (double)(((float)cloudOffsetX + f) * 0.03f);
	double d1 = mc->renderViewEntity->prevPosZ + (mc->renderViewEntity->posZ - mc->renderViewEntity->prevPosZ) * (double)f;

	int_t j = MathHelper::floor_double(d / 2048.0);
	int_t k = MathHelper::floor_double(d1 / 2048.0);

	d -= j * 2048;
	d1 -= k * 2048;

	float f9 = (worldObj->worldProvider->getCloudHeight() - f1) + 0.33f;
	float f10 = (float)(d * (double)f6);
	float f11 = (float)(d1 * (double)f6);

	tessellator->startDrawingQuads();
	tessellator->setColorRGBA_F(f2, f3, f4, 0.8f);

	for (int_t l = -byte0 * i; l < byte0 * i; l += byte0)
	{
		for (int_t i1 = -byte0 * i; i1 < byte0 * i; i1 += byte0)
		{
			tessellator->addVertexWithUV(l + 0, f9, i1 + byte0, (float)(l + 0) * f6 + f10, (float)(i1 + byte0) * f6 + f11);
			tessellator->addVertexWithUV(l + byte0, f9, i1 + byte0, (float)(l + byte0) * f6 + f10, (float)(i1 + byte0) * f6 + f11);
			tessellator->addVertexWithUV(l + byte0, f9, i1 + 0, (float)(l + byte0) * f6 + f10, (float)(i1 + 0) * f6 + f11);
			tessellator->addVertexWithUV(l + 0, f9, i1 + 0, (float)(l + 0) * f6 + f10, (float)(i1 + 0) * f6 + f11);
		}
	}

	tessellator->draw();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
}

bool RenderGlobal::func_27307_a(double d, double d1, double d2, float f)
{
	return false;
}

void RenderGlobal::renderCloudsFancy(float f)
{
	glDisable(GL_CULL_FACE);

	float f1 = (float)(mc->renderViewEntity->lastTickPosY + (mc->renderViewEntity->posY - mc->renderViewEntity->lastTickPosY) * (double)f);

	Tessellator *tessellator = &Tessellator::instance;

	float f2 = 12.0f;
	float f3 = 4.0f;

	double d = (mc->renderViewEntity->prevPosX + (mc->renderViewEntity->posX - mc->renderViewEntity->prevPosX) * (double)f + (double)(((float)cloudOffsetX + f) * 0.03f)) / (double)f2;
	double d1 = (mc->renderViewEntity->prevPosZ + (mc->renderViewEntity->posZ - mc->renderViewEntity->prevPosZ) * (double)f) / (double)f2 + 0.33000001311302185;

	float f4 = (worldObj->worldProvider->getCloudHeight() - f1) + 0.33f;

	int_t i = MathHelper::floor_double(d / 2048.0);
	int_t j = MathHelper::floor_double(d1 / 2048.0);

	d -= i * 2048;
	d1 -= j * 2048;

	glBindTexture(GL_TEXTURE_2D, renderEngine->getTexture("/environment/clouds.png"));
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Vec3D *vec3d = worldObj->getCloudFogColor(f);
	float f5 = (float)vec3d->xCoord;
	float f6 = (float)vec3d->yCoord;
	float f7 = (float)vec3d->zCoord;

	if (mc->gameSettings->anaglyph)
	{
		float f8 = (f5 * 30.0f + f6 * 59.0f + f7 * 11.0f) / 100.0f;
		float f10 = (f5 * 30.0f + f6 * 70.0f) / 100.0f;
		float f12 = (f5 * 30.0f + f7 * 70.0f) / 100.0f;
		f5 = f8;
		f6 = f10;
		f7 = f12;
	}

	float f9 = (float)(d * 0.0);
	float f11 = (float)(d1 * 0.0);
	float f13 = 0.00390625f;

	f9 = (float)MathHelper::floor_double(d) * f13;
	f11 = (float)MathHelper::floor_double(d1) * f13;

	float f14 = (float)(d - (double)MathHelper::floor_double(d));
	float f15 = (float)(d1 - (double)MathHelper::floor_double(d1));

	int_t k = 8;
	byte_t byte0 = 3;
	float f16 = 0.0009765625f;

	glScalef(f2, 1.0f, f2);

	for (int_t l = 0; l < 2; l++)
	{
		if (l == 0)
		{
			glColorMask(false, false, false, false);
		}
		else if (mc->gameSettings->anaglyph)
		{
			if (EntityRenderer::anaglyphField == 0)
				glColorMask(false, true, true, true);
			else
				glColorMask(true, false, false, true);
		}
		else
		{
			glColorMask(true, true, true, true);
		}

		for (int_t i1 = -byte0 + 1; i1 <= byte0; i1++)
		{
			for (int_t j1 = -byte0 + 1; j1 <= byte0; j1++)
			{
				tessellator->startDrawingQuads();

				float f17 = i1 * k;
				float f18 = j1 * k;
				float f19 = f17 - f14;
				float f20 = f18 - f15;

				if (f4 > -f3 - 1.0f)
				{
					tessellator->setColorRGBA_F(f5 * 0.7f, f6 * 0.7f, f7 * 0.7f, 0.8f);
					tessellator->setNormal(0.0f, -1.0f, 0.0f);
					tessellator->addVertexWithUV(f19 + 0.0f, f4 + 0.0f, f20 + (float)k, (f17 + 0.0f) * f13 + f9, (f18 + (float)k) * f13 + f11);
					tessellator->addVertexWithUV(f19 + (float)k, f4 + 0.0f, f20 + (float)k, (f17 + (float)k) * f13 + f9, (f18 + (float)k) * f13 + f11);
					tessellator->addVertexWithUV(f19 + (float)k, f4 + 0.0f, f20 + 0.0f, (f17 + (float)k) * f13 + f9, (f18 + 0.0f) * f13 + f11);
					tessellator->addVertexWithUV(f19 + 0.0f, f4 + 0.0f, f20 + 0.0f, (f17 + 0.0f) * f13 + f9, (f18 + 0.0f) * f13 + f11);
				}

				if (f4 <= f3 + 1.0f)
				{
					tessellator->setColorRGBA_F(f5, f6, f7, 0.8f);
					tessellator->setNormal(0.0f, 1.0f, 0.0f);
					tessellator->addVertexWithUV(f19 + 0.0f, (f4 + f3) - f16, f20 + (float)k, (f17 + 0.0f) * f13 + f9, (f18 + (float)k) * f13 + f11);
					tessellator->addVertexWithUV(f19 + (float)k, (f4 + f3) - f16, f20 + (float)k, (f17 + (float)k) * f13 + f9, (f18 + (float)k) * f13 + f11);
					tessellator->addVertexWithUV(f19 + (float)k, (f4 + f3) - f16, f20 + 0.0f, (f17 + (float)k) * f13 + f9, (f18 + 0.0f) * f13 + f11);
					tessellator->addVertexWithUV(f19 + 0.0f, (f4 + f3) - f16, f20 + 0.0f, (f17 + 0.0f) * f13 + f9, (f18 + 0.0f) * f13 + f11);
				}

				tessellator->setColorRGBA_F(f5 * 0.9f, f6 * 0.9f, f7 * 0.9f, 0.8f);

				if (i1 > -1)
				{
					tessellator->setNormal(-1.0f, 0.0f, 0.0f);
					for (int_t k1 = 0; k1 < k; k1++)
					{
						tessellator->addVertexWithUV(f19 + (float)k1 + 0.0f, f4 + 0.0f, f20 + (float)k, (f17 + (float)k1 + 0.5f) * f13 + f9, (f18 + (float)k) * f13 + f11);
						tessellator->addVertexWithUV(f19 + (float)k1 + 0.0f, f4 + f3, f20 + (float)k, (f17 + (float)k1 + 0.5f) * f13 + f9, (f18 + (float)k) * f13 + f11);
						tessellator->addVertexWithUV(f19 + (float)k1 + 0.0f, f4 + f3, f20 + 0.0f, (f17 + (float)k1 + 0.5f) * f13 + f9, (f18 + 0.0f) * f13 + f11);
						tessellator->addVertexWithUV(f19 + (float)k1 + 0.0f, f4 + 0.0f, f20 + 0.0f, (f17 + (float)k1 + 0.5f) * f13 + f9, (f18 + 0.0f) * f13 + f11);
					}
				}

				if (i1 <= 1)
				{
					tessellator->setNormal(1.0f, 0.0f, 0.0f);
					for (int_t l1 = 0; l1 < k; l1++)
					{
						tessellator->addVertexWithUV((f19 + (float)l1 + 1.0f) - f16, f4 + 0.0f, f20 + (float)k, (f17 + (float)l1 + 0.5f) * f13 + f9, (f18 + (float)k) * f13 + f11);
						tessellator->addVertexWithUV((f19 + (float)l1 + 1.0f) - f16, f4 + f3, f20 + (float)k, (f17 + (float)l1 + 0.5f) * f13 + f9, (f18 + (float)k) * f13 + f11);
						tessellator->addVertexWithUV((f19 + (float)l1 + 1.0f) - f16, f4 + f3, f20 + 0.0f, (f17 + (float)l1 + 0.5f) * f13 + f9, (f18 + 0.0f) * f13 + f11);
						tessellator->addVertexWithUV((f19 + (float)l1 + 1.0f) - f16, f4 + 0.0f, f20 + 0.0f, (f17 + (float)l1 + 0.5f) * f13 + f9, (f18 + 0.0f) * f13 + f11);
					}
				}

				tessellator->setColorRGBA_F(f5 * 0.8f, f6 * 0.8f, f7 * 0.8f, 0.8f);

				if (j1 > -1)
				{
					tessellator->setNormal(0.0f, 0.0f, -1.0f);
					for (int_t i2 = 0; i2 < k; i2++)
					{
						tessellator->addVertexWithUV(f19 + 0.0f, f4 + f3, f20 + (float)i2 + 0.0f, (f17 + 0.0f) * f13 + f9, (f18 + (float)i2 + 0.5f) * f13 + f11);
						tessellator->addVertexWithUV(f19 + (float)k, f4 + f3, f20 + (float)i2 + 0.0f, (f17 + (float)k) * f13 + f9, (f18 + (float)i2 + 0.5f) * f13 + f11);
						tessellator->addVertexWithUV(f19 + (float)k, f4 + 0.0f, f20 + (float)i2 + 0.0f, (f17 + (float)k) * f13 + f9, (f18 + (float)i2 + 0.5f) * f13 + f11);
						tessellator->addVertexWithUV(f19 + 0.0f, f4 + 0.0f, f20 + (float)i2 + 0.0f, (f17 + 0.0f) * f13 + f9, (f18 + (float)i2 + 0.5f) * f13 + f11);
					}
				}

				if (j1 <= 1)
				{
					tessellator->setNormal(0.0f, 0.0f, 1.0f);
					for (int_t j2 = 0; j2 < k; j2++)
					{
						tessellator->addVertexWithUV(f19 + 0.0f, f4 + f3, (f20 + (float)j2 + 1.0f) - f16, (f17 + 0.0f) * f13 + f9, (f18 + (float)j2 + 0.5f) * f13 + f11);
						tessellator->addVertexWithUV(f19 + (float)k, f4 + f3, (f20 + (float)j2 + 1.0f) - f16, (f17 + (float)k) * f13 + f9, (f18 + (float)j2 + 0.5f) * f13 + f11);
						tessellator->addVertexWithUV(f19 + (float)k, f4 + 0.0f, (f20 + (float)j2 + 1.0f) - f16, (f17 + (float)k) * f13 + f9, (f18 + (float)j2 + 0.5f) * f13 + f11);
						tessellator->addVertexWithUV(f19 + 0.0f, f4 + 0.0f, (f20 + (float)j2 + 1.0f) - f16, (f17 + 0.0f) * f13 + f9, (f18 + (float)j2 + 0.5f) * f13 + f11);
					}
				}

				tessellator->draw();
			}
		}
	}

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
}

bool RenderGlobal::updateRenderers(EntityLiving *entityliving, bool flag)
{
	int dbgN = -1;
	{
		static std::atomic<int> dbgC{0};
		dbgN = dbgC.fetch_add(1);
		if (dbgN < 12)
			(void)0;
	}
	bool flag1 = false;

	if (flag1)
	{
		// Collections.sort(worldRenderersToUpdate, new RenderSorter(entityliving));
		std::sort(worldRenderersToUpdate.begin(), worldRenderersToUpdate.end(), RenderSorter(entityliving));

		int_t i = (int_t)worldRenderersToUpdate.size() - 1;
		int_t j = (int_t)worldRenderersToUpdate.size();

		for (int_t k = 0; k < j; k++)
		{
			WorldRenderer *worldrenderer = worldRenderersToUpdate[i - k];

			if (!flag)
			{
				if (worldrenderer->distanceToEntitySquared(entityliving) > 256.0f)
				{
					if (worldrenderer->isInFrustum)
					{
						if (k >= 3)
							return false;
					}
					else if (k >= 1)
					{
						return false;
					}
				}
			}
			else if (!worldrenderer->isInFrustum)
			{
				continue;
			}

			worldrenderer->updateRenderer();
			dequeueRendererUpdate(worldrenderer);
			auto it = std::find(worldRenderersToUpdate.begin(), worldRenderersToUpdate.end(), worldrenderer);
			if (it != worldRenderersToUpdate.end())
				worldRenderersToUpdate.erase(it);
			worldrenderer->needsUpdate = false;
		}

		return worldRenderersToUpdate.size() == 0;
	}

	// OptiFine "Chunk Updates" (1-5): el batch de actualizaciones lejanas por frame
	// era fijo en 2; ahora lo controla la opcion. Mas alto = mundo carga mas rapido, menos FPS.
	// IMPORTANTE: este algoritmo reserva un slot-guarda (hace `--k2` antes de `if (--k2<=0) continue`),
	// asi que la capacidad UTIL es byte0-1. Por eso el original era 2 (=1 update/frame). Hay que
	// sumar 1 a la opcion: getUpdatesPerFrame()==1 -> byte0==2 (= comportamiento vanilla original).
	// NOTA: "Dynamic Updates" (x3 con jugador quieto) y la ruta MT se cablean en F7.
	int_t byte0 = Config::getUpdatesPerFrame() + 1;
	if (byte0 < 2)
		byte0 = 2;
	RenderSorter rendersorter(entityliving);
	// 8 ms, not 20: the budget is only checked between builds, so a dense
	// chunk still overshoots; a smaller window keeps the 4S frame time stable
	// (terrain fills in slightly slower but the fps stops collapsing).
	const long_t cwBudgetDeadline = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() + 8000000LL;
	int_t cwBuilt = 0;
	std::vector<WorldRenderer *> aworldrenderer(byte0, nullptr);
	std::vector<WorldRenderer *> arraylist;
	int_t l = (int_t)worldRenderersToUpdate.size();
	int_t i1 = 0;

	for (int_t j1 = 0; j1 < l; j1++)
	{
		WorldRenderer *worldrenderer1 = worldRenderersToUpdate[j1];

		if (!flag && worldrenderer1->isBuilt())
		{
			if (worldrenderer1->distanceToEntitySquared(entityliving) > 256.0f)
			{
				int_t k2;
				for (k2 = 0; k2 < byte0 && (aworldrenderer[k2] == nullptr || rendersorter.doCompare(aworldrenderer[k2], worldrenderer1) <= 0); k2++) {}

				if (--k2 <= 0)
					continue;

				for (int_t i3 = k2; --i3 != 0;)
					aworldrenderer[i3 - 1] = aworldrenderer[i3];

				aworldrenderer[k2] = worldrenderer1;
				continue;
			}
		}
		else if (worldrenderer1->isBuilt() && !worldrenderer1->isInFrustum)
		{
			// Only ALREADY-BUILT renderers are frustum-culled. Never-built
			// ones must always reach the build list, otherwise chunks only
			// appear when the camera points at them (or you walk into them).
			continue;
		}

		if (arraylist.empty())
			arraylist = std::vector<WorldRenderer *>();

		i1++;
		arraylist.push_back(worldrenderer1);
		worldRenderersToUpdate[j1] = nullptr;
	}

	if (dbgN >= 0 && dbgN < 12)
		(void)0;

	if (!arraylist.empty())
	{
		if (arraylist.size() > 1)
		{
			std::sort(arraylist.begin(), arraylist.end(), rendersorter);
		}

		int_t k1 = (int_t)arraylist.size() - 1;
		for (; k1 >= 0; k1--)
		{
			if (cwBuilt > 0 && std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() > cwBudgetDeadline) break;
			WorldRenderer *worldrenderer2 = arraylist[k1];
			if (!worldrenderer2->updateRenderer()) { worldRenderersToUpdate.push_back(worldrenderer2); continue; }
			if (dbgN >= 0 && dbgN < 12)
				(void)0;
			worldrenderer2->needsUpdate = false;
			if (dbgN >= 0 && dbgN < 12)
				(void)0;
			dequeueRendererUpdate(worldrenderer2);
			cwBuilt++;
			if (dbgN >= 0 && dbgN < 12)
				(void)0;
		}
		// Budget hit: arraylist[0..k1] were never examined. They were already
		// removed from worldRenderersToUpdate by the collection loop, so
		// requeue them or they are lost forever (only the load chunk builds).
		for (int_t r = k1; r >= 0; r--)
			worldRenderersToUpdate.push_back(arraylist[r]);
	}

	if (dbgN >= 0 && dbgN < 12)
		(void)0;

	int_t l1 = 0;
	int_t i2 = byte0 - 1;
	for (; i2 >= 0; i2--)
	{
		WorldRenderer *worldrenderer3 = aworldrenderer[i2];
		if (worldrenderer3 == nullptr)
			continue;

		if (cwBuilt > 0 && std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() > cwBudgetDeadline) break;
		if (!aworldrenderer[i2]->updateRenderer())
		{
			// Requeue AND clear the slot: the compaction below drops any
			// renderer still referenced here, which would undo the requeue
			// and leave it dirty-but-unqueued forever (missing chunk column).
			worldRenderersToUpdate.push_back(aworldrenderer[i2]);
			aworldrenderer[i2] = nullptr;
			continue;
		}
		aworldrenderer[i2]->needsUpdate = false;
		aworldrenderer[i2] = nullptr;
		dequeueRendererUpdate(worldrenderer3);
		l1++;
		cwBuilt++;
	}
	// Budget hit: aworldrenderer[0..i2] were never processed, and the
	// compaction below drops any renderer still referenced there from the
	// queue. Requeue them or they are lost forever -> unbuilt holes
	// (chunks that never draw until a full world reload).
	for (int_t r = i2; r >= 0; r--)
		if (aworldrenderer[r] != nullptr)
		{
			worldRenderersToUpdate.push_back(aworldrenderer[r]);
			aworldrenderer[r] = nullptr;
		}

	if (dbgN >= 0 && dbgN < 12)
		(void)0;

	int_t j2 = 0;
	int_t l2 = 0;
	for (int_t j3 = (int_t)worldRenderersToUpdate.size(); j2 != j3; j2++)
	{
		WorldRenderer *worldrenderer4 = worldRenderersToUpdate[j2];
		if (worldrenderer4 == nullptr)
			continue;

		bool flag2 = false;
		for (int_t k3 = 0; k3 < byte0 && !flag2; k3++)
		{
			if (worldrenderer4 == aworldrenderer[k3])
				flag2 = true;
		}

		if (flag2)
			continue;

		if (l2 != j2)
			worldRenderersToUpdate[l2] = worldrenderer4;

		l2++;
	}

	worldRenderersToUpdate.resize(l2);

	if (dbgN >= 0 && dbgN < 12)
		(void)0;

	return l == i1 + l1;
}

void RenderGlobal::drawBlockBreaking(EntityPlayer *entityplayer, MovingObjectPosition *movingobjectposition, int_t i, ItemStack *itemstack, float f)
{
	Tessellator *tessellator = &Tessellator::instance;

	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glColor4f(1.0f, 1.0f, 1.0f, (MathHelper::sin((float)SDL_GetTicks() / 100.0f) * 0.2f + 0.4f) * 0.5f);

	if (i == 0)
	{
		if (damagePartialTime > 0.0f)
		{
			glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
			int_t j = renderEngine->getTexture("/terrain.png");
			glBindTexture(GL_TEXTURE_2D, j);
			glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
			glPushMatrix();

			int_t k = worldObj->getBlockId(movingobjectposition->blockX, movingobjectposition->blockY, movingobjectposition->blockZ);
			Block *block = k <= 0 ? nullptr : Block::blocksList[k];

			glDisable(GL_ALPHA_TEST);
			glPolygonOffset(-3.0f, -3.0f);
			glEnable(GL_POLYGON_OFFSET_FILL);

			double d = entityplayer->lastTickPosX + (entityplayer->posX - entityplayer->lastTickPosX) * (double)f;
			double d1 = entityplayer->lastTickPosY + (entityplayer->posY - entityplayer->lastTickPosY) * (double)f;
			double d2 = entityplayer->lastTickPosZ + (entityplayer->posZ - entityplayer->lastTickPosZ) * (double)f;

			if (block == nullptr)
				block = Block::stone;

			glEnable(GL_ALPHA_TEST);
			tessellator->startDrawingQuads();
			tessellator->setTranslationD(-d, -d1, -d2);
			tessellator->disableColor();
			globalRenderBlocks->renderBlockUsingTexture(block, movingobjectposition->blockX, movingobjectposition->blockY, movingobjectposition->blockZ, 240 + (int_t)(damagePartialTime * 10.0f));
			tessellator->draw();
			tessellator->setTranslationD(0.0, 0.0, 0.0);

			glDisable(GL_ALPHA_TEST);
			glPolygonOffset(0.0f, 0.0f);
			glDisable(GL_POLYGON_OFFSET_FILL);
			glEnable(GL_ALPHA_TEST);
			glDepthMask(true);
			glPopMatrix();
		}
	}
	else if (itemstack != nullptr)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		float f1 = MathHelper::sin((float)SDL_GetTicks() / 100.0f) * 0.2f + 0.8f;
		glColor4f(f1, f1, f1, MathHelper::sin((float)SDL_GetTicks() / 200.0f) * 0.2f + 0.5f);

		int_t l = renderEngine->getTexture("/terrain.png");
		glBindTexture(GL_TEXTURE_2D, l);

		int_t i1 = movingobjectposition->blockX;
		int_t j1 = movingobjectposition->blockY;
		int_t k1 = movingobjectposition->blockZ;

		if (movingobjectposition->sideHit == 0)
			j1--;
		if (movingobjectposition->sideHit == 1)
			j1++;
		if (movingobjectposition->sideHit == 2)
			k1--;
		if (movingobjectposition->sideHit == 3)
			k1++;
		if (movingobjectposition->sideHit == 4)
			i1--;
		if (movingobjectposition->sideHit == 5)
			i1++;
	}

	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
}

void RenderGlobal::drawChunkBorders(float f)
{
	if (mc == nullptr || mc->thePlayer == nullptr || mc->theWorld == nullptr)
		return;

	EntityPlayer *player = mc->thePlayer;
	double dx = player->lastTickPosX + (player->posX - player->lastTickPosX) * (double)f;
	double dy = player->lastTickPosY + (player->posY - player->lastTickPosY) * (double)f;
	double dz = player->lastTickPosZ + (player->posZ - player->lastTickPosZ) * (double)f;

	int_t pcx = MathHelper::floor_double(player->posX) >> 4;
	int_t pcz = MathHelper::floor_double(player->posZ) >> 4;

	const int_t range = 8;
	const double ly = (double)MathHelper::floor_double(player->posY) + 0.02 - dy;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDepthMask(false);

	Tessellator *tessellator = &Tessellator::instance;

	glColor4f(0.10f, 0.10f, 0.10f, 0.60f);
	glLineWidth(1.0f);
	tessellator->startDrawing(1);
	for (int_t cx = pcx - range; cx <= pcx + range + 1; cx++)
	{
		double x = (double)(cx * 16) - dx;
		tessellator->addVertex(x, ly, (double)((pcz - range) * 16) - dz);
		tessellator->addVertex(x, ly, (double)((pcz + range + 1) * 16) - dz);
	}
	for (int_t cz = pcz - range; cz <= pcz + range + 1; cz++)
	{
		double z = (double)(cz * 16) - dz;
		tessellator->addVertex((double)((pcx - range) * 16) - dx, ly, z);
		tessellator->addVertex((double)((pcx + range + 1) * 16) - dx, ly, z);
	}
	tessellator->draw();

	// Vertical posts at every chunk corner so the borders read in 3D instead of
	// only as a flat grid at the player's feet.
	{
		const double yTop = (double)MathHelper::floor_double(player->posY) + 20.0 - dy;
		const double yBot = (double)MathHelper::floor_double(player->posY) - 4.0 - dy;
		glColor4f(0.10f, 0.10f, 0.10f, 0.60f);
		tessellator->startDrawing(1);
		for (int_t cx = pcx - range; cx <= pcx + range + 1; cx++)
		{
			double x = (double)(cx * 16) - dx;
			for (int_t cz = pcz - range; cz <= pcz + range + 1; cz++)
			{
				double z = (double)(cz * 16) - dz;
				tessellator->addVertex(x, yBot, z);
				tessellator->addVertex(x, yTop, z);
			}
		}
		tessellator->draw();
	}

	glColor4f(1.0f, 1.0f, 1.0f, 0.85f);
	glLineWidth(2.0f);
	double a = (double)(pcx * 16) - dx;
	double b = (double)((pcx + 1) * 16) - dx;
	double c = (double)(pcz * 16) - dz;
	double d = (double)((pcz + 1) * 16) - dz;
	tessellator->startDrawing(1);
	tessellator->addVertex(a, ly, c); tessellator->addVertex(b, ly, c);
	tessellator->addVertex(b, ly, c); tessellator->addVertex(b, ly, d);
	tessellator->addVertex(b, ly, d); tessellator->addVertex(a, ly, d);
	tessellator->addVertex(a, ly, d); tessellator->addVertex(a, ly, c);
	tessellator->draw();

	glLineWidth(1.0f);
	glDepthMask(true);
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

void RenderGlobal::drawSelectionBox(EntityPlayer *entityplayer, MovingObjectPosition *movingobjectposition, int_t i, ItemStack *itemstack, float f)
{
	if (i == 0 && movingobjectposition->typeOfHit == EnumMovingObjectType::TILE)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.0f, 0.0f, 0.0f, 0.4f);
		glLineWidth(2.0f);
		glDisable(GL_TEXTURE_2D);
		glDepthMask(false);

		float f1 = 0.002f;

		int_t j = worldObj->getBlockId(movingobjectposition->blockX, movingobjectposition->blockY, movingobjectposition->blockZ);
		if (j > 0)
		{
			Block::blocksList[j]->setBlockBoundsBasedOnState(worldObj, movingobjectposition->blockX, movingobjectposition->blockY, movingobjectposition->blockZ);

			double d = entityplayer->lastTickPosX + (entityplayer->posX - entityplayer->lastTickPosX) * (double)f;
			double d1 = entityplayer->lastTickPosY + (entityplayer->posY - entityplayer->lastTickPosY) * (double)f;
			double d2 = entityplayer->lastTickPosZ + (entityplayer->posZ - entityplayer->lastTickPosZ) * (double)f;

			drawOutlinedBoundingBox(Block::blocksList[j]->getSelectedBoundingBoxFromPool(worldObj, movingobjectposition->blockX, movingobjectposition->blockY, movingobjectposition->blockZ)->expand(f1, f1, f1)->getOffsetBoundingBox(-d, -d1, -d2));
		}

		glDepthMask(true);
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}
}

void RenderGlobal::drawOutlinedBoundingBox(AxisAlignedBB *axisalignedbb)
{	Tessellator *tessellator = &Tessellator::instance;

	tessellator->startDrawing(GL_LINE_STRIP);
	tessellator->addVertex(axisalignedbb->minX, axisalignedbb->minY, axisalignedbb->minZ);
	tessellator->addVertex(axisalignedbb->maxX, axisalignedbb->minY, axisalignedbb->minZ);
	tessellator->addVertex(axisalignedbb->maxX, axisalignedbb->minY, axisalignedbb->maxZ);
	tessellator->addVertex(axisalignedbb->minX, axisalignedbb->minY, axisalignedbb->maxZ);
	tessellator->addVertex(axisalignedbb->minX, axisalignedbb->minY, axisalignedbb->minZ);
	tessellator->draw();

	tessellator->startDrawing(GL_LINE_STRIP);
	tessellator->addVertex(axisalignedbb->minX, axisalignedbb->maxY, axisalignedbb->minZ);
	tessellator->addVertex(axisalignedbb->maxX, axisalignedbb->maxY, axisalignedbb->minZ);
	tessellator->addVertex(axisalignedbb->maxX, axisalignedbb->maxY, axisalignedbb->maxZ);
	tessellator->addVertex(axisalignedbb->minX, axisalignedbb->maxY, axisalignedbb->maxZ);
	tessellator->addVertex(axisalignedbb->minX, axisalignedbb->maxY, axisalignedbb->minZ);
	tessellator->draw();

	tessellator->startDrawing(GL_LINES);
	tessellator->addVertex(axisalignedbb->minX, axisalignedbb->minY, axisalignedbb->minZ);
	tessellator->addVertex(axisalignedbb->minX, axisalignedbb->maxY, axisalignedbb->minZ);
	tessellator->addVertex(axisalignedbb->maxX, axisalignedbb->minY, axisalignedbb->minZ);
	tessellator->addVertex(axisalignedbb->maxX, axisalignedbb->maxY, axisalignedbb->minZ);
	tessellator->addVertex(axisalignedbb->maxX, axisalignedbb->minY, axisalignedbb->maxZ);
	tessellator->addVertex(axisalignedbb->maxX, axisalignedbb->maxY, axisalignedbb->maxZ);
	tessellator->addVertex(axisalignedbb->minX, axisalignedbb->minY, axisalignedbb->maxZ);
	tessellator->addVertex(axisalignedbb->minX, axisalignedbb->maxY, axisalignedbb->maxZ);
	tessellator->draw();
}

void RenderGlobal::markBlockAndNeighborsNeedsUpdate(int_t i, int_t j, int_t k)
{
	markRenderersInRange(i - 1, j - 1, k - 1, i + 1, j + 1, k + 1);
}

void RenderGlobal::markBlockRangeNeedsUpdate(int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1)
{
	g_dbgMarks.fetch_add(1, std::memory_order_relaxed);
	markRenderersInRange(i - 1, j - 1, k - 1, l + 1, i1 + 1, j1 + 1);
}

void RenderGlobal::markRenderersInRange(int_t i, int_t j, int_t k, int_t l, int_t i1, int_t j1) // func_949_a
{
	int_t k1 = MathHelper::bucketInt(i, 16);
	int_t l1 = MathHelper::bucketInt(j, 16);
	int_t i2 = MathHelper::bucketInt(k, 16);
	int_t j2 = MathHelper::bucketInt(l, 16);
	int_t k2 = MathHelper::bucketInt(i1, 16);
	int_t l2 = MathHelper::bucketInt(j1, 16);

	for (int_t i3 = k1; i3 <= j2; i3++)
	{
		int_t j3 = i3 % renderChunksWide;
		if (j3 < 0)
			j3 += renderChunksWide;

		for (int_t k3 = l1; k3 <= k2; k3++)
		{
			int_t l3 = k3 % renderChunksTall;
			if (l3 < 0)
				l3 += renderChunksTall;

			for (int_t i4 = i2; i4 <= l2; i4++)
			{
				int_t j4 = i4 % renderChunksDeep;
				if (j4 < 0)
					j4 += renderChunksDeep;

				int_t k4 = (j4 * renderChunksTall + l3) * renderChunksWide + j3;
				WorldRenderer *worldrenderer = worldRenderers[k4];

				if (!worldrenderer->needsUpdate)
				{
					enqueueRendererUpdate(worldrenderer);
					worldrenderer->markDirty();
				}
			}
		}
	}
}

void RenderGlobal::playSound(const jstring &s, double d, double d1, double d2, float f, float f1)
{
	float f2 = 16.0f;
	if (f > 1.0f)
		f2 *= f;

	if (mc->renderViewEntity->getDistanceSq(d, d1, d2) < (double)(f2 * f2))
		mc->sndManager->playSound(s, (float)d, (float)d1, (float)d2, f, f1);
}

void RenderGlobal::spawnParticle(const jstring &s, double d, double d1, double d2, double d3, double d4, double d5)
{
	if (mc == nullptr || mc->renderViewEntity == nullptr || mc->effectRenderer == nullptr)
		return;

	double d6 = mc->renderViewEntity->posX - d;
	double d7 = mc->renderViewEntity->posY - d1;
	double d8 = mc->renderViewEntity->posZ - d2;
	double d9 = 16.0;

	if (d6 * d6 + d7 * d7 + d8 * d8 > d9 * d9)
		return;

	if (s == u"bubble")
		mc->effectRenderer->addEffect(new EntityBubbleFX(worldObj, d, d1, d2, d3, d4, d5));
	else if (s == u"smoke")
		mc->effectRenderer->addEffect(new EntitySmokeFX(worldObj, d, d1, d2, d3, d4, d5));
	else if (s == u"note")
		mc->effectRenderer->addEffect(new EntityNoteFX(worldObj, d, d1, d2, d3, d4, d5));
	else if (s == u"portal")
		mc->effectRenderer->addEffect(new EntityPortalFX(worldObj, d, d1, d2, d3, d4, d5));
	else if (s == u"explode")
		mc->effectRenderer->addEffect(new EntityExplodeFX(worldObj, d, d1, d2, d3, d4, d5));
	else if (s == u"flame")
		mc->effectRenderer->addEffect(new EntityFlameFX(worldObj, d, d1, d2, d3, d4, d5));
	else if (s == u"lava")
		mc->effectRenderer->addEffect(new EntityLavaFX(worldObj, d, d1, d2));
	else if (s == u"footstep")
		mc->effectRenderer->addEffect(new EntityFootStepFX(renderEngine, worldObj, d, d1, d2));
	else if (s == u"splash")
		mc->effectRenderer->addEffect(new EntitySplashFX(worldObj, d, d1, d2, d3, d4, d5));
	else if (s == u"largesmoke")
		mc->effectRenderer->addEffect(new EntitySmokeFX(worldObj, d, d1, d2, d3, d4, d5, 2.5f));
	else if (s == u"reddust")
		mc->effectRenderer->addEffect(new EntityReddustFX(worldObj, d, d1, d2, (float)d3, (float)d4, (float)d5));
	else if (s == u"snowballpoof")
		mc->effectRenderer->addEffect(new EntitySlimeFX(worldObj, d, d1, d2, Item::snowball));
	else if (s == u"snowshovel")
		mc->effectRenderer->addEffect(new EntitySnowShovelFX(worldObj, d, d1, d2, d3, d4, d5));
	else if (s == u"slime")
		mc->effectRenderer->addEffect(new EntitySlimeFX(worldObj, d, d1, d2, Item::slimeBall));
	else if (s == u"heart")
		mc->effectRenderer->addEffect(new EntityHeartFX(worldObj, d, d1, d2, d3, d4, d5));
}

void RenderGlobal::obtainEntitySkin(Entity *entity)
{
	// entity->updateCloak(); // TODO
	if (!entity->skinUrl.empty())
		renderEngine->obtainImageData(entity->skinUrl, new ImageBufferDownload());

	if (!entity->cloakUrl.empty())
		renderEngine->obtainImageData(entity->cloakUrl, new ImageBufferDownload());
}

void RenderGlobal::releaseEntitySkin(Entity *entity)
{
	if (!entity->skinUrl.empty())
		renderEngine->releaseImageData(entity->skinUrl);

	if (!entity->cloakUrl.empty())
		renderEngine->releaseImageData(entity->cloakUrl);
}

void RenderGlobal::updateAllRenderers()
{
	int_t total = renderChunksWide * renderChunksTall * renderChunksDeep;
	for (int_t i = 0; i < total; i++)
	{
		if (worldRenderers[i]->isChunkLit && !worldRenderers[i]->needsUpdate)
		{
			enqueueRendererUpdate(worldRenderers[i]);
			worldRenderers[i]->markDirty();
		}
	}
}

void RenderGlobal::playRecord(const jstring &s, int_t i, int_t j, int_t k)
{
	if (!s.empty())
		mc->ingameGUI->setRecordPlayingMessage(u"C418 - " + s);

	mc->sndManager->playStreaming(s, i, j, k, 1.0f, 1.0f);
}

void RenderGlobal::doNothingWithTileEntity(int_t i, int_t j, int_t k, TileEntity *tileentity)
{
	// Does nothing - intentionally empty
}

void RenderGlobal::playAuxSFX(EntityPlayer *entityplayer, int_t i, int_t j, int_t k, int_t l, int_t i1)
{
	Random random(worldObj->rand.nextLong());

	switch (i)
	{
	default:
		break;

	case 1001:
		worldObj->playSoundEffect(j, k, l, u"random.click", 1.0f, 1.2f);
		break;

	case 1000:
		worldObj->playSoundEffect(j, k, l, u"random.click", 1.0f, 1.0f);
		break;

	case 1002:
		worldObj->playSoundEffect(j, k, l, u"random.bow", 1.0f, 1.2f);
		break;

	case 2000:
	{
		int_t j1 = i1 % 3 - 1;
		int_t k1 = (i1 / 3) % 3 - 1;
		double d = (double)j + (double)j1 * 0.59999999999999998 + 0.5;
		double d1 = (double)k + 0.5;
		double d2 = (double)l + (double)k1 * 0.59999999999999998 + 0.5;

		for (int_t l1 = 0; l1 < 10; l1++)
		{
			double d3 = random.nextDouble() * 0.20000000000000001 + 0.01;
			double d4 = d + (double)j1 * 0.01 + (random.nextDouble() - 0.5) * (double)k1 * 0.5;
			double d5 = d1 + (random.nextDouble() - 0.5) * 0.5;
			double d6 = d2 + (double)k1 * 0.01 + (random.nextDouble() - 0.5) * (double)j1 * 0.5;
			double d7 = (double)j1 * d3 + random.nextGaussian() * 0.01;
			double d8 = -0.029999999999999999 + random.nextGaussian() * 0.01;
			double d9 = (double)k1 * d3 + random.nextGaussian() * 0.01;
			spawnParticle(u"smoke", d4, d5, d6, d7, d8, d9);
		}
		break;
	}

	case 2001:
	{
		int_t i2 = i1 & 0xff;
		if (i2 > 0)
		{
			Block *block = Block::blocksList[i2];
			mc->sndManager->playSound(block->stepSound->stepSoundDir(), (float)j + 0.5f, (float)k + 0.5f, (float)l + 0.5f, (block->stepSound->getVolume() + 1.0f) / 2.0f, block->stepSound->getPitch() * 0.8f);
		}
		// mc->effectRenderer->addBlockDestroyEffects(j, k, l, i1 & 0xff, i1 >> 8 & 0xff); // TODO
		break;
	}

	case 1003:
		if (worldObj->rand.nextDouble() < 0.5)
			worldObj->playSoundEffect((double)j + 0.5, (double)k + 0.5, (double)l + 0.5, u"random.door_open", 1.0f, worldObj->rand.nextFloat() * 0.1f + 0.9f);
		else
			worldObj->playSoundEffect((double)j + 0.5, (double)k + 0.5, (double)l + 0.5, u"random.door_close", 1.0f, worldObj->rand.nextFloat() * 0.1f + 0.9f);
		break;

	case 1004:
		worldObj->playSoundEffect((float)j + 0.5f, (float)k + 0.5f, (float)l + 0.5f, u"random.fizz", 0.5f, 2.6f + (random.nextFloat() - random.nextFloat()) * 0.8f);
		break;

	case 1005:
		if (Item::itemsList[i1] != nullptr && dynamic_cast<ItemRecord *>(Item::itemsList[i1]) != nullptr)
			worldObj->playRecord(((ItemRecord *)Item::itemsList[i1])->recordName, j, k, l);
		else
			worldObj->playRecord(u"", j, k, l);
		break;
	}
}

void RenderGlobal::clipRenderersByFrustrum(ICamera *icamera, float f)
{
	int_t total = renderChunksWide * renderChunksTall * renderChunksDeep;
	for (int_t i = 0; i < total; i++)
	{
		if (!worldRenderers[i]->skipAllRenderPasses() && (!worldRenderers[i]->isInFrustum || (i + frustrumCheckOffset & 0xf) == 0))
			worldRenderers[i]->updateInFrustrum(icamera);
	}

	frustrumCheckOffset++;
}

bool RenderGlobal::isCloudFog(double x, double y, double z, float partialTicks)
{
	return false;
}

void RenderGlobal::clearWorldRenderers() // func_28137_f
{
	if (worldRenderers != nullptr)
	{
		int_t total = renderChunksWide * renderChunksTall * renderChunksDeep;
		for (int_t i = 0; i < total; i++)
		{
			delete worldRenderers[i];
		}
		delete[] worldRenderers;
		worldRenderers = nullptr;
	}
	delete[] sortedWorldRenderers;
	sortedWorldRenderers = nullptr;
	worldRenderersToUpdate.clear();
	worldRenderersQueuedForUpdate.clear();
	tileEntities.clear();
	glRenderLists.clear();
	renderChunksWide = 0;
	renderChunksTall = 0;
	renderChunksDeep = 0;
	worldRenderersCheckIndex = 0;
}

void RenderGlobal::renderAllRenderLists(int_t pass, double partialTick)
{
	for (int_t i = 0; i < 4; i++)
	{
		if (allRenderLists[i] != nullptr)
			allRenderLists[i]->render();
	}
}
