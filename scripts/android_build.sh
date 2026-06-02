#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
ANDROID_DIR="${ROOT_DIR}/app/android"
GRADLEW="${ANDROID_DIR}/gradlew"

"${GRADLEW}" -p "${ANDROID_DIR}" assembleDebug

echo "Android APK: ${ANDROID_DIR}/build/outputs/apk/debug/APlayReceiver-debug.apk"
echo "Android AAR: ${ROOT_DIR}/sdk/build/outputs/aar/APlaySdk-debug.aar"
