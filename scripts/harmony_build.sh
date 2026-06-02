#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
HARMONY_DIR="${ROOT_DIR}/app/harmony"

if [[ -z "${DEVECO_SDK_HOME:-}" || ! -d "${DEVECO_SDK_HOME}/default" ]]; then
    echo "Invalid or missing DEVECO_SDK_HOME: ${DEVECO_SDK_HOME:-<unset>}" >&2
    echo "Set DEVECO_SDK_HOME to the DevEco SDK root directory, and ensure Harmony tools are in PATH." >&2
    exit 1
fi

if ! command -v hvigorw >/dev/null 2>&1; then
    echo "hvigorw not found in PATH. Configure the Harmony toolchain path, then retry." >&2
    exit 127
fi

if ! command -v ohpm >/dev/null 2>&1; then
    echo "ohpm not found in PATH. Configure the Harmony toolchain path, then retry." >&2
    exit 127
fi

pushd "${HARMONY_DIR}" > /dev/null
ohpm install
pushd APlayReceiver > /dev/null
ohpm install
popd > /dev/null
hvigorw assembleHar assembleHap --no-daemon
popd > /dev/null

echo "Harmony HAR output: ${ROOT_DIR}/sdk/build/default/outputs/default"
echo "Harmony HAP output: ${HARMONY_DIR}/APlayReceiver/build/default/outputs/default"
