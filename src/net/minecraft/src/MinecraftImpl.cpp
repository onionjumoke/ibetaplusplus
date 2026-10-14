#include "MinecraftImpl.h"

#include <iostream>

#include "PanelCrashReport.h"
#include "UnexpectedThrowable.h"

MinecraftImpl::MinecraftImpl(void *component, void *canvas, void *minecraftapplet,
                             int_t width, int_t height, bool fullscreen, void *frame)
    : Minecraft(width, height, fullscreen),
      mcFrame(frame)
{
}

void MinecraftImpl::displayUnexpectedThrowable(UnexpectedThrowable *unexpectedthrowable)
{
    PanelCrashReport report(unexpectedthrowable);
    std::cerr << report.getText() << std::endl;
    // In SDL2 the crash text is written to stderr;
    // the frame reference (mcFrame) would receive the panel in the AWT build.
}
