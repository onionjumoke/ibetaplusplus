# Porting BetaPlusPlus to Other Platforms

BetaPlusPlus is a C++ port of Minecraft Beta 1.7.3. The game logic itself is
platform-neutral C++17; everything that touches the operating system lives in
a thin platform layer under `src/pc/`. This document explains that layer, how
to add a new OS target, and what is special about porting to legacy iOS
(iPhone OS 2.x – iOS 6 era hardware).

---

## 1. Architecture Overview

```
src/
├── java/                  # Portable "Java runtime" shims (File, String,
│   │                      # Random, Resource...). Interfaces only; each OS
│   │                      # provides implementations.
├── net/minecraft/src/     # 100% portable game code (blocks, entities,
│   │                      # rendering via OpenGL 1.x, NBT, packets...)
├── util/                  # Portable helpers (Mth, GLU...)
├── SharedConstants.cpp    # Portable (reads assets/font.txt at static init)
└── pc/                    # THE PLATFORM LAYER — this is all you rewrite
    ├── Main.cpp           # Entry point: SDL init -> GLContext -> Minecraft::run
    ├── client/Minecraft.* # PC-side bootstrap of the game class
    ├── lwjgl/             # LWJGL-style API the game expects:
    │   ├── Display.h      #   window + swap + processMessages
    │   ├── GLContext.h    #   create GL context, load glad
    │   ├── Keyboard.h/.cpp#   key events, key state, key names
    │   └── Mouse.h/.cpp   #   cursor pos, deltas, buttons, wheel, grab
    ├── linux/java/        # File.cpp, Resource.cpp, Runtime.cpp  (POSIX)
    ├── win32/java/        # same interfaces, Win32 implementation
    ├── external/          # stb_vorbis / miniaudio translation units
    ├── CrashHandler.*     # crash reporting
    └── OpenGL.h           # glad include wrapper used by game code
```

**Key fact:** `net/minecraft/src` never calls SDL or POSIX directly. It talks
to `lwjgl::*` (Display/Mouse/Keyboard/GLContext), `Resource::getResource()`,
`java::File`, and OpenGL 1.x fixed-function through glad. If you provide those
symbols on your target OS, the entire game compiles unmodified.

### What the game actually requires from an OS

| Requirement | Where it's needed | Notes |
|---|---|---|
| OpenGL 1.x fixed-function | everywhere | immediate mode (`glBegin/glEnd`), display lists, matrix stack |
| Window + GL context | `pc/lwjgl/{Display,GLContext}` | currently SDL2 |
| Keyboard/mouse input | `pc/lwjgl/{Keyboard,Mouse}` | currently SDL2 events pushed into queues |
| Threads | std::thread | portable already |
| Sockets | SDL_net | TCP client only (multiplayer) |
| zlib | bundled | save files (.mcr region format) |
| Audio decode | miniaudio + stb_vorbis | OGG/music playback; needs an audio device backend |
| File I/O | `pc/<os>/java/File.cpp` | saves, options, assets |
| Assets on disk | `<exe dir>/assets/` | font.txt, terrain.png, gui/, item/, ... |

---

## 2. Adding a New Platform — Checklist

Example: adding a hypothetical `src/pc/fuchsia`.

1. **Create the platform directory**
   ```
   src/pc/fuchsia/java/File.cpp      # implement File_Impl (stat/opendir/etc.)
   src/pc/fuchsia/java/Resource.cpp  # getResource(): open file relative to
                                     # File::openResourceDirectory()
   src/pc/fuchsia/java/Runtime.cpp   # env vars, exit, etc.
   ```

2. **Implement or adapt the lwjgl layer.** The Linux versions are plain SDL2;
   if your OS has SDL2 you can reuse them verbatim. Otherwise reimplement
   these five functions' worth of surface area:
   - `GLContext::instantiate()` — window, context, `gladLoadGLLoader`
   - `Display::{create,swapBuffers,processMessages,update,...}`
   - `Mouse::pushEvent()` / `Keyboard::pushEvent()` — feed your OS input in as
     SDL_Event-shaped data (or refactor to your own event struct)

3. **Entry point.** Copy `src/pc/Main.cpp`; replace `SDL_Init` with your init.
   Everything after that is identical.

4. **CMake.** In `CMakeLists.txt` the source filter is:
   ```cmake
   if(WIN32)   exclude pc/linux
   elseif(UNIX) exclude pc/win32
   else()       FATAL_ERROR
   endif()
   ```
   Add your platform branch, plus any link libraries. For cross-compiling use
   a `CMAKE_TOOLCHAIN_FILE` rather than editing this project.

5. **Audio.** `src/pc/external/miniaudio.cpp` uses miniaudio's built-in
   backends. On exotic targets implement a miniaudio *custom backend* or swap
   in whatever the OS offers (ALSA/Pulse on Linux, XAudio2 on Windows,
   AudioUnit RemoteIO on iOS, SDL audio elsewhere).

6. **Assets.** Ship `assets/` next to the executable (see
   `GameResources::getExeDir()` and `File::openResourceDirectory()`). On
   sandboxed/mobile OSes point both at your app-bundle resource directory.

7. **Build & iterate.** First milestone is reaching the main menu; then world
   creation; then chunk render.

---

## 3. Porting to Legacy iOS (iPhone OS 3 – iOS 6)

Old iOS is one of the harder but very doable targets. The constraints:

### 3.1 Hardware reality check

| Device | SoC | RAM | GPU (GLES version) |
|---|---|---|---|
| iPhone / iPhone 3G | ARM1176JZF-S @ 412–620 MHz (armv6) | 128 MB | PowerVR MBX Lite (GLES 1.1 only) |
| iPhone 3GS / iPod touch 3 | Cortex-A8 @ 600–833 MHz (armv7 + NEON) | 256 MB | SGX535 (GLES 1.1 + 2.0) |
| iPhone 4 / iPod touch 4 | A4 @ 800–1000 MHz (armv7) | 512 MB | SGX535 |
| iPad 1 | A4 | 256 MB | SGX535 |
| iPhone 4S / iPad 2 | A5 (armv7) | 512 MB | SGX543MP2 |

Practical minimum: **iPhone 3GS-class** (armv7). The MBX in armv6 devices has
no GLES2 and weak fill rate; the game runs there only with heavy cuts.
Target **iOS 5.x/6.x** for the best SDK/toolchain overlap; iOS 3.x is possible
but painful (see toolchain notes).

### 3.2 Graphics: the big problem

The renderer is written against **desktop OpenGL 1.x**: immediate mode
(`glBegin/glEnd`, ~6 core files), display lists (`FontRenderer`,
`ModelRenderer`), and the fixed-function matrix stack.

OpenGL ES 1.1 (what old iPhones expose) **removed** `glBegin/glEnd` and
display lists. It kept the fixed-function pipeline (matrix stack, fog, alpha
test, lighting), which is the good news — the shading model ports 1:1. You
have three strategies, in increasing order of fidelity:

**A. Write a GL 1.x → GLES 1.1 compatibility shim (recommended).**
Intercept the desktop-GL entry points the game uses and emulate:
- `glBegin/glVertex*/glEnd` → accumulate into a CPU vertex buffer; on `glEnd`
  issue `glDrawArrays` with the tracked primitive type.
- `glNewList/glCallList` → record the command stream into a CPU buffer and
  replay it on call (display lists here are small; fonts are 256 tiny quads).
- Matrix stack functions exist natively in ES 1.1 (`glOrtho`, `glFrustum`,
  `glTranslatef`, ...) — pass them straight through.
- `glGetFloatv(GL_MODELVIEW_MATRIX)` etc. — available natively.
- Texture upload path: convert `GL_RGB/GL_RGBA` formats as-is; watch for
  non-power-of-two sizes (MBX/SGX require POT without extensions — all MC
  textures are 16x16/64x32/etc., so fine).
- `glPolygonOffset` exists in ES 1.1; `glPushAttrib/glPopAttrib` do not —
  audit every call site (grep for them; they're rare).

Where to put it: a new `src/pc/ios/GLES11Shim.{h,cpp}` providing the missing
symbols, loaded instead of desktop glad. The game's own `Tessellator` batches
most geometry already, so the shim sees few, large draws per frame — fast
enough even interpreted through a shim.

**B. Use ANGLE-style translation to GLES 2.0.** Overkill here; ES 1.1 is
natively fixed-function, no shaders needed.

**C. Rewrite the ~6 immediate-mode call sites into vertex arrays.** Cleanest
long-term (this is exactly what Minecraft PE did), but touches more game code
than A.

### 3.3 Windowing & input: SDL2 vs raw UIKit

SDL 2.0.x supports iOS, but modern SDL releases require recent iOS/SDKs. For
legacy targets:

- **iOS 5/6:** early SDL 2.0 (2.0.0–2.0.3 era) still built against those SDKs,
  but honestly the cleanest path is a **raw UIKit layer** (~500 lines):
  - `EAGLView` with a `kEAGLColorFormatRGB565` drawable + `kEAGLRenderingAPIOpenGLES1`
    context; present via `[context presentRenderbuffer:GL_RENDERBUFFER_OES]`.
  - Map `UITouch` events → `Mouse::pushEvent()`: touchesBegan→button 0 press,
    touchesMoved→motion deltas, two-finger drag → wheel or right-click.
  - `setGrabbed(true)` becomes "pointer-lock mode": hide cursor, treat touch
    deltas as look deltas (exactly like mobile MCPE controls); draw a virtual
    joystick overlay later if desired.
  - Chat/inventory need text: show `UITextField`, forward its output through
    `Keyboard::pushEvent()` as char events.
- **iOS 3/4:** same approach; EAGL exists since iPhone OS 2.0.

The `lwjgl` abstraction makes this swap painless — nothing above it changes.

### 3.4 Audio

miniaudio has no legacy-iOS backend. Two options:
1. Implement a miniaudio **custom backend** using AudioUnit RemoteIO
   (kAudioUnitSubType_RemoteIO) — ~200 lines, works on every iOS version.
2. Bypass miniaudio: route its decoded PCM into your own AudioUnit queue.
Keep the decode side (stb_vorbis) untouched.

### 3.5 Filesystem

- App sandbox: `HOME` is set to the app sandbox root even on old iOS, so
  `File::openWorkingDirectory()` works unchanged for saves (`~/saves` etc.).
- Assets: bundle them in the .app (`<app>/assets/...`) and make
  `File::openResourceDirectory()` return the main bundle path
  (`[[NSBundle mainBundle] bundlePath]`).

### 3.6 Toolchain — the hard part

Modern macOS/Xcode cannot build for armv6/armv7 or deploy below iOS 8ish.
Options:

1. **Legacy Xcode VM (most practical):** Xcode 4.x–5.x with iOS 5.1/6.1 SDKs
   in a macOS VM. Build with `-miphoneos-version-min=5.0`, architectures
   `armv7` (add `armv6` only for pre-3GS devices).
2. **Linux cross-toolchain:** `cctools-port` + ld64 + an extracted iPhoneOS
   SDK, driving clang with `-target armv7-apple-ios5`. Well-documented in the
   "ios-toolchain" community projects. Good for CI.
3. **Theos:** primarily for jailbroken tweaks, but can build apps; easiest
   deploy path on jailbroken devices.

Deployment:
- **App Store route:** effectively impossible today for legacy SDKs (signing
  infrastructure moved on).
- **Jailbroken route:** package as a `.deb`, install to
  `/Applications/YourApp.app`, launch via UIWebView-free SpringBoard icon or
  directly from ssh (`uicache` + respring). This is the standard route for
  legacy-iOS homebrew (same ecosystem as the other projects in this repo).

### 3.7 Performance budgeting (128–256 MB devices)

- Cap render distance at 4–6 chunks (`gameSettings.renderDistance`), far
  plane accordingly.
- The C++ port's baseline footprint (~50–80 MB RSS on Linux x86_64 Release)
  shrinks further with `-Os`; Java Beta 1.7.3 wanted ~512 MB heap, the port
  does not — this is precisely why a native port is feasible at 128 MB.
- Compile `-O2 -fno-exceptions`? No — the port uses exceptions for control
  flow in places; keep them, just avoid `-fwhole-program` surprises.
- Enable NEON (`-mfpu=neon`) on Cortex-A8+; keep VFP for armv6 builds.
- Consider RGB565 framebuffer (halves bandwidth vs RGBA8888) — classic MBX-era
  optimization; the art is authored for 32-bit but dithers acceptably.
- Sound decoding: vorbis decode on a 412 MHz arm1176 is borderline; consider
  pre-decoding music to PCM in the bundle, or lower sample rates.

### 3.8 Suggested milestone order

1. Cross-compile a trivial UIKit app with your chosen toolchain; verify codesign/deploy loop.
2. Bring up `GLES11Shim` against a spinning-triangle test.
3. Wire Display/Mouse/Keyboard over EAGLView; get the Mojang logo screen.
4. Assets loading + main menu (`font.txt`, `logo.png`, buttons).
5. World gen on-device; profile chunk meshing time (the first big perf cliff).
6. Audio; multiplayer last (SDL_net → BSD sockets is nearly free, but test NAT behavior).

---

## 4. Other Platforms (quick notes)

- **Android:** SDL2 officially supports Android ≥ API 19-ish; older Android
  (2.x–4.x) needs old NDK releases. Same shim strategy applies for GLES.
  JNI activity → SDL2 → existing pc/lwjgl works unchanged.
- **WebAssembly/Emscripten:** WebGL 1 ≈ GLES 2.0, so you'd take strategy C
  (rewrite immediate mode) or run a full GL-to-WebGL translation layer.
  Filesystem via Emscripten virtual FS with assets preloaded.
- **macOS:** closest to the Linux build; deprecated-but-present GL works up to
  10.14 for GL 2.1/3.x contexts; fixed-function compat profiles removed in
  newer SDKs — the GLES shim strategy again pays off.
- **Windows XP/9x retro targets:** MinGW with `-D_WIN32_WINNT=0x0500`; the
  win32 platform layer already exists; drop SDL2 for SDL 1.2 with a small
  adapter, or GDI+WGL directly.

---

## 5. Rule of Thumb

Anything inside `src/net/minecraft/src/`, `src/java/`, `src/util/` should
compile untouched on your target. The moment you're tempted to put an
`#ifdef` for your OS into game code, push it down into `src/pc/<your-os>/`
behind the existing interfaces instead. That discipline is what makes the
fourth port cheaper than the second.
