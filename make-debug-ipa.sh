#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# make-debug-ipa.sh — interactive builder for a self-contained *debug* IPA that
# has the Minecraft Beta 1.7.3 assets bundled inside the .app.
#
# The shipping app contains NO Minecraft assets (only the game binary is our own
# C++ code; the assets are copyright Mojang). A normal build shows an "assets
# required" alert and quits until assets are dropped into the app's Documents
# folder over iTunes File Sharing. This script is for DEBUGGING ONLY: it packs
# the assets into the app bundle so the app can seed them into Documents itself.
#
# You pick:
#   * with the app: an existing .ipa to modify, a fresh build from the iOS
#     .app bundle, or download a base .ipa from a URL and modify that;
#   * where the assets come from: Mojang's Piston service (b1.7.3) or local
#     assets/ + resources/ folders.
#
# Non-interactive use (any of these skip the matching prompt):
#   TARGET_MODE=existing|build|download  TARGET_IPA=path  BASE_IPA_URL=url
#   ASSET_SOURCE=piston|local            ASSETS_DIR=...    RESOURCES_DIR=...
#   APP_BUNDLE=path  OUT_DIR=dir  OUT_NAME=iBPP.ipa  VERSION=b1.7.3
#   NONINTERACTIVE=1
#
# Requires: curl, unzip, zip, python3.
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VERSION="${VERSION:-b1.7.3}"
OUT_DIR="${OUT_DIR:-$ROOT}"
OUT_NAME="${OUT_NAME:-iBPP.ipa}"
APP_BUNDLE="${APP_BUNDLE:-$ROOT/bin/ios/BetaPlusPlus.app}"

MANIFESTS=(
    "https://piston-meta.mojang.com/mc/game/version_manifest_v2.json"
    "https://piston-meta.mojang.com/mc/game/version_manifest.json"
    "https://launchermeta.mojang.com/mc/game/version_manifest.json"
)
SOUND_DIRS="sound music newsound newmusic streaming records"

die() { echo "error: $*" >&2; exit 1; }
need() { command -v "$1" >/dev/null 2>&1 || die "missing required tool: $1"; }

INTERACTIVE=1
[ -t 0 ] || INTERACTIVE=0
[ "${NONINTERACTIVE:-0}" = "1" ] && INTERACTIVE=0

# ── prompting ────────────────────────────────────────────────────────────────
ask() { # ask <prompt> <default> -> stdout
    local prompt="$1" def="$2" reply=""
    if [ "$INTERACTIVE" = "1" ]; then
        read -r -p "$prompt [$def]: " reply || true
    fi
    printf '%s' "${reply:-$def}"
}

ask_required() { # ask <prompt> [default] -> stdout, loops until non-empty
    local prompt="$1" def="${2:-}" reply
    while :; do
        reply="$(ask "$prompt" "$def")"
        [ -n "$reply" ] && { printf '%s' "$reply"; return; }
        echo "  (please enter a value)" >&2
    done
}

menu() { # menu <header> <option1> <option2> ... -> prints chosen index (1-based)
    local header="$1"; shift
    echo "$header" >&2
    local i=1
    for opt in "$@"; do echo "  $i) $opt" >&2; i=$((i + 1)); done
    local n=$((i - 1))
    local choice
    while :; do
        choice="$(ask "Choose" "1")"
        if [[ "$choice" =~ ^[0-9]+$ ]] && [ "$choice" -ge 1 ] && [ "$choice" -le "$n" ]; then
            printf '%s' "$choice"; return
        fi
        echo "  enter a number 1..$n" >&2
    done
}

banner() {
    cat >&2 <<EOF

╔══════════════════════════════════════════════════════════════╗
║  iB++ debug IPA builder — bundles Minecraft $VERSION assets  ║
╚══════════════════════════════════════════════════════════════╝
EOF
}

# ── asset acquisition ────────────────────────────────────────────────────────
obtain_assets_piston() { # sets A, R
    need curl; need python3
    echo "==> Locating $VERSION in the Piston version manifest" >&2
    local manifest=""
    for m in "${MANIFESTS[@]}"; do
        if curl -fSL --retry 3 --max-time 60 -o "$WORK/manifest.json" "$m"; then
            manifest="$WORK/manifest.json"; echo "    via $m" >&2; break
        fi
    done
    [ -n "$manifest" ] || die "could not reach any Piston manifest"

    local version_url
    version_url="$(MANIFEST="$manifest" VERSION="$VERSION" python3 - <<'PY'
import json, os
data = json.load(open(os.environ["MANIFEST"]))
for e in data.get("versions", []):
    if e.get("id") == os.environ["VERSION"]:
        print(e.get("url", "")); break
PY
)"
    [ -n "$version_url" ] || die "$VERSION not found in the version manifest"
    curl -fSL --retry 3 --max-time 60 -o "$WORK/version.json" "$version_url"

    local jar_url
    jar_url="$(python3 - <<PY
import json
print(json.load(open("$WORK/version.json"))["downloads"]["client"]["url"])
PY
)"
    [ -n "$jar_url" ] || die "no client download for $VERSION"
    echo "==> Downloading client jar: $jar_url" >&2
    curl -fSL --retry 3 --max-time 600 -o "$WORK/$VERSION.jar" "$jar_url"

    echo "==> Extracting assets" >&2
    mkdir -p "$WORK/jar"
    ( cd "$WORK/jar" && unzip -oq "$WORK/$VERSION.jar" )
    ( cd "$WORK/jar"
      for entry in *; do
          [ -e "$entry" ] || continue
          case "$entry" in META-INF|*.class|readme.txt) continue ;; esac
          is_sound=0
          for s in $SOUND_DIRS; do [ "$entry" = "$s" ] && is_sound=1; done
          if [ "$is_sound" = 1 ]; then cp -R "$entry" "$R/"; else cp -R "$entry" "$A/"; fi
      done
    )
}

obtain_assets_local() { # sets A, R
    local adir rdir
    adir="$(ask_required "Path to the assets folder" "${ASSETS_DIR:-}")"
    [ -d "$adir" ] || die "not a directory: $adir"
    cp -R "$adir/." "$A/"
    rdir="$(ask "Path to the resources folder (blank to skip)" "${RESOURCES_DIR:-}")"
    if [ -n "$rdir" ]; then
        [ -d "$rdir" ] || die "not a directory: $rdir"
        cp -R "$rdir/." "$R/"
    fi
}

# ── IPA assembly ─────────────────────────────────────────────────────────────
inject_and_pack() { # inject_and_pack <unpacked_root_containing_Payload>
    local stage="$1"
    local app
    app="$(find "$stage/Payload" -maxdepth 1 -name '*.app' -type d | head -n1)"
    [ -n "$app" ] || die "no .app found inside the IPA Payload"
    echo "==> Injecting assets into $(basename "$app")" >&2
    rm -rf "$app/assets" "$app/resources"
    cp -R "$A" "$app/assets"
    if [ -n "$(ls -A "$R" 2>/dev/null)" ]; then cp -R "$R" "$app/resources"; fi

    mkdir -p "$OUT_DIR"
    local out="$OUT_DIR/$OUT_NAME"
    rm -f "$out"
    ( cd "$stage" && zip -qry "$out" Payload )
    echo "Done: $out" >&2
}

unpack_ipa() { # unpack_ipa <ipa> <dest>
    local ipa="$1" dest="$2"
    [ -f "$ipa" ] || die "no such IPA: $ipa"
    mkdir -p "$dest"
    unzip -oq "$ipa" -d "$dest"
}

# ── main ─────────────────────────────────────────────────────────────────────
need unzip; need zip
banner

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
WORK="$TMP/work"
A="$WORK/assets"; R="$WORK/resources"
mkdir -p "$A" "$R"

# Step 1: target IPA
TARGET_MODE="${TARGET_MODE:-}"
if [ -z "$TARGET_MODE" ]; then
    case "$(menu "Step 1 — which IPA do you want to produce?" \
        "Modify an existing .ipa on disk" \
        "Build a fresh IPA from the iOS app bundle ($APP_BUNDLE)" \
        "Download a base .ipa from a URL, then modify it")" in
        1) TARGET_MODE=existing ;;
        2) TARGET_MODE=build ;;
        3) TARGET_MODE=download ;;
    esac
fi

# Step 2: asset source
ASSET_SOURCE="${ASSET_SOURCE:-}"
if [ -z "$ASSET_SOURCE" ]; then
    case "$(menu "Step 2 — where do the assets come from?" \
        "Download Minecraft $VERSION from Mojang Piston" \
        "Use local assets/ + resources/ folders")" in
        1) ASSET_SOURCE=piston ;;
        2) ASSET_SOURCE=local ;;
    esac
fi

OUT_NAME="$(ask "Output IPA filename" "$OUT_NAME")"
case "$OUT_NAME" in *.ipa) ;; *) OUT_NAME="$OUT_NAME.ipa" ;; esac

if [ "$ASSET_SOURCE" = "piston" ]; then obtain_assets_piston; else obtain_assets_local; fi
[ -e "$A/font.txt" ] || die "assets/font.txt missing — unexpected asset layout"
[ -e "$A/terrain.png" ] || echo "warning: assets/terrain.png missing" >&2

case "$TARGET_MODE" in
    existing)
        target="${TARGET_IPA:-}"
        if [ -z "$target" ]; then
            # Offer the .ipa files we can see; otherwise ask for a path.
            mapfile -t ipas < <(find "$PWD" "$ROOT" -maxdepth 1 -name '*.ipa' -type f 2>/dev/null | sort -u)
            if [ "${#ipas[@]}" -gt 0 ]; then
                opts=("${ipas[@]}" "Enter a path…")
                sel="$(menu "Choose the .ipa to modify" "${opts[@]}")"
                if [ "$sel" -le "${#ipas[@]}" ]; then target="${ipas[$((sel - 1))]}"
                else target="$(ask_required "Path to .ipa")"; fi
            else
                target="$(ask_required "Path to .ipa to modify")"
            fi
        fi
        unpack_ipa "$target" "$TMP/stage"
        inject_and_pack "$TMP/stage"
        ;;
    build)
        app="${APP_BUNDLE:-}"
        [ -n "$app" ] || app="$(ask_required "Path to the .app bundle" "$APP_BUNDLE")"
        [ -d "$app" ] || die "app bundle not found: $app (build iOS first)"
        mkdir -p "$TMP/stage/Payload"
        cp -R "$app" "$TMP/stage/Payload/"
        inject_and_pack "$TMP/stage"
        ;;
    download)
        url="${BASE_IPA_URL:-}"
        [ -n "$url" ] || url="$(ask_required "URL of the base .ipa to download")"
        out_ipa="$TMP/base.ipa"
        echo "==> Downloading $url" >&2
        curl -fSL --retry 3 --max-time 600 -o "$out_ipa" "$url"
        unpack_ipa "$out_ipa" "$TMP/stage"
        inject_and_pack "$TMP/stage"
        ;;
    *)
        die "unknown TARGET_MODE: $TARGET_MODE"
        ;;
esac
