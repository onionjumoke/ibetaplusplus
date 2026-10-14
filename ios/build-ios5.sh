#!/bin/bash
# ─────────────────────────────────────────────────────────────────────────────
# build-ios5.sh — one-shot configure + cross-build + package for iOS 5.
# Follows the BUILD.md automation pattern: a single invocation replaces the
# whole make/bundle/ldid dance so agents burn one tool call, not five.
#
# Usage:
#   bash ios/build-ios5.sh                 # build + package bin/ios/BetaPlusPlus.ipa-style .app zip
#   DEPLOY=1 DEVICE_IP=192.168.1.x bash ios/build-ios5.sh
#                                          # also scp to /Applications and respring
#
# Requirements (iOS5-Guide-Debian.md):
#   ~/ios-toolchain/bin/{ios5-clang,arm-apple-darwin11-*}
#   ~/ios-toolchain/SDKs/iPhoneOS5.0.sdk
#   ldid on PATH for fake-signing (jailbroken deployment)
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT/build-ios"
APP_NAME="BetaPlusPlus.app"
BIN_DIR="$ROOT/bin/ios"
OUT_APP="$BIN_DIR/$APP_NAME"

NINJA=""
command -v ninja >/dev/null 2>&1 && NINJA="-G Ninja"

echo "==> Configuring (MC_IOS=ON, ios5 toolchain)"
cmake -B "$BUILD_DIR" $NINJA \
    -DCMAKE_BUILD_TYPE=Release \
    -DMC_IOS=ON \
    -DCMAKE_CXX_FLAGS_RELEASE="-O2 -g" \
    -DCMAKE_C_FLAGS_RELEASE="-O2 -g" \
    -DCMAKE_TOOLCHAIN_FILE="$ROOT/cmake/ios5.toolchain.cmake"

echo "==> Building"
cmake --build "$BUILD_DIR" -j"$(nproc)"

echo "==> Assembling $APP_NAME"
rm -rf "$OUT_APP"
mkdir -p "$OUT_APP"

# iOS binaries are written under $BUILD_DIR/bin/<config> (see CMakeLists) so
# they never overwrite the host Linux binaries in <root>/bin.
cp "$BUILD_DIR/bin/Release/BetaPlusPlus" "$OUT_APP/BetaPlusPlus" 2>/dev/null ||
    cp "$BUILD_DIR/bin/"*"/BetaPlusPlus" "$OUT_APP/BetaPlusPlus" 2>/dev/null ||
    cp "$BUILD_DIR/BetaPlusPlus" "$OUT_APP/BetaPlusPlus" 2>/dev/null ||
    cp "$ROOT/bin/Release/BetaPlusPlus" "$OUT_APP/BetaPlusPlus" 2>/dev/null ||
    { echo "Binary not found after build"; find "$ROOT" "$BUILD_DIR" -name BetaPlusPlus -type f 2>/dev/null; exit 1; }

cp "$ROOT/ios/Info.plist" "$OUT_APP/Info.plist"

# App icons (Icon.png / @2x / iPad 72 / Small etc.).
for icon in "$ROOT"/ios/Icon*.png; do
    [ -e "$icon" ] && cp "$icon" "$OUT_APP/"
done

printf 'APPL????' > "$OUT_APP/PkgInfo"

# Assets are deliberately NOT bundled: they are copyright Mojang, while this app
# is an independent C++ re-implementation. The user supplies them in the app's
# Documents folder over iTunes File Sharing:
#   Documents/assets      (required)
#   Documents/resources   (optional sound/music)
# The app shows an alert and quits until Documents/assets/font.txt exists.

if command -v ldid >/dev/null 2>&1; then
    # Entitlements match the working jailbreak apps (Codepad2 et al.); without
    # them some legacy platforms apply a stricter seatbelt profile.
    ldid -S"$ROOT/ios/entitlements.plist" "$OUT_APP/BetaPlusPlus"
else
    echo "!! ldid not found: binary is NOT fake-signed. Install ldid for on-device launch."
fi

echo "==> Packaging zip"
( cd "$BIN_DIR" && rm -f BetaPlusPlus.zip && zip -qry BetaPlusPlus.zip "$APP_NAME" )

echo "Done: $OUT_APP"

if [ "${DEPLOY:-0}" = "1" ]; then
    : "${DEVICE_IP:?DEPLOY=1 requires DEVICE_IP=<device address>}"
    SSH_OPTS=(-o HostKeyAlgorithms=+ssh-rsa)
    echo "==> Deploying to root@$DEVICE_IP:/Applications/"
    scp "${SSH_OPTS[@]}" -r "$OUT_APP" "root@$DEVICE_IP:/Applications/"
    ssh "${SSH_OPTS[@]}" "root@$DEVICE_IP" "uicache; killall SpringBoard" || true
    echo "Deployed. Launch from the home screen icon."
fi
