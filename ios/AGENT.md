# BetaPlusPlus — Legacy iOS 5 Port

Minecraft Beta 1.7.3 (C++ port) running natively on **iOS 5.0+ armv7** devices:
iPhone 3GS / 4 / 4S, iPod touch 3 / 4, iPad 1 / 2. Renders through
**OpenGL ES 1.1** via an EAGLView; no SDL, no Xcode required.

```
src/pc/ios/
├── GLES11Shim.{h,cpp}     desktop GL 1.x → ES 1.1 (display lists, glBegin,
│                          glOrtho/glFogi/glColor3f, ARB aliases, queries fail-open)
├── sdlcompat/             mini-SDL over UIKit-era APIs:
│   ├── sdlcompat.h        the API surface game code touches + event model
│   ├── *_core/app/audio/net.cpp  ticks, clipboard, RemoteIO audio, WAV,
│                                  BSD-socket SDLNet, crash alert
│   └── (SDL*.h shims)     header names resolved by include-path precedence —
│                          zero shared headers were modified for this port
├── lwjgl/                 Display/GLContext/Mouse/Keyboard implementations
│                          (shared pc/lwjgl headers used verbatim)
├── EAGLView.{h,mm}        CAEAGLLayer surface (RGB565, 24-bit depth) and the
│                          full touch → mouse/key mapping incl. overlay buttons
├── IOSApp.mm              UIApplication delegate, asset bootstrap into HOME,
│                          text input field, bridge implementation
├── IosBridge.h            C++ ↔ ObjC seam (context ownership, app state)
├── Main_ios.mm            main() → UIApplicationMain; game thread bootstrap
└── java/                  POSIX File/Resource/Runtime (sandbox paths)

external/libcxx-ios/       vendored LLVM 19.1.7 libc++ built as a static
                           runtime (the iOS 5 system libc++.dylib predates
                           modern headers); includes an iOS-5 POSIX backport
                           of std::filesystem's library side.
ios/                       Info.plist + this build script
cmake/ios5.toolchain.cmake cross-toolchain wiring
```

## Building (Debian/Ubuntu host)

Requires the legacy toolchain from `iOS5-Guide-Debian.md`:

```
~/ios-toolchain/bin/{ios5-clang,arm-apple-darwin11-*}
~/ios-toolchain/SDKs/iPhoneOS5.0.sdk
ldid            (fake-signing; linuxbrew/homebrew or apt builds work)
```

One shot:

```bash
bash ios/build-ios5.sh          # configure + build + assemble bin/ios/BetaPlusPlus.app (+ zip)
DEPLOY=1 DEVICE_IP=192.168.1.50 bash ios/build-ios5.sh   # also scp + respring
```

Game assets are bundled automatically if an `assets/` directory exists next to
this file; at first launch the delegate copies them from the read-only bundle
into the app sandbox (`HOME/assets`) so saves (`HOME/.minecraft`) stay writable.

## Deploying (jailbroken device)

The `.app` installs to `/Applications` like any system app:

```bash
scp -o HostKeyAlgorithms=+ssh-rsa -r bin/ios/BetaPlusPlus.app root@<ip>:/Applications/
ssh -o HostKeyAlgorithms=+ssh-rsa root@<ip> "uicache; killall SpringBoard"
```

Launch from the home-screen icon. Minimum OS 5.0 is baked into the binary
(`LC_VERSION_MIN_IPHONEOS 5.0`); the static libc++ means only stock system
dylibs are linked.

## Controls

| Input | Action |
|---|---|
| Tap (menus) | left click at position |
| Left-third drag (in game) | virtual joystick → WASD |
| Right-area drag | camera look |
| Short tap (right area) | use/place (right click) |
| Hold ≥300 ms (right area) | mine/attack (left button held) |
| Second finger vertical drag | hotbar scroll (wheel) |
| Second finger quick tap | attack pulse |
| MENU / INV / JUMP / DBG buttons | ESC / E / SPACE(hold) / F3 |

Text entry (chat, world names): a small UITextField appears top-left when a GUI
asks for text; Return sends. Multiplayer chat works; single-player needs none.

## Design notes & known limitations

- **Sandbox**: iOS only permits writes inside the container's `Documents/`,
  `Library/` and `tmp/`. All mutable state lives under `Library/`:
  seeded assets in `Library/assets`, game data in `Library/.minecraft`,
  debug logs in `tmp/bp.log`. Resource reads fall back to the read-only
  bundle copy during dyld static initializers (pre-main), so first launch
  never touches a forbidden path.
- **ES1 EAGL API**: iOS ≤5's ES 1.1 `EAGLContext` only implements
  `renderbufferStorage:fromDrawable:` — the modern `fromLayer:` variant exists
  solely on ES 2+ contexts and throws `NSInvalidArgumentException` at startup.
  All framebuffer calls use the OES-suffixed ES1 forms accordingly.
- **Assets**: bundle them with `ASSETS_DIR=<path> bash ios/build-ios5.sh`
  (defaults to `<repo>/assets`). The binary is signed with the entitlements
  in `ios/entitlements.plist` (`get-task-allow`, keychain group) matching
  other jailbreak apps on legacy platforms.
- **Debugging real launches**: create an empty file named `bp.debug` inside
  the container's `tmp/` (e.g. over SSH) and relaunch; stderr — including
  assert/terminate messages — is mirrored to `tmp/bp.log`.
- **Headless testing**: SSH-launched UIKit apps never reach
  `didFinishLaunching` (no backboardd attachment); with `bp.debug` armed the
  game thread still autostarts once the view exists. Full verification needs
  a genuine SpringBoard launch.
- **Renderer**: the shim intercepts the ~30 desktop-GL symbols the port uses;
  everything else passes straight to ES 1.1. Display lists record a compact
  command stream and deep-copy client vertex arrays (the Tessellator reuses its
  buffer immediately after closing a list). Occlusion queries fail open —
  chunk culling relies on frustum + distance only, exactly like a driver
  without GL_ARB_occlusion_query on PC.
- **RGB565 framebuffer + contentScaleFactor 1.0**: halves fill bandwidth on
  SGX535-class GPUs; change both in `EAGLView.mm` if you have an iPhone 4S and
  want sharper output.
- **No Auto-Rotation** (pinned landscape) keeps framebuffer ownership simple.
- **miniaudio is excluded**; SoundManager's SDL-audio mixer runs on a RemoteIO
  unit instead. Music/sounds must exist in `HOME/resources` — the original
  download source (s3.amazonaws.com/MinecraftResources) has been dead for
  years, so copy sound packs over scp.
- **std::filesystem**: upstream libc++ operations need `openat()` (iOS 8+);
  `ios_operations_backport.cpp` reimplements the library side with plain POSIX.
- **Futures are stubbed** (`future_stub.cpp`) — nothing in the port uses them,
  but thread.cpp references one symbol from that object.
- Screenshots land in `HOME/.minecraft/screenshots`.

### Perf budgeting quick reference

Render distance 4–6 chunks for 256 MB devices (3GS/iPad 1); the F3 overlay
shows fps + chunk updates. If audio stutters, pre-convert OGG→WAV 44.1k.
