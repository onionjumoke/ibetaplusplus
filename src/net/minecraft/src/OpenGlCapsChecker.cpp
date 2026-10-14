#include "OpenGlCapsChecker.h"

#include "pc/lwjgl/GLContext.h"

bool OpenGlCapsChecker::tryCheckOcclusionCapable = true;

OpenGlCapsChecker::OpenGlCapsChecker()
{
}

bool OpenGlCapsChecker::checkARBOcclusion()
{
	return tryCheckOcclusionCapable && lwjgl::GLContext::getCapabilities()["GL_ARB_occlusion_query"];
}
