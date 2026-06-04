#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

echo "=== Syncing and updating submodules ==="
git -C "${ROOT_DIR}" submodule sync --recursive
git -C "${ROOT_DIR}" submodule update --init --recursive

HARMONY_DIR="${ROOT_DIR}/app/harmony"

if [[ -z "${HARMONY_COMMAND_LINE_TOOLS:-}" ]]; then
    if [[ "$(uname -s)" == "Darwin" ]]; then # macOS DevEcho Studio
        for DEVECO_CONTENTS_DIR in \
            "/Applications/DevEco-Studio.app/Contents" \
            "/Applications/DevEco\ Studio.app/Contents"
        do
            if [[ -d "${DEVECO_CONTENTS_DIR}" ]]; then
                HARMONY_COMMAND_LINE_TOOLS="${DEVECO_CONTENTS_DIR}"
                break
            fi
        done
    fi
    HARMONY_COMMAND_LINE_TOOLS="${HARMONY_COMMAND_LINE_TOOLS:-${HOME}/tools/command-line-tools}"
fi

if [[ ! -d "${HARMONY_COMMAND_LINE_TOOLS}" ]]; then
    echo "Invalid HARMONY_COMMAND_LINE_TOOLS: ${HARMONY_COMMAND_LINE_TOOLS}" >&2
    echo "Set HARMONY_COMMAND_LINE_TOOLS to the DevEco Studio Contents directory or command-line-tools directory." >&2
    exit 1
fi

HARMONY_DEFAULT_SDK_HOME="${HARMONY_COMMAND_LINE_TOOLS}/sdk"
if [[ -d "${HARMONY_COMMAND_LINE_TOOLS}/tools" ]]; then # macOS DevEcho Studio
    HARMONY_TOOLS_BIN="${HARMONY_COMMAND_LINE_TOOLS}/bin"
    HARMONY_OHPM_BIN_DIR="${HARMONY_COMMAND_LINE_TOOLS}/tools/ohpm/bin"
    HARMONY_HVIGOR_BIN_DIR="${HARMONY_COMMAND_LINE_TOOLS}/tools/hvigor/bin"
    HARMONY_NODE_HOME="${HARMONY_COMMAND_LINE_TOOLS}/tools/node"
else # Linux command-line-tools
    HARMONY_TOOLS_BIN="${HARMONY_COMMAND_LINE_TOOLS}/bin"
    HARMONY_OHPM_BIN_DIR="${HARMONY_COMMAND_LINE_TOOLS}/ohpm/bin"
    HARMONY_HVIGOR_BIN_DIR="${HARMONY_COMMAND_LINE_TOOLS}/hvigor/bin"
    HARMONY_NODE_HOME="${HARMONY_COMMAND_LINE_TOOLS}/tool/node"
fi

export HARMONY_SDK_HOME="${HARMONY_SDK_HOME:-${DEVECO_SDK_HOME:-${HARMONY_DEFAULT_SDK_HOME}}}"
export DEVECO_SDK_HOME="${DEVECO_SDK_HOME:-${HARMONY_SDK_HOME}}"
HARMONY_TOOLCHAINS_DIR="${HARMONY_TOOLCHAINS_DIR:-${HARMONY_SDK_HOME}/default/openharmony/toolchains}"
if [[ -z "${NODE_HOME:-}" ]]; then
    if NODE_BIN="$(command -v node 2>/dev/null)"; then
        NODE_HOME="$(cd "$(dirname "${NODE_BIN}")/.." && pwd)"
    else
        NODE_HOME="${HARMONY_NODE_HOME}"
    fi
fi

HARMONY_PATH_ENTRIES=(
    "${HARMONY_TOOLS_BIN}"
    "${HARMONY_OHPM_BIN_DIR}"
    "${HARMONY_HVIGOR_BIN_DIR}"
    "${HARMONY_TOOLCHAINS_DIR}"
    "${NODE_HOME}/bin"
)
HARMONY_PATH_PREFIX="$(IFS=:; echo "${HARMONY_PATH_ENTRIES[*]}")"
export PATH="${HARMONY_PATH_PREFIX}:${PATH}"

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
