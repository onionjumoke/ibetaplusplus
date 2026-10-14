![Titlecard](images-for-readme/titlecard.PNG)

# iBetaPlusPlus

iBetaPlusPlus is a modified version of BetaPlusPlus (by [OptiJuegos](https://github.com/OptiJuegos/)), which is a heavily modified C++ implementation of classic Minecraft-era gameplay (Beta 1.7.3) designed around portability, low-end hardware, and platform-specific optimization.

if you want to check out [OptiJuegos](https://github.com/OptiJuegos/)' BetaPlusPlus, it's now called [OptiCraftHeritageEdition](https://github.com/OptiJuegos/OptiCraftHeritageEdition)

This repository is not intended to be a line-for-line source translation. The runtime, platform layers, rendering paths, input backends, storage systems, user interface, asset loading, and memory policies have been extensively reworked for the needs of this project.

## Features
![controls](images-for-readme/multiplayer.PNG)
### Mobile controls
- Virtual joystick + on-screen buttons: MENU, INV, JUMP, CROUCH, CHAT, KB, DBG, CHNKS
- Hotbar tap-to-select, hold-to-mine / tap-to-place
- MCPE-style touch crafting screen — category tabs, Craftable filter, tap-to-craft
- Native iOS keyboard for usernames, world names, seeds, and chat
![qol](images-for-readme/flat.PNG)
### Extra content & QoL
- World types: Superflat (always Forest, tiered ores — coal/iron common, gold rare, redstone rarer, diamonds only in veins of 4+ — plus a starter tree and a small pond) and a flat Nether (lava, glowstone, soul sand; zombie pigmen only)
- F3 debug screen, F3+G chunk borders (plus a mobile button), with vertical borders
- FOV slider; Mobs / Particles / Cave Sounds toggles; Unstuck button
- Texture packs (pick up from the Documents folder)
- iTunes File Sharing for assets and world saves
- Custom app icon + "not affiliated with Mojang" disclaimer
![visuals](images-for-readme/house.PNG)
### Performance / visuals
- GLES 1.1 shim with op-stream repacking and a software-authoritative matrix path for PowerVR SGX
- Batched font rendering (sign text went ~1000 ms → ~45 ms)
- Time-budgeted chunk renderer + streaming
- Big graphics options list: render distance, fog, clouds, trees, grass, animated water/lava/fire/portal/redstone, smooth lighting, better grass, stars/sky, anaglyph, framerate limit, AO, GUI scale, brightness, more

## Project goals

- Keep the implementation portable across legacy iOS (32-bit armv7), desktop PC, and a headless terminal renderer.
- Preserve the intended classic gameplay and visual behavior where practical while allowing platform-specific adaptations.
- Run on constrained hardware through aggressive memory, rendering, chunk, and asset-loading optimizations.
- Keep platform code isolated behind explicit backends instead of scattering host-specific logic through the game code.
- Maintain a debuggable C++17 codebase.

## Legal and disclaimers

iBetaPlusPlus is an unofficial fan project: a C++ port of classic Minecraft-era (Beta 1.7.3) gameplay, provided for personal, educational, and preservation use.

This project is **not affiliated with, endorsed by, sponsored by, approved by, or associated with** Mojang Studios or Microsoft. "Minecraft", "Mojang", and all related names, logos, and assets are trademarks and/or copyright of Mojang Studios / Microsoft and are used here only descriptively to identify the game whose behavior this project reproduces.

iBetaPlusPlus ships **no Minecraft assets**. Textures, sounds, fonts, language files, and all other game data are copyright Mojang Studios, are **not** included in this repository, and are **not** included in any binary this project produces. The software does not and cannot run without a user-supplied copy of those assets: on iOS, copy an `assets` folder into the app's Documents directory over iTunes File Sharing; on desktop, place an `assets` directory next to the executable. You are solely responsible for obtaining those assets lawfully and for complying with Mojang's End User License Agreement and Usage Guidelines.

This software is provided "as is", without warranty of any kind, express or implied, and the authors accept no liability for its use. See `LICENSE` for the full terms. Nothing in this document grants any rights to Mojang's intellectual property.

The grass block texture shown in the iOS icon is owned by Mojang, not by me.

## Supported targets

### iOS (legacy)

The iOS build targets 32-bit armv7 devices on iOS 5–6 (e.g. iPhone 4S, iPad 4) using OpenGL ES 1.1 with a software-authoritative matrix path for PowerVR SGX hardware. User-supplied assets and world saves live in the app's Documents folder (iTunes File Sharing).

### Desktop PC

The desktop build uses SDL2 and OpenGL with the shared platform abstraction layer.

### Terminal

A headless terminal renderer (kitty graphics protocol, with an ANSI half-block fallback) is available for environments without a GPU.

## Source layout

```text
src/
  client/       Client-side shared code
  java/         Java compatibility/runtime helpers
  net/          Game implementation
  platform/     Shared platform interfaces and backend selection
  pc/           Desktop implementation
  pc/ios/       Legacy iOS implementation
  pc/terminal/  Terminal renderer
  util/         Shared utility code

cmake/          Toolchains, source selection, and platform build logic
external/       Third-party dependencies
ios/            iOS packaging, Info.plist, and the IPA builder
```

## Setting up the assets and resources folders

iBetaPlusPlus ships no game data. Before it will start, it needs two folders:

```text
assets/       textures, fonts, language files, GUI, particles, etc.
resources/    sound + music pools
```

Both are extracted from the official Minecraft **Beta 1.7.3** client jar
(`b1.7.3.jar`). The split is simply "sounds vs. everything else":

1. Obtain `b1.7.3.jar` (Mojang's Piston service serves it; the official
   launcher downloads the same file).
2. Unzip it into a temporary folder.
3. Move the sound/music directories into `resources/`:
   `sound/`, `music/`, `newsound/`, `newmusic/`, `streaming/`, `records/`.
4. Move everything else (ignore `.class` files and `META-INF/`) into
   `assets/`: `achievement/`, `armor/`, `art/`, `environment/`, `font/`,
   `font.txt`, `gui/`, `item/`, `lang/`, `misc/`, `mob/`, `pack.png`,
   `pack.txt`, `particles.png`, `terrain/`, `terrain.png`, `title/`, …

`make-debug-ipa.sh` performs steps 1–4 for you and can pack the result straight
into a test IPA.

### Where the folders go

- **iOS** - copy **both** folders into the app's **Documents** directory over
  iTunes File Sharing (`device → Apps → iBetaPlusPlus → Documents`), then relaunch.
- **Desktop** - place both folders next to the executable (the same directory
  as `BetaPlusPlus`).

If either folder is missing, the app shows a "You're missing the files!" alert
and closes.

## Building

### Desktop

```text
cmake -B build-gui -DCMAKE_BUILD_TYPE=Release
cmake --build build-gui
```

### iOS

```text
bash ios/build-ios5.sh
```

To produce a debug IPA with the assets bundled inside the app (testing only):

```text
bash make-debug-ipa.sh
```

## Development notes

iBetaPlusPlus contains substantial platform-specific changes compared with the behavior it reproduces. Examples include custom render backends, legacy UI work, low-memory chunk policies, touch input layers, asset loading, profiling, and platform-specific performance tuning.

When changing shared systems, keep the platform abstraction boundary intact and avoid introducing PC-only assumptions into common code.

## Known bugs
- game sometimes crashes for no reason
- sometimes there's an "end of stream" error when trying to connect in multiplayer (goes away when killing and reopening the app)
- some blocks can visually corrupt, doesnt affect gameplay

## Third-party software

Third-party libraries are kept under `external/` and retain their respective licenses and notices. Review those licenses independently before redistributing binaries.
