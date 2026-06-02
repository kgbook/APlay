# Agent Notes

## Build Boundaries

- There is no root CMake or root Gradle entrypoint.
- `app/linux/CMakeLists.txt` is the Linux C/C++ executable entrypoint.
- `app/android/build.gradle.kts` is the Android app Gradle entrypoint.
- `app/harmony` is the HarmonyOS/DevEco Studio entrypoint.
- `sdk` is the shared SDK module.
- `sdk/src/main/cpp` owns the C++ SDK object library and the exported aplay_sdk.so.
- `sdk/src/main/cpp/third-party` owns third-party C/C++ dependency submodules used by the SDK.
- `SpdlogHelper` is a third-party submodule and has its own `spdlog` submodule; initialize third-party dependencies recursively.
- `sdk/src/main/cpp/osal/android/jni` owns the Java SDK JNI binding `.so`; build with `APLAY_BUILD_ANDROID`.
- `sdk/src/main/cpp/osal/harmony/napi` owns the ETS SDK NAPI binding `.so`; build with `APLAY_BUILD_HARMONY`.
- `sdk/src/main/cpp/osal/CMakeLists.txt` owns platform subdirectory selection, including platform codec/render modules and native binding submodules.
- `sdk/src/main/java` owns the Java SDK and exports Android AAR.
- `sdk/src/main/ets` owns the ETS SDK and exports Harmony HAR.
- `sdk/src/main/ets/libaplay_napi` owns the Harmony native module `.d.ts` package for `libaplay_napi.so`.
- `app/android` is an Android application module that consumes `:aplay-sdk`; it must not own C/C++ code.
- `app/harmony` is a Harmony app/HAP consumer entry.

## Platform Responsibilities

- `app/linux` owns Linux UI/business logic: CLI, config, process lifecycle, signal handling, and desktop integration.
- `app/android` owns Android UI/business logic: Activity, Service, Foreground Service, permissions, notifications, and Android lifecycle.
- `sdk` owns CPP/Java/ETS SDK APIs and their `.so`/AAR/HAR packaging.
- `app/harmony` owns Harmony-facing app lifecycle and imports the ETS SDK HAR.
- `utils` owns the STL-based cross-platform helpers such as socket/thread/poll and related shared utility code.
- `osal` currently owns platform codec/render modules and platform native binding submodules.

## Implementation Rules

- Do not put native C/C++ under `app`; keep the C++ SDK under `sdk/src/main/cpp`.
- After changing third-party submodule paths, run `git submodule sync --recursive` and `git submodule update --init --recursive`.
- Keep Harmony ETS SDK under `sdk/src/main/ets/com/kgbook/aplay`; Harmony app lifecycle belongs under `app/harmony`.
- Keep Harmony NAPI `.so` type declarations under `sdk/src/main/ets/<library>` with a matching `oh-package.json5`.
- Do not put UI or business lifecycle logic in OSAL.
- Keep protocol, streaming, and crypto independent from app platform code so harness can drive them offline.
- BLE service discovery is TODO and should not be implemented in the first baseline.

## Validation

- Linux: `./scripts/linux_build.sh`, or `cmake -S app/linux -B build/linux -G Ninja` then `cmake --build build/linux --target aplay`.
- C++ SDK object library is built by CMake target `aplay_cpp_sdk`; shared SDK library is built by CMake target `aplay_sdk`.
- Java JNI binding is built by CMake target `aplay_jni`.
- ETS NAPI binding is built by CMake target `aplay_napi` when Harmony native headers/toolchain are available.
- Android Java SDK AAR: `./app/android/gradlew -p app/android :aplay-sdk:assembleDebug`.
- Android app: `./scripts/android_build.sh`; `app/android` is the Gradle root and consumes `:aplay-sdk`.
- Harmony HAP: `./scripts/harmony_build.sh` with `ohpm` and `hvigorw` or `hvigor` available in `PATH`; `DEVECO_SDK_HOME` must point at the DevEco SDK root.
