// ─────────────────────────────────────────────────────────────────────────────
// Profiling.h — coarse phase accumulators for remote performance triage.
// Accummulate milliseconds via SCOPED_PROFILE(name) or Profiling::add().
// Minecraft::run() drains and prints the averages every N frames (stderr,
// only visible when tmp/bp.debug arms the log redirect).
// ─────────────────────────────────────────────────────────────────────────────

#pragma once

#include <cstdint>

namespace Profiling
{

enum Slot
{
	S_TICK_WORLD = 0,   // World::tick()
	S_TICK_ENTITIES,    // entity/tileentity updates inside World::tick
	S_TICK_SPAWNER,     // SpawnerAnimals::performSpawning
	S_TICK_UNLOAD,      // chunk unload pass
	S_TICK_CONTROLLER,  // playerController->updateController
	S_TICK_TEXFX,       // renderEngine->updateDynamicTextures
	S_TICK_RANDOMDISP,  // randomDisplayUpdates
	S_TICK_EFFECTS,     // effectRenderer->updateEffects
	S_TICK_LIGHT,       // TickUpdates (lighting queue)
	S_TICK_RANDBLOCK,   // updateBlocksAndPlayCaveSounds
	S_TICK_STAT,        // statFileWriter tick
	S_TICK_INGAME,      // ingameGUI updateTick
	S_TICK_MOUSEOVER,   // getMouseOver ray trace
	S_REND_CLIP,        // clipRenderersByFrustrum
	S_REND_UPDATE,      // RenderGlobal::updateRenderers (chunk builds)
	S_REND_SORTDRAW,    // sortAndRender / draw passes
	S_REND_ENTITIES,    // entity + particle rendering
	S_REND_GUI,         // GUI screens / ingame overlay
	S_REND_PRE,         // ScaledResolution + SDL_GetMouseState
	S_REND_MOUSE,       // mouse delta handling
	S_REND_WORLD,       // whole renderWorld call incl fps-limit loop
	S_COUNT
};

void add(int slot, int64_t nanos);
void resetAll();
double takeAvgMs(int slot, int *countOut);
void advanceFrame();
int currentCount();

struct Auto
{
	int slot;
	int64_t t0;
	explicit Auto(int s) : slot(s), t0(now()) {}
	~Auto() { add(slot, now() - t0); }
	static int64_t now();
};

} // namespace Profiling

#define SCOPED_PROFILE(slot) ::Profiling::Auto _prof_auto_##__LINE__(slot)
