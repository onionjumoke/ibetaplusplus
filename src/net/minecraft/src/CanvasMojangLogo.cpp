#include "CanvasMojangLogo.h"

#include <iostream>
#include <fstream>

#include "pc/OpenGL.h"
#include "java/BufferedImage.h"
#include "GameResources.h"

CanvasMojangLogo::CanvasMojangLogo()
    : logoTextureId(-1), logoWidth(0), logoHeight(0)
{
    try
    {
        // Java: ImageIO.read(PanelCrashReport.class.getResource("/gui/logo.png"))
        std::ifstream file(GameResources::resolve("/gui/logo.png"), std::ios::binary);
        if (file.is_open())
        {
            BufferedImage img = BufferedImage::ImageIO_read(file);
            logoWidth  = img.getWidth();
            logoHeight = img.getHeight();

            if (logoWidth > 0 && logoHeight > 0)
            {
                // Upload to OpenGL
                glGenTextures(1, (GLuint *)&logoTextureId);
                glBindTexture(GL_TEXTURE_2D, logoTextureId);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                             logoWidth, logoHeight, 0,
                             GL_RGBA, GL_UNSIGNED_BYTE,
                             img.getRawPixels());
            }
        }
    }
    catch (...)
    {
        // Logo not essential; continue without it
    }
}

void CanvasMojangLogo::render(int_t containerWidth)
{
    if (logoTextureId < 0 || logoWidth == 0)
        return;

    int_t x = containerWidth / 2 - logoWidth / 2;
    int_t y = 32;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, (GLuint)logoTextureId);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2i(x,              y);
        glTexCoord2f(0.0f, 1.0f); glVertex2i(x,              y + logoHeight);
        glTexCoord2f(1.0f, 1.0f); glVertex2i(x + logoWidth,  y + logoHeight);
        glTexCoord2f(1.0f, 0.0f); glVertex2i(x + logoWidth,  y);
    glEnd();
}
