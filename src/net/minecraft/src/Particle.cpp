#include "Particle.h"

#include "GuiParticle.h"

std::mt19937 Particle::rng;

Particle::Particle() :
    posX(0.0), posY(0.0),
    prevPosX(0.0), prevPosY(0.0),
    velX(0.0), velY(0.0),
    friction(1.0),
    isDead(false),
    age(0),
    lifetime(0),
    red(0.0), green(0.0), blue(0.0), alpha(0.0),
    prevRed(0.0), prevGreen(0.0), prevBlue(0.0), prevAlpha(0.0)
{
}

void Particle::updateParticle()
{
    prevRed   = red;
    prevGreen = green;
    prevBlue  = blue;
    prevAlpha = alpha;
    prevPosX  = posX;
    prevPosY  = posY;

    posX += velX;
    posY += velY;
    velX *= friction;
    velY *= friction;
    velY += 0.1;

    if (++age > lifetime)
        kill();

    alpha = 2.0 - ((double)age / (double)lifetime) * 2.0;
    if (alpha > 1.0)
        alpha = 1.0;
    alpha = alpha * alpha;
    alpha *= 0.5;
}

void Particle::renderIntoGui(GuiParticle *)
{
}

void Particle::kill()
{
    isDead = true;
}
