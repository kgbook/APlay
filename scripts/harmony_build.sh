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

if command -v hvigorw >/dev/null 2>&1; then
    HVIGOR=(hvigorw)
elif command -v hvigor >/dev/null 2>&1; then
    HVIGOR=(hvigor)
else
    echo "Unable to find hvigorw or hvigor in PATH. Configure the Harmony toolchain path, then retry." >&2
    exit 127
fi

if command -v ohpm >/dev/null 2>&1; then
    OHPM=(ohpm)
else
    echo "Unable to find ohpm in PATH. Configure the Harmony toolchain path, then retry." >&2
    exit 127
fi

(
    cd "${HARMONY_DIR}"
    "${OHPM[@]}" install
    (
        cd entry
        "${OHPM[@]}" install
    )
    "${HVIGOR[@]}" assembleHap --no-daemon
)

echo "Harmony HAP output: ${HARMONY_DIR}/entry/build/default/outputs/default"
