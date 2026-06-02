# APlay

APlay is a Linux-compatible AirPlay receiver.

Current status: phase 0 engineering baseline.

> [!NOTE]
> This document is the English version. For 中文版, see [README_zh.md](README_zh.md).

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
- OSAL is the platform abstraction layer for codec and render interfaces.
- BLE service discovery is intentionally deferred as a TODO.

## Linux Build

`app/linux/CMakeLists.txt` is the Linux entrypoint. It imports shared native modules from `sdk/src/main/cpp` and builds the `aplay` executable and `aplay_cpp_sdk` shared library.

```sh
./scripts/linux_build.sh
```

```sh
./build/linux/aplay --help
./build/linux/aplay --smoke-run 100
```

## Android Build

`app/android/build.gradle.kts` is the Android app entrypoint. It depends on `:aplay-sdk` Java SDK which lives in `sdk/src/main/java` and calls native code through `sdk/src/main/cpp/jni` JNI binding, which links back to `aplay_cpp_sdk`. The Android build also compiles `aplay_jni` when `APLAY_BUILD_JNI_BINDING=ON` is set in cmake arguments.

```sh
./scripts/android_build.sh
```

Debug AAR output:

```text
sdk/build/outputs/aar/aplay-sdk-debug.aar
```

## Harmony Build

`app/harmony` is the DevEco Studio import entry. It builds the `entry` HAP and depends on the local `aplay_sdk` HAR module under `sdk/src/main/ets`. The ETS SDK native interface is routed through `aplay_napi` NAPI binding (compiled with `APLAY_BUILD_NAPI_BINDING=ON`), which links `aplay_cpp_sdk`.

Build requires Harmony toolchain. Two options:

**Option 1 — DevEco Studio SDK** (Windows/macOS only): Use the SDK and bin bundled with DevEco Studio IDE.

**Option 2 — Command Line Tools** (macOS/Linux/Windows): Download [Command Line Tools for HMOS](https://developer.huawei.com/consumer/cn/download/command-line-tools-for-hmos) and extract.

Configure either one with:

```sh
export DEVECO_SDK_HOME="/path/to/sdk"          # DevEco Studio: .../sdk   Command Line Tools: .../command-line-tools/sdk
export PATH="/path/to/bin:/path/to/hvigor/bin:/path/to/ohpm/bin:$PATH"
```

Then run:

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