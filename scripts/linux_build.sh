#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

echo "=== Syncing and updating submodules ==="
git -C "${ROOT_DIR}" submodule sync --recursive
git -C "${ROOT_DIR}" submodule update --init --remote --recursive

BUILD_DIR="${APLAY_LINUX_BUILD_DIR:-${ROOT_DIR}/build/linux}"
GENERATOR="${CMAKE_GENERATOR:-Ninja}"

rm -rf "${BUILD_DIR}"
cmake \
    -S "${ROOT_DIR}/app/linux" \
    -B "${BUILD_DIR}" \
    -G "${GENERATOR}" \
    -DAPLAY_BUILD_HARNESS=ON \
    -DAPLAY_BUILD_LINUX=ON \
    -DAPLAY_BUILD_ANDROID=OFF \
    -DAPLAY_BUILD_HARMONY=OFF
cmake --build "${BUILD_DIR}"

SDK_LIBRARY="$(find "${BUILD_DIR}/sdk-cpp" -maxdepth 1 -type f \( -name 'libAPlaySdk.so' -o -name 'libAPlaySdk.dylib' \) -print -quit)"
echo "Linux SDK library: ${SDK_LIBRARY}"
echo "Linux executable: ${BUILD_DIR}/APlayReceiver"
