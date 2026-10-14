// iOS build: renderer files include <SDL2/SDL_opengl.h> for desktop-GL names.
// Resolve to the GLES 1.1 compatibility shim.
#include "../../GLES11Shim.h"
