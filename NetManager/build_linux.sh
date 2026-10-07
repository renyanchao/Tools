#!/usr/bin/env sh
set -u

PROJECT_ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
VCPKG_PATH="$PROJECT_ROOT/../../vcpkg"
BUILD_PRESET="linux-debug"
VCPKG_TRIPLET="x64-linux"

pause_exit() {
    printf "Press Enter to continue..."
    read _unused
}

fail() {
    echo "[NetManager] Build failed." >&2
    pause_exit
    exit 1
}

echo "[NetManager] Project root: $PROJECT_ROOT"
echo "[NetManager] vcpkg path: $VCPKG_PATH"
echo "[NetManager] Build preset: $BUILD_PRESET"
echo "[NetManager] vcpkg triplet: $VCPKG_TRIPLET"

if [ ! -d "$VCPKG_PATH" ]; then
    echo "[NetManager] vcpkg was not found. Cloning vcpkg..."
    git clone https://github.com/microsoft/vcpkg "$VCPKG_PATH" || fail
else
    echo "[NetManager] vcpkg already exists. Skipping clone."
fi

VCPKG_TOOLCHAIN="$VCPKG_PATH/scripts/buildsystems/vcpkg.cmake"
if [ ! -x "$VCPKG_PATH/vcpkg" ]; then
    echo "[NetManager] Bootstrapping vcpkg..."
    "$VCPKG_PATH/bootstrap-vcpkg.sh" || fail
else
    echo "[NetManager] vcpkg toolchain already exists. Skipping bootstrap."
fi

if [ ! -f "$VCPKG_TOOLCHAIN" ]; then
    echo "[NetManager] vcpkg toolchain file not found: $VCPKG_TOOLCHAIN" >&2
    fail
fi

cd "$PROJECT_ROOT" || fail

echo "[NetManager] Configuring CMake..."
cmake --preset "$BUILD_PRESET" || fail

echo "[NetManager] Building CMake target..."
cmake --build --preset "$BUILD_PRESET" || fail

echo "[NetManager] Build finished."
pause_exit
