#include "RenderList.h"
#include "GLAllocation.h"
#include <SDL2/SDL_opengl.h>

RenderList::RenderList() {
    field_1236_g = GLAllocation::createDirectIntBuffer(0x10000);
    field_1235_h = false;
    field_1234_i = false;
}

void RenderList::setup(int i, int j, int k, double d, double d1, double d2) {
    field_1235_h = true;
    field_1236_g.clear();
    field_1242_a = i;
    field_1241_b = j;
    field_1240_c = k;
    field_1239_d = (float)d;
    field_1238_e = (float)d1;
    field_1237_f = (float)d2;
}

bool RenderList::matchesPos(int i, int j, int k) {
    if (!field_1235_h) {
        return false;
    } else {
        return i == field_1242_a && j == field_1241_b && k == field_1240_c;
    }
}

void RenderList::addGLRenderList(int i) {
    field_1236_g.push_back(i);
    if (field_1236_g.size() >= field_1236_g.capacity()) {
        render();
    }
}

void RenderList::render() {
    if (!field_1235_h) {
        return;
    }
    if (!field_1234_i) {
        field_1234_i = true;
    }
    if (field_1236_g.size() > 0) {
        glPushMatrix();
        glTranslatef((float)field_1242_a - field_1239_d, (float)field_1241_b - field_1238_e, (float)field_1240_c - field_1237_f);
        glCallLists((GLsizei)field_1236_g.size(), GL_INT, field_1236_g.data());
        glPopMatrix();
    }
}

void RenderList::reset() {
    field_1235_h = false;
    field_1234_i = false;
    field_1236_g.clear();
}