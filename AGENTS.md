# Agent Notes

## Build Boundaries

- There is no root CMake or root Gradle entrypoint.
- `app/linux/CMakeLists.txt` is the Linux C/C++ executable entrypoint.
- `app/android/build.gradle.kts` is the Android app Gradle entrypoint.
- `app/harmony` is the HarmonyOS/DevEco Studio entrypoint.
- `sdk` is the shared SDK module.
- `sdk/src/main/cpp` owns the C++ SDK object library and the exported `aplay_sdk` `.so`.
- `sdk/src/main/cpp/third-party` owns third-party C/C++ dependency submodules used by the SDK.
- `SpdlogHelper` is a third-party submodule and has its own `spdlog` submodule; initialize third-party dependencies recursively.
- `sdk/src/main/cpp/osal/android/jni` owns the Java SDK JNI binding `.so`; build with `APLAY_BUILD_ANDROID`.
- `sdk/src/main/cpp/osal/harmony/napi` owns the ETS SDK NAPI binding `.so`; build with `APLAY_BUILD_HARMONY`.
- `sdk/src/main/cpp/osal/CMakeLists.txt` owns platform subdirectory selection, including platform codec/render modules and native binding submodules.
- `sdk/src/main/java` owns the Java SDK and exports Android AAR.
- `sdk/src/main/ets` owns the ETS SDK facade and exports the local Harmony HAR module.
- `app/android` is an Android application module that consumes `:aplay-sdk`; it must not own C/C++ code.
- `app/harmony` is a Harmony app/HAP consumer entry. HarmonyOS TV/large-screen support remains TODO beyond the baseline shell.

## Platform Responsibilities

- `app/linux` owns Linux UI/business logic: CLI, config, process lifecycle, signal handling, and desktop integration.
- `app/android` owns Android UI/business logic: Activity, Service, Foreground Service, permissions, notifications, and Android lifecycle.
- `sdk` owns CPP/Java/ETS SDK APIs and their `.so`/AAR/HAR packaging.
- `app/harmony` owns Harmony-facing app lifecycle and imports the ETS SDK HAR.
- `utils` owns the STL-based cross-platform helpers such as socket/thread/poll and related shared utility code.
- `osal` currently owns platform codec/render modules and platform native binding submodules.
- Linux GStreamer and Android MediaCodec/AudioTrack/Surface belong behind OSAL codec/render interfaces.

## Implementation Rules

- Do not put Android Java app code under root CMake targets.
- Do not put Android native C/C++ under `app/android`; keep the C++ SDK under `sdk/src/main/cpp`.
- Do not restore root CMake, root Gradle, root Gradle wrapper, or root Gradle settings as platform entrypoints.
- Do not put third-party C/C++ dependency submodules at the repository root; keep them under `sdk/src/main/cpp/third-party`.
- After changing third-party submodule paths, run `git submodule sync --recursive` and `git submodule update --init --recursive`.
- Do not make `app/android` load native libraries directly; it should use the Java SDK in `sdk/src/main/java`.
- Keep Harmony ETS SDK facade and HAR configuration under `sdk/src/main/ets`; Harmony app lifecycle belongs under `app/harmony`.
- Do not put UI or business lifecycle logic in OSAL.
- Keep protocol, streaming, and crypto independent from app platform code so harness can drive them offline.
- BLE service discovery is TODO and should not be implemented in the first baseline.
- Harmony large-screen business logic is TODO; the baseline may include importable DevEco/HAP/HAR configuration only.

## Validation

- Linux: `./scripts/linux_build.sh`, or `cmake -S app/linux -B build/linux -G Ninja` then `cmake --build build/linux --target aplay`.
- C++ SDK object library is built by CMake target `aplay_cpp_sdk`; shared SDK library is built by CMake target `aplay_sdk`.
- Java JNI binding is built by CMake target `aplay_jni`.
- ETS NAPI binding is built by CMake target `aplay_napi` when Harmony native headers/toolchain are available.
- Android Java SDK AAR: `./app/android/gradlew -p app/android :aplay-sdk:assembleDebug`.
- Android app: `./scripts/android_build.sh`; `app/android` is the Gradle root and consumes `:aplay-sdk`.
- Harmony HAP: `./scripts/harmony_build.sh` with `ohpm` and `hvigorw` or `hvigor` available in `PATH`; `DEVECO_SDK_HOME` must point at the DevEco SDK root.
