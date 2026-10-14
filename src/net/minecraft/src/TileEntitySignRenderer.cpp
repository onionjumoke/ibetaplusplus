#include "TileEntitySignRenderer.h"

#include "pc/OpenGL.h"
#include "TileEntitySign.h"
#include "SignModel.h"
#include "Block.h"
#include "FontRenderer.h"
#include "ModelRenderer.h"
#include "java/System.h"

TileEntitySignRenderer::TileEntitySignRenderer()
    : signModel(new SignModel())
{
}

void TileEntitySignRenderer::renderTileEntitySignAt(TileEntitySign* tileentitysign, double d, double d1, double d2, float f)
{
    Block* block = tileentitysign->getBlockType();
    long_t signStartNs = System::nanoTime();
    glPushMatrix();
    float f1 = 0.6666667f;
    if (block == Block::signPost)
    {
        glTranslatef((float)d + 0.5f, (float)d1 + 0.75f * f1, (float)d2 + 0.5f);
        float f2 = (float)(tileentitysign->getBlockMetadata() * 360) / 16.0f;
        glRotatef(-f2, 0.0f, 1.0f, 0.0f);
        signModel->signStick->showModel = true;
    }
    else
    {
        int i = tileentitysign->getBlockMetadata();
        float f3 = 0.0f;
        if (i == 2) f3 = 180.0f;
        if (i == 4) f3 = 90.0f;
        if (i == 5) f3 = -90.0f;
        glTranslatef((float)d + 0.5f, (float)d1 + 0.75f * f1, (float)d2 + 0.5f);
        glRotatef(-f3, 0.0f, 1.0f, 0.0f);
        glTranslatef(0.0f, -0.3125f, -0.4375f);
        signModel->signStick->showModel = false;
    }
    bindTextureByName("/item/sign.png");
    glPushMatrix();
    glScalef(f1, -f1, -f1);
    signModel->renderSign();
    glPopMatrix();
    long_t tModel = System::nanoTime();
    FontRenderer* fontrenderer = getFontRenderer();
    float f4 = 0.01666667f * f1;
    glTranslatef(0.0f, 0.5f * f1, 0.07f * f1);
    glScalef(f4, -f4, f4);
    glNormal3f(0.0f, 0.0f, -1.0f * f4);
    glDepthMask(false);
    for (int k = 0; k < 4; k++)
    {
        std::string s = tileentitysign->signText[k];
        if (k == tileentitysign->lineBeingEdited)
            s = "> " + s + " <";
        fontrenderer->drawString(s, -fontrenderer->getStringWidth(s) / 2, k * 10 - 20, 0);
    }
    glDepthMask(true);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glPopMatrix();

    {
        long_t tText = System::nanoTime();
        static long_t s_modelNs = 0, s_textNs = 0, s_lastLog = 0, s_count = 0;
        s_modelNs += (tModel - signStartNs);
        s_textNs  += (tText - tModel);
        s_count++;
        long_t now = System::currentTimeMillis();
        if (now - s_lastLog >= 2000)
        {
            s_lastLog = now;
            (void)0;
            s_modelNs = s_textNs = s_count = 0;
        }
    }
}

void TileEntitySignRenderer::renderTileEntityAt(TileEntity* tileentity, double d, double d1, double d2, float f)
{
    renderTileEntitySignAt((TileEntitySign*)tileentity, d, d1, d2, f);
}
