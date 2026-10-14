#include "RenderSnowball.h"
#include "Tessellator.h"
#include "pc/OpenGL.h"

RenderSnowball::RenderSnowball(int_t iconIndex) : itemIconIndex(iconIndex) {}

void RenderSnowball::doRender(Entity*, double x, double y, double z, float, float)
{
	glPushMatrix();
	glTranslatef((float)x, (float)y, (float)z);
	glEnable(GL_RESCALE_NORMAL);
	glScalef(0.5f, 0.5f, 0.5f);
	loadTexture("/gui/items.png");
	float u0 = (float)((itemIconIndex % 16) * 16) / 256.0f;
	float u1 = (float)((itemIconIndex % 16) * 16 + 16) / 256.0f;
	float v0 = (float)((itemIconIndex / 16) * 16) / 256.0f;
	float v1 = (float)((itemIconIndex / 16) * 16 + 16) / 256.0f;
	glRotatef(180.0f - renderManager->playerViewY, 0.0f, 1.0f, 0.0f);
	glRotatef(-renderManager->playerViewX, 1.0f, 0.0f, 0.0f);
	Tessellator *tessellator = &Tessellator::instance;
	tessellator->startDrawingQuads();
	tessellator->setNormal(0.0f, 1.0f, 0.0f);
	tessellator->addVertexWithUV(-0.5f, -0.25f, 0.0, u0, v1);
	tessellator->addVertexWithUV( 0.5f, -0.25f, 0.0, u1, v1);
	tessellator->addVertexWithUV( 0.5f,  0.75f, 0.0, u1, v0);
	tessellator->addVertexWithUV(-0.5f,  0.75f, 0.0, u0, v0);
	tessellator->draw();
	glDisable(GL_RESCALE_NORMAL);
	glPopMatrix();
}
