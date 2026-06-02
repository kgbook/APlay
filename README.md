# APlay

APlay is a Linux-compatible AirPlay receiver.

Current status: phase 0 engineering baseline.

- Linux is built from `app/linux/CMakeLists.txt`.
- `sdk` is the shared SDK module.
- `sdk/src/main/cpp` provides the C++ SDK and exports `.so`.
- `sdk/src/main/cpp/third-party` contains third-party C/C++ dependency submodules used by the SDK.
- `sdk/src/main/cpp/jni` provides the Java SDK native binding and exports `libaplay_jni.so`.
- `sdk/src/main/cpp/napi` provides the ETS SDK native binding and will export `libaplay_napi.so`.
- `sdk/src/main/java` provides the Java SDK and exports Android AAR.
- `sdk/src/main/ets` provides the ETS SDK facade and exports the local Harmony HAR module.
- `app/android` is the Android Gradle entry and consumes `:aplay-sdk`.
- `app/harmony` is the HarmonyOS/DevEco Studio entry and consumes the local ETS SDK HAR.
- OSAL is only the platform abstraction layer for codec, render, socket, thread, timer, file, and network interfaces.
- BLE service discovery is intentionally deferred as a TODO.

## Linux Build

`app/linux/CMakeLists.txt` is the Linux entrypoint. It imports shared native modules from `sdk/src/main/cpp` and builds the `aplay` executable.

```sh
./scripts/linux_build.sh
```

```sh
./build/linux/aplay --help
./build/linux/aplay --smoke-run 100
```

## C++ SDK

The C++ SDK lives under `sdk/src/main/cpp` and builds the `aplay_cpp_sdk` shared library.

```sh
cmake -S sdk/src/main/cpp -B build/sdk-cpp -G Ninja
cmake --build build/sdk-cpp --target aplay_cpp_sdk
```

JNI and NAPI bindings are independent C++ submodules:

```text
sdk/src/main/cpp/jni   -> aplay_jni
sdk/src/main/cpp/napi  -> aplay_napi
```

Use `APLAY_BUILD_JNI_BINDING` and `APLAY_BUILD_NAPI_BINDING` to select them.

## Java SDK AAR

The Java SDK is built from the `sdk` module and exported as an Android AAR. The app-facing Java API lives in `sdk/src/main/java` and calls the JNI binding in `sdk/src/main/cpp/jni`.

```sh
./app/android/gradlew -p app/android :aplay-sdk:assembleDebug
```

Debug AAR output:

```text
sdk/build/outputs/aar/aplay-sdk-debug.aar
```

## Android App

`app/android/build.gradle.kts` is the Android app entrypoint. It depends on `:aplay-sdk` and uses the Java SDK instead of loading native libraries directly.

```sh
./scripts/android_build.sh
```

## Harmony App and ETS SDK HAR

`app/harmony` is the DevEco Studio import entry. It builds the `entry` HAP and depends on the local `aplay_sdk` HAR module under `sdk/src/main/ets`. The ETS SDK native interface is routed through the NAPI binding in `sdk/src/main/cpp/napi`, which links `aplay_cpp_sdk`.

Make sure the Harmony toolchain commands are available in `PATH` before running the script. `DEVECO_SDK_HOME` must point at the DevEco SDK root directory.

```sh
./scripts/harmony_build.sh
```

## Submodules

Third-party C/C++ dependencies are kept under `sdk/src/main/cpp/third-party` so they stay inside the shared native SDK boundary.
`SpdlogHelper` also depends on `spdlog` as a nested submodule, so always update submodules recursively.

```sh
git submodule sync --recursive
git submodule update --init --recursive
```
