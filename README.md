# APlay

APlay is a cross-platform AirPlay receiver that enables seamless media casting across Linux, Android, and HarmonyOS, with future plans to expand support to additional platforms such as RTOS.

Current status: phase 0 engineering baseline.

> [!NOTE]
> This document is the English version. For 中文版, see [README_zh.md](README_zh.md).

- `app/linux` is the Linux entrypoint.
- `app/android` is the Android Gradle entry, consumes the `:APlaySdk` module, and exports the `APlayReceiver` APK.
- `app/harmony` is the HarmonyOS entry and consumes the local ETS SDK HAR.
- Project-owned native C/C++ is kept C++11-compatible for embedded toolchains and uses POSIX APIs plus the C++ STL.
- `harness` contains agent/CI validation scripts; `example` builds manually runnable example binaries.
- BLE service discovery is intentionally deferred as a TODO.

## Linux Build

`app/linux/CMakeLists.txt` is the Linux entrypoint. It imports shared native modules from `sdk/src/main/cpp` with `APLAY_BUILD_LINUX=ON` and builds the `APlayReceiver` executable and `APlaySdk` shared library.

```sh
./scripts/linux_build.sh
```

Agent/CI validation:

```sh
./harness/verify_linux.sh
```

## Android Build

`app/android/build.gradle.kts` is the Android app entrypoint. The Android Gradle root is named `APlayReceiver`; it depends on the `:APlaySdk` Java SDK module which lives in `sdk/src/main/java` and calls native code through the `sdk/src/main/cpp/osal/android/jni` JNI binding, which links the C++ SDK object library (`aplay_cpp_sdk`) and loads `APlaySdk` at runtime. 

The Android build uses `APLAY_BUILD_ANDROID=ON`.

```sh
./scripts/android_build.sh
```

Debug AAR output:

```text
sdk/build/outputs/aar/APlaySdk-debug.aar
```

## Harmony Build

`app/harmony` is the DevEco Studio import entry. It builds the `APlayReceiver` HAP target and depends on the local `APlaySdk` HAR target rooted at `sdk`. the Harmony build uses `APLAY_BUILD_HARMONY=ON`.

Build requires Harmony toolchain. Two options:

**Option 1 — DevEco Studio SDK** (Windows/macOS only): Use the SDK and bin bundled with DevEco Studio IDE.

**Option 2 — Command Line Tools** (macOS/Linux/Windows): Download [Command Line Tools for HMOS](https://developer.huawei.com/consumer/cn/download/command-line-tools-for-hmos) and extract.

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
