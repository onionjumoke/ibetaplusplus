#include "TileEntityRendererPiston.h"

#include "pc/OpenGL.h"
#include "TileEntityPiston.h"
#include "World.h"
#include "Block.h"
#include "BlockPistonBase.h"
#include "BlockPistonExtension.h"
#include "RenderBlocks.h"
#include "RenderHelper.h"
#include "Tessellator.h"
#include "pc/client/Minecraft.h"

TileEntityRendererPiston::TileEntityRendererPiston()
    : renderBlocks(nullptr)
{
}

void TileEntityRendererPiston::func_31069_a(World* world)
{
    renderBlocks = new RenderBlocks(world);
}

void TileEntityRendererPiston::renderPistonAt(TileEntityPiston* tileentitypiston, double d, double d1, double d2, float f)
{
    Block* block = Block::blocksList[tileentitypiston->getStoredBlockID()];
    if (block == nullptr || tileentitypiston->getProgress(f) >= 1.0f)
        return;

    Tessellator& tessellator = Tessellator::instance;
    bindTextureByName("/terrain.png");
    RenderHelper::disableStandardItemLighting();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    if (Minecraft::isAmbientOcclusionEnabled())
        glShadeModel(GL_SMOOTH);
    else
        glShadeModel(GL_FLAT);

    tessellator.startDrawingQuads();
    tessellator.setTranslationD(
        ((float)d  - (float)tileentitypiston->xCoord) + tileentitypiston->getOffsetX(f),
        ((float)d1 - (float)tileentitypiston->yCoord) + tileentitypiston->getOffsetY(f),
        ((float)d2 - (float)tileentitypiston->zCoord) + tileentitypiston->getOffsetZ(f));
    tessellator.setColorOpaque(1, 1, 1);

    if (block == Block::pistonExtension && tileentitypiston->getProgress(f) < 0.5f)
    {
        renderBlocks->renderPistonExtensionAllFaces(block, tileentitypiston->xCoord, tileentitypiston->yCoord, tileentitypiston->zCoord, false);
    }
    else if (tileentitypiston->isHead() && !tileentitypiston->isExtending())
    {
        Block::pistonExtension->getPistonExtensionState(((BlockPistonBase*)block)->getPistonHeadTexture());
        renderBlocks->renderPistonExtensionAllFaces(Block::pistonExtension, tileentitypiston->xCoord, tileentitypiston->yCoord, tileentitypiston->zCoord, tileentitypiston->getProgress(f) < 0.5f);
        Block::pistonExtension->getPistonExtensionFacing();
        tessellator.setTranslationD(
            (float)d  - (float)tileentitypiston->xCoord,
            (float)d1 - (float)tileentitypiston->yCoord,
            (float)d2 - (float)tileentitypiston->zCoord);
        renderBlocks->renderPistonBaseAllFaces(block, tileentitypiston->xCoord, tileentitypiston->yCoord, tileentitypiston->zCoord);
    }
    else
    {
        renderBlocks->renderBlockAllFaces(block, tileentitypiston->xCoord, tileentitypiston->yCoord, tileentitypiston->zCoord);
    }

    tessellator.setTranslationD(0.0, 0.0, 0.0);
    tessellator.draw();
    RenderHelper::enableStandardItemLighting();
}

void TileEntityRendererPiston::renderTileEntityAt(TileEntity* tileentity, double d, double d1, double d2, float f)
{
    renderPistonAt((TileEntityPiston*)tileentity, d, d1, d2, f);
}
