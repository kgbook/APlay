#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

sh "${ROOT_DIR}/scripts/update_submodules.sh" "${ROOT_DIR}"

ANDROID_DIR="${ROOT_DIR}/app/android"
GRADLEW="${ANDROID_DIR}/gradlew"

export ANDROID_HOME="${ANDROID_HOME:-${HOME}/Android/Sdk}"
export ANDROID_SDK_ROOT="${ANDROID_SDK_ROOT:-${ANDROID_HOME}}"
export GRADLE_USER_HOME="${GRADLE_USER_HOME:-${ROOT_DIR}/.gradle}"
export PATH="${ANDROID_HOME}/cmdline-tools/latest/bin:${ANDROID_HOME}/cmdline-tools/bin:${ANDROID_HOME}/platform-tools:${PATH}"

"${GRADLEW}" -p "${ANDROID_DIR}" --no-daemon assembleDebug

echo "Android APK: ${ANDROID_DIR}/build/outputs/apk/debug/APlayReceiver-debug.apk"
echo "Android AAR: ${ROOT_DIR}/sdk/build/outputs/aar/aplay-sdk-debug.aar"
