#!/usr/bin/env sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD_DIR="${ROOT_DIR}/build/linux"
PCAP_FIXTURE="${ROOT_DIR}/resources/pcap/Airplay_Discovery_and_Connection.pcapng"

echo "=== Syncing and updating submodules ==="
git -C "${ROOT_DIR}" submodule sync --recursive
git -C "${ROOT_DIR}" submodule update --init --recursive

cmake -S "${ROOT_DIR}/app/linux" -B "${BUILD_DIR}" -G Ninja \
    -DAPLAY_BUILD_LINUX=ON \
    -DAPLAY_BUILD_EXAMPLES=ON
cmake --build "${BUILD_DIR}" --target \
    APlayReceiver \
    aplay_sdk \
    aplay_example_mdns_replay \
    aplay_example_mdns_announce

"${BUILD_DIR}/example/aplay_example_mdns_replay" "${PCAP_FIXTURE}"
