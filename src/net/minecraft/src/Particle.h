#pragma once

#include <random>
#include "java/Type.h"

class GuiParticle;

// net.minecraft.src.Particle  (GUI particle for title screen / menu animations)
class Particle
{
public:
    Particle();

    void updateParticle();                   // func_25125_a – tick; saves prev state then advances
    void renderIntoGui(GuiParticle *gui);    // per-particle render hook (no-op; rendering done in GuiParticle::renderParticles)
    void kill();                             // func_25126_b

    double posX;      // field_25146_a
    double posY;      // field_25145_b
    double prevPosX;  // field_25144_c
    double prevPosY;  // field_25143_d
    double velX;      // field_25142_e
    double velY;      // field_25141_f
    double friction;  // field_25140_g
    bool   isDead;    // field_25139_h
    int_t  age;       // field_25138_i
    int_t  lifetime;  // field_25137_j
    double red;       // field_25136_k
    double green;     // field_25135_l
    double blue;      // field_25134_m
    double alpha;     // field_25133_n
    double prevRed;   // field_25132_o
    double prevGreen; // field_25131_p
    double prevBlue;  // field_25130_q
    double prevAlpha; // field_25129_r

private:
    static std::mt19937 rng;
};
