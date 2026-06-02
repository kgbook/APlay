#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${APLAY_LINUX_BUILD_DIR:-${ROOT_DIR}/build/linux}"
GENERATOR="${CMAKE_GENERATOR:-Ninja}"

rm -rf BUILD_DIR
cmake \
    -S "${ROOT_DIR}/app/linux" \
    -B "${BUILD_DIR}" \
    -G "${GENERATOR}" \
    -DAPLAY_BUILD_HARNESS=OFF \
    -DAPLAY_BUILD_LINUX=ON \
    -DAPLAY_BUILD_ANDROID=OFF \
    -DAPLAY_BUILD_HARMONY=OFF
cmake --build "${BUILD_DIR}" --target aplay

echo "Linux executable: ${BUILD_DIR}/aplay"
