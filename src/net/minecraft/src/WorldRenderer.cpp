#include "WorldRenderer.h"

#include "pc/OpenGL.h"
#include "World.h"
#include "ICamera.h"
#include "Entity.h"
#include "MathHelper.h"
#include "java/System.h"
#include "Block.h"
#include "RenderBlocks.h"
#include "RenderItem.h"
#include "Tessellator.h"
#include "Chunk.h"
#include "ChunkCache.h"
#include "TileEntity.h"
#include "TileEntityRenderer.h"
#include "AxisAlignedBB.h"

#include <algorithm>
#include <unordered_set>
#include <atomic>
#include <cstdio>

int_t WorldRenderer::chunksUpdated = 0;

WorldRenderer::WorldRenderer(World *world, std::vector<TileEntity *> *tileEntitiesIn, int_t posX, int_t posY, int_t posZ, int_t size, int_t glListId)
{
	worldObj      = world;
	tileEntities  = tileEntitiesIn;
	sizeWidth = sizeHeight = sizeDepth = size;
	rendererRadius = MathHelper::sqrt_float((float)(sizeWidth * sizeWidth + sizeHeight * sizeHeight + sizeDepth * sizeDepth)) / 2.0f;
	glRenderList   = glListId;
	needsUpdate    = false;
	isChunkLit     = false;
	isWaitingOnOcclusionQuery = false;
	isVisible      = true;
	isInFrustum    = false;
	chunkIndex     = 0;
	glOcclusionQuery = 0;
	isInitialized  = false;
	_skipRenderPass[0] = false;
	_skipRenderPass[1] = false;
	rendererBoundingBox = nullptr;

	this->posX = -999;
	setPosition(posX, posY, posZ);
	needsUpdate = false;
}

WorldRenderer::~WorldRenderer()
{
	cleanup();
}

void WorldRenderer::cleanup()
{
	// Display lists and occlusion queries come from ranges owned by
	// RenderGlobal. Deleting individual entries frees names that are reused by
	// new WorldRenderers and may then collide with model or sky display lists.
	setDontDraw();
	worldObj = nullptr;
	glRenderList = 0;
	glOcclusionQuery = 0;
	delete rendererBoundingBox;
	rendererBoundingBox = nullptr;
}

void WorldRenderer::setPosition(int_t x, int_t y, int_t z)
{
	if (x == posX && y == posY && z == posZ)
		return;

	setDontDraw();
	posX = x;
	posY = y;
	posZ = z;
	posXPlus = x + sizeWidth  / 2;
	posYPlus = y + sizeHeight / 2;
	posZPlus = z + sizeDepth  / 2;
	posXClip = x & 0x3ff;
	posYClip = y;
	posZClip = z & 0x3ff;
	posXMinus = x - posXClip;
	posYMinus = y - posYClip;
	posZMinus = z - posZClip;

	float f = 6.0f;
	// setPosition runs for all renderers on every grid shift; the previous
	// box was leaked every time (new AxisAlignedBB, never freed) -> OOM/jetsam.
	delete rendererBoundingBox;
	rendererBoundingBox = AxisAlignedBB::getBoundingBox(
		(float)x - f, (float)y - f, (float)z - f,
		(float)(x + sizeWidth) + f, (float)(y + sizeHeight) + f, (float)(z + sizeDepth) + f);

	// Build occlusion box display list (list index +2)
	glNewList(glRenderList + 2, GL_COMPILE);
	RenderItem::renderAABB(AxisAlignedBB::getBoundingBoxFromPool(
		(float)posXClip - f, (float)posYClip - f, (float)posZClip - f,
		(float)(posXClip + sizeWidth) + f, (float)(posYClip + sizeHeight) + f, (float)(posZClip + sizeDepth) + f));
	glEndList();

	markDirty();
}

void WorldRenderer::updateInFrustrum(ICamera *icamera)
{
	isInFrustum = icamera->isBoundingBoxInFrustum(rendererBoundingBox);
}

bool WorldRenderer::updateRenderer()
{
	{
		// DIAG: dump the first renderers that reach the build, and bail out
		// (instead of crashing) if the tile-entity vector looks corrupt.
		static std::atomic<int> dbgUR{0};
		int d = dbgUR.fetch_add(1);
		size_t n = tileEntityRenderers.size();
		size_t cap = tileEntityRenderers.capacity();
		if (d < 48 || n > 4096 || cap > 4096 || worldObj == nullptr || glRenderList <= 0)
			(void)0;
		if (n > 4096 || cap > 4096 || worldObj == nullptr || glRenderList <= 0)
			return true;
	}

	if (!needsUpdate)
		return true;

	{
		int cx = posX >> 4;
		int cz = posZ >> 4;
		Chunk *c = worldObj->getChunkFromChunkCoords(cx, cz);
		if (c == nullptr || !c->hasRealData)
		{
			{
				static long_t deferLast = 0;
				long_t deferNow = System::currentTimeMillis();
				if (deferNow - deferLast > 4000)
				{
					deferLast = deferNow;
					(void)0;
				}
			}
			return false;
		}
	}
	chunksUpdated++;

	int_t x0 = posX,             y0 = posY,              z0 = posZ;
	int_t x1 = posX + sizeWidth, y1 = posY + sizeHeight, z1 = posZ + sizeDepth;

	for (int_t k1 = 0; k1 < 2; k1++)
		_skipRenderPass[k1] = true;

	Chunk::isLit = false;

	// TE bookkeeping disabled (crashes on this iOS5/libc++ build). Rebuild
	// the global list elsewhere later; chunks must render first.
	tileEntityRenderers.clear();

	int_t margin = 1;
	ChunkCache *chunkcachePtr = new ChunkCache(worldObj, x0 - margin, y0 - margin, z0 - margin,
	                                x1 + margin, y1 + margin, z1 + margin);
	ChunkCache &chunkcache = *chunkcachePtr;
	RenderBlocks renderblocks(&chunkcache);

	Tessellator *tessellator = &Tessellator::instance;

	for (int_t pass = 0; pass < 2; )
	{
		bool hasOtherPass = false;
		bool drewAnything = false;
		bool listOpen = false;

		for (int_t y = y0; y < y1; y++)
		{
			for (int_t z = z0; z < z1; z++)
			{
				for (int_t x = x0; x < x1; x++)
				{
					int_t id = chunkcache.getBlockId(x, y, z);
					if (id <= 0)
						continue;

					if (!listOpen)
					{
						listOpen = true;
						glNewList(glRenderList + pass, GL_COMPILE);
						glPushMatrix();
						// Translate to the clip-space origin of this chunk
						glTranslatef((float)posXClip, (float)posYClip, (float)posZClip);
						// Slight scale to avoid z-fighting on chunk borders
						float f = 1.000001f;
						glTranslatef(-(float)sizeDepth / 2.0f, -(float)sizeHeight / 2.0f, -(float)sizeDepth / 2.0f);
						glScalef(f, f, f);
						glTranslatef( (float)sizeDepth / 2.0f,  (float)sizeHeight / 2.0f,  (float)sizeDepth / 2.0f);
						tessellator->startDrawingQuads();
						tessellator->setTranslationD(-(double)posX, -(double)posY, -(double)posZ);
					}


					Block *block = Block::blocksList[id];
					if (block == nullptr)
						continue;
					int_t blockPass = block->getRenderBlockPass();

					if (blockPass != pass)
					{
						hasOtherPass = true;
						continue;
					}

					drewAnything |= renderblocks.renderBlockByRenderType(block, x, y, z);
				}
			}
		}

		if (listOpen)
		{
			tessellator->draw();
			glPopMatrix();
			glEndList();
			tessellator->setTranslationD(0.0, 0.0, 0.0);
		}
		else
		{
			drewAnything = false;
		}

		if (drewAnything)
			_skipRenderPass[pass] = false;

		if (!hasOtherPass)
			break;

		pass++;
	}


	{
		static std::atomic<int> dbgE{0};
		if (dbgE.fetch_add(1) < 12) (void)0;
	}
	isChunkLit  = Chunk::isLit;
	isInitialized = true;
	needsUpdate = false;
	{
		static std::atomic<int> dbgD{0};
		if (dbgD.fetch_add(1) < 12) (void)0;
	}
	delete chunkcachePtr;
	{
		static std::atomic<int> dbgD2{0};
		if (dbgD2.fetch_add(1) < 12) (void)0;
	}
	{
		static std::atomic<int> dbgZ{0};
		if (dbgZ.fetch_add(1) < 12) (void)0;
	}
	return true;
}

void WorldRenderer::markDirty()
{
	needsUpdate = true;
}

void WorldRenderer::setDontDraw()
{
	_skipRenderPass[0] = true;
	_skipRenderPass[1] = true;
	isInFrustum   = false;
	isInitialized = false;
}

void WorldRenderer::func_1204_c()
{
	setDontDraw();
	worldObj = nullptr;
}

void WorldRenderer::callOcclusionQueryList()
{
	glCallList(glRenderList + 2);
}

int_t WorldRenderer::getGLCallListForPass(int_t pass)
{
	// TEST: bypass the frustum gate (isInFrustum). If chunks appear, the
	// frustum built from the shim's matrix mirror is the culprit.
	if (!_skipRenderPass[pass])
		return glRenderList + pass;
	return -1;
}

bool WorldRenderer::skipAllRenderPasses()
{
	if (!isInitialized)
		return false;
	return _skipRenderPass[0] && _skipRenderPass[1];
}

bool WorldRenderer::skipRenderPass(int_t pass)
{
	if (pass < 0 || pass > 1)
		return true;
	return _skipRenderPass[pass];
}

float WorldRenderer::distanceToEntitySquared(Entity *entity)
{
	if (entity == nullptr)
		return 0.0f;
	float dx = (float)(entity->posX - (double)posXPlus);
	float dy = (float)(entity->posY - (double)posYPlus);
	float dz = (float)(entity->posZ - (double)posZPlus);
	return dx * dx + dy * dy + dz * dz;
}
