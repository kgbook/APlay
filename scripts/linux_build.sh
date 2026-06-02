#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${APLAY_LINUX_BUILD_DIR:-${ROOT_DIR}/build/linux}"
GENERATOR="${CMAKE_GENERATOR:-Ninja}"

cmake -S "${ROOT_DIR}/app/linux" -B "${BUILD_DIR}" -G "${GENERATOR}"
cmake --build "${BUILD_DIR}" --target aplay

echo "Linux executable: ${BUILD_DIR}/aplay"
