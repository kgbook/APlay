#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
HARMONY_DIR="${ROOT_DIR}/app/harmony"

HARMONY_COMMAND_LINE_TOOLS="${HARMONY_COMMAND_LINE_TOOLS:-${HOME}/tools/command-line-tools}"
if [[ ! -d "${HARMONY_COMMAND_LINE_TOOLS}" ]]; then
    echo "Invalid HARMONY_COMMAND_LINE_TOOLS: ${HARMONY_COMMAND_LINE_TOOLS}" >&2
    echo "Set HARMONY_COMMAND_LINE_TOOLS to the DevEco command-line-tools directory." >&2
    exit 1
fi

HARMONY_TOOLCHAINS_DIR="${HARMONY_TOOLCHAINS_DIR:-${HARMONY_COMMAND_LINE_TOOLS}/sdk/default/openharmony/toolchains}"
export HARMONY_SDK_HOME="${HARMONY_SDK_HOME:-${HARMONY_COMMAND_LINE_TOOLS}/sdk}"
if [[ -z "${NODE_HOME:-}" ]]; then
    if NODE_BIN="$(command -v node 2>/dev/null)"; then
        NODE_HOME="$(cd "$(dirname "${NODE_BIN}")/.." && pwd)"
    else
        NODE_HOME="${HARMONY_COMMAND_LINE_TOOLS}/tool/node"
    fi
fi
export PATH="${HARMONY_COMMAND_LINE_TOOLS}/bin:${HARMONY_COMMAND_LINE_TOOLS}/ohpm/bin:${HARMONY_COMMAND_LINE_TOOLS}/hvigor/bin:${HARMONY_TOOLCHAINS_DIR}:${NODE_HOME}/bin:${PATH}"

if [[ -z "${HARMONY_SDK_HOME:-}" || ! -d "${HARMONY_SDK_HOME}/default" ]]; then
    echo "Invalid or missing HARMONY_SDK_HOME: ${HARMONY_SDK_HOME:-<unset>}" >&2
    echo "Expected a DevEco SDK root with a default SDK under ${HARMONY_SDK_HOME}/default." >&2
    exit 1
fi

if ! HVIGOR_BIN="$(command -v hvigorw 2>/dev/null)"; then
    echo "hvigorw not found in PATH. Configure HARMONY_COMMAND_LINE_TOOLS, then retry." >&2
    exit 127
fi

if ! OHPM_BIN="$(command -v ohpm 2>/dev/null)"; then
    echo "ohpm not found in PATH. Configure HARMONY_COMMAND_LINE_TOOLS, then retry." >&2
    exit 127
fi

pushd "${HARMONY_DIR}" > /dev/null
"${OHPM_BIN}" install
pushd APlayReceiver > /dev/null
"${OHPM_BIN}" install
popd > /dev/null
"${HVIGOR_BIN}" assembleHar assembleHap --no-daemon
popd > /dev/null

echo "Harmony HAR output: ${ROOT_DIR}/sdk/build/default/outputs/default"
echo "Harmony HAP output: ${HARMONY_DIR}/APlayReceiver/build/default/outputs/default"
