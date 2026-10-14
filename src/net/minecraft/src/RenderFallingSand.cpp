#include "RenderFallingSand.h"
#include "EntityFallingSand.h"
#include "Block.h"
#include "World.h"
#include "MathHelper.h"
#include <SDL2/SDL_opengl.h>

RenderFallingSand::RenderFallingSand() {
    field_197_d = new RenderBlocks();
    shadowSize = 0.5f;
}

void RenderFallingSand::doRenderFallingSand(EntityFallingSand* entityfallingsand, double d, double d1, double d2, float f, float f1) {
    glPushMatrix();
    glTranslatef((float)d, (float)d1, (float)d2);
    loadTexture("/terrain.png");
    Block* block = Block::blocksList[entityfallingsand->blockID];
    World* world = entityfallingsand->worldObj;
    glDisable(GL_LIGHTING);
    field_197_d->renderBlockFallingSand(block, world, MathHelper::floor_double(entityfallingsand->posX), MathHelper::floor_double(entityfallingsand->posY), MathHelper::floor_double(entityfallingsand->posZ));
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

void RenderFallingSand::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    doRenderFallingSand((EntityFallingSand*)entity, d, d1, d2, f, f1);
}