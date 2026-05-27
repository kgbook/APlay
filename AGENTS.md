# Agent Notes

## Build Boundaries

- Root `CMakeLists.txt` is the Linux C/C++ entrypoint.
- Root CMake builds shared native modules and `app/linux`.
- Root CMake must not enter `app/android` as a normal CMake subdirectory.
- `app/android` is an Android Java Gradle project root.
- Android native code uses the conventional path `app/android/src/main/cpp`.
- Android Gradle points `externalNativeBuild.cmake.path` at `app/android/src/main/cpp/CMakeLists.txt`.
- `app/android/src/main/cpp/CMakeLists.txt` imports the repository root CMake project as a native dependency.

## Platform Responsibilities

- `app/linux` owns Linux UI/business logic: CLI, config, process lifecycle, signal handling, and desktop integration.
- `app/android` owns Android UI/business logic: Activity, Service, Foreground Service, permissions, notifications, and Android lifecycle.
- `osal` is only the platform capability abstraction: socket, thread, timer, file, network interface, codec, and render.
- Linux GStreamer and Android MediaCodec/AudioTrack/Surface belong behind OSAL codec/render interfaces.

## Implementation Rules

- Do not put Android Java app code under root CMake targets.
- Do not put UI or business lifecycle logic in OSAL.
- Keep protocol, streaming, and crypto independent from app platform code so harness can drive them offline.
- BLE service discovery is TODO and should not be implemented in the first baseline.

## Validation

- Linux: `cmake -S . -B build -G Ninja`, `cmake --build build`, `ctest --test-dir build --output-on-failure`.
- Android: run `./gradlew assembleDebug` from `app/android`; it imports root CMake with Linux app and harness disabled.
