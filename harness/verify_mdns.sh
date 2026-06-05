#!/usr/bin/env sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD_DIR="${ROOT_DIR}/build/linux"
PCAP_CAPTURE="${APLAY_PCAP_CAPTURE:-${ROOT_DIR}/resources/pcap/mdns_announce.pcapng}"
RESULT_DIR="${APLAY_MDNS_RESULT_DIR:-${ROOT_DIR}/resources/pcap}"
RESULT_FILE="${APLAY_MDNS_RESULT:-${RESULT_DIR}/mdns_verify_result.txt}"
REPLAY_LOG="${APLAY_MDNS_REPLAY_LOG:-${RESULT_DIR}/mdns_replay.log}"
CAPTURE_IFACE="${APLAY_CAPTURE_IFACE:-any}"
RECEIVER_NAME="APlayHarness"
DEVICE_ID="02:00:00:00:00:01"
ANNOUNCE_PID=
CAPTURE_PID=
CAPTURE_TOOL=
PREFLIGHT_CAPTURE="${TMPDIR:-/tmp}/aplay-mdns-preflight.pcapng"

cleanup() {
    if [ -n "${ANNOUNCE_PID}" ]; then
        kill "${ANNOUNCE_PID}" 2>/dev/null || true
        wait "${ANNOUNCE_PID}" 2>/dev/null || true
    fi
    if [ -n "${CAPTURE_PID}" ]; then
        kill "${CAPTURE_PID}" 2>/dev/null || true
        wait "${CAPTURE_PID}" 2>/dev/null || true
    fi
}

trap cleanup EXIT INT TERM

write_result() {
    STATUS=$1
    PHASE=$2
    MESSAGE=$3

    mkdir -p "${RESULT_DIR}"
    {
        echo "status=${STATUS}"
        echo "phase=${PHASE}"
        echo "capture=${PCAP_CAPTURE}"
        echo "replay_log=${REPLAY_LOG}"
        echo "message=${MESSAGE}"
    } >"${RESULT_FILE}"
}

capture_setup_error() {
    write_result fail capture-setup "failed to start live mDNS capture without unprivileged packet capture"
    cat >&2 <<EOF
failed to start live mDNS capture without unprivileged packet capture

Configure capture privileges once outside the harness:

Debian/Ubuntu:
  sudo apt-get install wireshark-common
  sudo dpkg-reconfigure wireshark-common
  sudo usermod -aG wireshark "${USER:-${LOGNAME:-current-user}}"
  # start a new login session after changing groups

Capability-based alternative:
  sudo setcap cap_net_raw,cap_net_admin=eip "\$(command -v dumpcap)"
  # or:
  sudo setcap cap_net_raw,cap_net_admin=eip "\$(command -v tcpdump)"
EOF
    exit 1
}

dependency_setup_error() {
    write_result fail dependencies "missing dependencies for Linux harness validation"
    cat >&2 <<EOF
missing dependencies for Linux harness validation

Required commands: git, cmake, ninja, timeout, and either dumpcap or tcpdump.

Debian/Ubuntu setup:
  sudo apt-get install git cmake ninja-build tcpdump wireshark-common
  sudo dpkg-reconfigure wireshark-common
  sudo usermod -aG wireshark "${USER:-${LOGNAME:-current-user}}"
  # start a new login session after changing groups
EOF
    exit 1
}

require_command() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "missing required command: $1" >&2
        return 1
    fi
}

check_dependencies() {
    MISSING=0
    require_command git || MISSING=1
    require_command cmake || MISSING=1
    require_command ninja || MISSING=1
    require_command timeout || MISSING=1

    if ! command -v dumpcap >/dev/null 2>&1 && ! command -v tcpdump >/dev/null 2>&1; then
        echo "missing required command: dumpcap or tcpdump" >&2
        MISSING=1
    fi

    if [ "${MISSING}" -ne 0 ]; then
        dependency_setup_error
    fi
}

try_dumpcap_capture() {
    OUTPUT_FILE=$1

    if ! command -v dumpcap >/dev/null 2>&1; then
        return 1
    fi

    timeout 15 dumpcap -i "${CAPTURE_IFACE}" -p -f "udp port 5353" -c 3 \
        -w "${OUTPUT_FILE}" &
    CAPTURE_PID=$!
    sleep 1
    if kill -0 "${CAPTURE_PID}" 2>/dev/null; then
        CAPTURE_TOOL=dumpcap
        return 0
    fi

    wait "${CAPTURE_PID}" 2>/dev/null || true
    CAPTURE_PID=
    rm -f "${OUTPUT_FILE}"
    return 1
}

probe_dumpcap_capture() {
    if ! command -v dumpcap >/dev/null 2>&1; then
        return 1
    fi

    timeout 15 dumpcap -i "${CAPTURE_IFACE}" -p -f "udp port 5353" \
        -w "${PREFLIGHT_CAPTURE}" &
    CAPTURE_PID=$!
    sleep 1
    if ! kill -0 "${CAPTURE_PID}" 2>/dev/null; then
        wait "${CAPTURE_PID}" 2>/dev/null || true
        CAPTURE_PID=
        rm -f "${PREFLIGHT_CAPTURE}"
        return 1
    fi

    CAPTURE_TOOL=dumpcap
    kill "${CAPTURE_PID}" 2>/dev/null || true
    wait "${CAPTURE_PID}" 2>/dev/null || true
    CAPTURE_PID=
    return 0
}

try_tcpdump_capture() {
    OUTPUT_FILE=$1

    if ! command -v tcpdump >/dev/null 2>&1; then
        return 1
    fi

    timeout 15 tcpdump -i "${CAPTURE_IFACE}" -p -n -U -s 0 -c 3 "udp port 5353" \
        -w - >"${OUTPUT_FILE}" &
    CAPTURE_PID=$!
    sleep 1
    if kill -0 "${CAPTURE_PID}" 2>/dev/null; then
        CAPTURE_TOOL=tcpdump
        return 0
    fi

    wait "${CAPTURE_PID}" 2>/dev/null || true
    CAPTURE_PID=
    rm -f "${OUTPUT_FILE}"
    return 1
}

probe_tcpdump_capture() {
    if ! command -v tcpdump >/dev/null 2>&1; then
        return 1
    fi

    timeout 15 tcpdump -i "${CAPTURE_IFACE}" -p -n -U -s 0 "udp port 5353" \
        -w - >"${PREFLIGHT_CAPTURE}" &
    CAPTURE_PID=$!
    sleep 1
    if ! kill -0 "${CAPTURE_PID}" 2>/dev/null; then
        wait "${CAPTURE_PID}" 2>/dev/null || true
        CAPTURE_PID=
        rm -f "${PREFLIGHT_CAPTURE}"
        return 1
    fi

    CAPTURE_TOOL=tcpdump
    kill "${CAPTURE_PID}" 2>/dev/null || true
    wait "${CAPTURE_PID}" 2>/dev/null || true
    CAPTURE_PID=
    return 0
}

check_capture_permissions() {
    rm -f "${PREFLIGHT_CAPTURE}"
    probe_dumpcap_capture || probe_tcpdump_capture || capture_setup_error
    rm -f "${PREFLIGHT_CAPTURE}"
    echo "using ${CAPTURE_TOOL} for live mDNS capture"
}

start_capture() {
    if [ "${CAPTURE_TOOL}" = "dumpcap" ]; then
        try_dumpcap_capture "${PCAP_CAPTURE}"
    elif [ "${CAPTURE_TOOL}" = "tcpdump" ]; then
        try_tcpdump_capture "${PCAP_CAPTURE}"
    else
        return 1
    fi
}

echo "=== Checking Linux harness dependencies ==="
check_dependencies
check_capture_permissions

echo "=== Syncing and updating submodules ==="
git -C "${ROOT_DIR}" submodule sync --recursive
git -C "${ROOT_DIR}" submodule update --init --remote --recursive

cmake -S "${ROOT_DIR}/app/linux" -B "${BUILD_DIR}" -G Ninja \
    -DAPLAY_BUILD_LINUX=ON \
    -DAPLAY_BUILD_HARNESS=ON
cmake --build "${BUILD_DIR}" --target \
    APlayReceiver \
    aplay_sdk \
    aplay_harness_mdns_replay \
    aplay_harness_mdns_announce

mkdir -p "$(dirname -- "${PCAP_CAPTURE}")" "${RESULT_DIR}"
rm -f "${PCAP_CAPTURE}"
rm -f "${RESULT_FILE}" "${REPLAY_LOG}"

echo "=== Capturing live mDNS announcements ==="
start_capture || {
    write_result fail capture-start "failed to start live mDNS capture"
    capture_setup_error
}

"${BUILD_DIR}/harness/mdns/aplay_harness_mdns_announce" --interval-ms 250 "${RECEIVER_NAME}" &
ANNOUNCE_PID=$!

set +e
wait "${CAPTURE_PID}"
CAPTURE_STATUS=$?
CAPTURE_PID=
set -e

kill "${ANNOUNCE_PID}" 2>/dev/null || true
wait "${ANNOUNCE_PID}" 2>/dev/null || true
ANNOUNCE_PID=

if [ "${CAPTURE_STATUS}" -ne 0 ]; then
    write_result fail capture "failed to capture live mDNS announcements"
    echo "failed to capture live mDNS announcements into ${PCAP_CAPTURE}" >&2
    exit "${CAPTURE_STATUS}"
fi

if [ ! -s "${PCAP_CAPTURE}" ]; then
    write_result fail capture "captured pcap is missing or empty"
    echo "captured pcap is missing or empty: ${PCAP_CAPTURE}" >&2
    exit 1
fi

echo "=== Analyzing captured mDNS pcap ==="
set +e
"${BUILD_DIR}/harness/mdns/aplay_harness_mdns_replay" \
    "${PCAP_CAPTURE}" "${RECEIVER_NAME}" "${DEVICE_ID}" >"${REPLAY_LOG}" 2>&1
REPLAY_STATUS=$?
set -e
cat "${REPLAY_LOG}"

if [ "${REPLAY_STATUS}" -ne 0 ]; then
    write_result fail pcap-analysis "mDNS replay analysis failed"
    echo "mDNS replay analysis failed for ${PCAP_CAPTURE}" >&2
    exit "${REPLAY_STATUS}"
fi

write_result pass pcap-analysis "mDNS capture analysis passed"
echo "=== mDNS harness validation passed ==="
