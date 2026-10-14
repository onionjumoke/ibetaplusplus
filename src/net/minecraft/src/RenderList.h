#ifndef RENDERLIST_H
#define RENDERLIST_H

#include <vector>
#include "java/Type.h"

// net.minecraft.src.RenderList
class RenderList {
public:
    RenderList();

    void setup(int i, int j, int k, double d, double d1, double d2);
    bool matchesPos(int i, int j, int k);
    void addGLRenderList(int i);
    void render();
    void reset();

private:
    int field_1242_a;
    int field_1241_b;
    int field_1240_c;
    float field_1239_d;
    float field_1238_e;
    float field_1237_f;
    std::vector<int_t> field_1236_g;
    bool field_1235_h;
    bool field_1234_i;
};

#endif