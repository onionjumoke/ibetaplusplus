#include "GuiParticle.h"
#include "Particle.h"
#include "RenderEngine.h"
#include "Minecraft.h"
#include "pc/OpenGL.h"

GuiParticle::GuiParticle(Minecraft *minecraft)
	: mc(minecraft)
{
}

void GuiParticle::updateParticles()
{
	for (int_t i = 0; i < (int_t)particles.size(); i++)
	{
		Particle *p = particles[i];
		p->updateParticle();
		p->renderIntoGui(this);
		if (p->isDead)
		{
			particles.erase(particles.begin() + i);
			i--;
		}
	}
}

void GuiParticle::updateAndRender(float_t partialTick)
{
	updateParticles();
	renderParticles(partialTick);
}

void GuiParticle::renderParticles(float_t partialTick)
{
	mc->renderEngine->bindTexture(mc->renderEngine->getTexture("/gui/particles.png"));
	for (int_t i = 0; i < (int_t)particles.size(); i++)
	{
		Particle *p = particles[i];
		int_t x = (int_t)((p->prevPosX + (p->posX - p->prevPosX) * (double)partialTick) - 4.0);
		int_t y = (int_t)((p->prevPosY + (p->posY - p->prevPosY) * (double)partialTick) - 4.0);
		float_t alpha  = (float_t)(p->prevAlpha  + (p->alpha  - p->prevAlpha)  * (double)partialTick);
		float_t red    = (float_t)(p->prevRed    + (p->red    - p->prevRed)    * (double)partialTick);
		float_t green  = (float_t)(p->prevGreen  + (p->green  - p->prevGreen)  * (double)partialTick);
		float_t blue   = (float_t)(p->prevBlue   + (p->blue   - p->prevBlue)   * (double)partialTick);
		glColor4f(red, green, blue, alpha);
		drawTexturedModalRect(x, y, 40, 0, 8, 8);
	}
}
