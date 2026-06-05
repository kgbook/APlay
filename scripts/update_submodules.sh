#!/usr/bin/env sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT_DIR=${1:-$(CDPATH= cd -- "${SCRIPT_DIR}/.." && pwd)}

echo "=== Syncing submodule URLs ==="
git -C "${ROOT_DIR}" submodule sync --recursive

SUBMODULE_STATUS=$(git -C "${ROOT_DIR}" submodule status --recursive)
if printf '%s\n' "${SUBMODULE_STATUS}" | grep -q '^-'; then
    echo "=== Initializing and updating submodules from remote ==="
    git -C "${ROOT_DIR}" submodule update --init --remote --recursive
else
    echo "=== Updating submodules from remote ==="
    git -C "${ROOT_DIR}" submodule update --remote --recursive
fi
