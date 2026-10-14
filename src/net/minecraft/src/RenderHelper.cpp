#include "RenderHelper.h"
#include "Vec3D.h"
#include <SDL2/SDL_opengl.h>

std::vector<float> RenderHelper::field_1695_a(16);

RenderHelper::RenderHelper() {
}

void RenderHelper::disableStandardItemLighting() {
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_COLOR_MATERIAL);
}

void RenderHelper::enableStandardItemLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    float f = 0.4f;
    float f1 = 0.6f;
    float f2 = 0.0f;
    Vec3D* vec3d = Vec3D::createVector(0.2, 1.0, -0.7);
    vec3d = vec3d->normalize();
    std::vector<float> light0Pos = createFloatBufferFromDoubles(vec3d->xCoord, vec3d->yCoord, vec3d->zCoord, 0.0);
    glLightfv(GL_LIGHT0, GL_POSITION, light0Pos.data());
    std::vector<float> light0Diffuse = createFloatBuffer(f1, f1, f1, 1.0f);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Diffuse.data());
    std::vector<float> light0Ambient = createFloatBuffer(0.0f, 0.0f, 0.0f, 1.0f);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0Ambient.data());
    std::vector<float> light0Specular = createFloatBuffer(f2, f2, f2, 1.0f);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0Specular.data());

    vec3d = Vec3D::createVector(-0.2, 1.0, 0.7);
    vec3d = vec3d->normalize();
    std::vector<float> light1Pos = createFloatBufferFromDoubles(vec3d->xCoord, vec3d->yCoord, vec3d->zCoord, 0.0);
    glLightfv(GL_LIGHT1, GL_POSITION, light1Pos.data());
    std::vector<float> light1Diffuse = createFloatBuffer(f1, f1, f1, 1.0f);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light1Diffuse.data());
    std::vector<float> light1Ambient = createFloatBuffer(0.0f, 0.0f, 0.0f, 1.0f);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light1Ambient.data());
    std::vector<float> light1Specular = createFloatBuffer(f2, f2, f2, 1.0f);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light1Specular.data());

    glShadeModel(GL_FLAT);
    std::vector<float> lightModelAmbient = createFloatBuffer(f, f, f, 1.0f);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightModelAmbient.data());
}

std::vector<float> RenderHelper::createFloatBufferFromDoubles(double d, double d1, double d2, double d3) {
    return createFloatBuffer((float)d, (float)d1, (float)d2, (float)d3);
}

std::vector<float> RenderHelper::createFloatBuffer(float f, float f1, float f2, float f3) {
    std::vector<float> buffer(4);
    buffer[0] = f;
    buffer[1] = f1;
    buffer[2] = f2;
    buffer[3] = f3;
    return buffer;
}
