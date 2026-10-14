#include "RenderEntity.h"
#include "Entity.h"
#include "AxisAlignedBB.h"
#include <SDL2/SDL_opengl.h>

RenderEntity::RenderEntity() {
}

void RenderEntity::doRender(Entity* entity, double d, double d1, double d2, float f, float f1) {
    glPushMatrix();
    renderOffsetAABB(entity->boundingBox, d - entity->lastTickPosX, d1 - entity->lastTickPosY, d2 - entity->lastTickPosZ);
    glPopMatrix();
}