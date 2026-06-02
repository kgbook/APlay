# APlay

APlay is a cross-platform AirPlay receiver that enables seamless media casting across Linux, Android, and HarmonyOS, with future plans to expand support to additional platforms such as RTOS.

Current status: phase 0 engineering baseline.

> [!NOTE]
> This document is the English version. For 中文版, see [README_zh.md](README_zh.md).

- `app/linux` is the Linux entrypoint.
- `app/android` is the Android Gradle entry, consumes the `:APlaySdk` module, and exports the `APlayReceiver` APK.
- `app/harmony` is the HarmonyOS entry and consumes the local ETS SDK HAR.
- BLE service discovery is intentionally deferred as a TODO.

## Linux Build

`app/linux/CMakeLists.txt` is the Linux entrypoint. It imports shared native modules from `sdk/src/main/cpp` with `APLAY_BUILD_LINUX=ON` and builds the `APlayReceiver` executable and `APlaySdk` shared library.

```sh
./scripts/linux_build.sh
```

## Android Build

`app/android/build.gradle.kts` is the Android app entrypoint. The Android Gradle root is named `APlayReceiver`; it depends on the `:APlaySdk` Java SDK module which lives in `sdk/src/main/java` and calls native code through the `sdk/src/main/cpp/osal/android/jni` JNI binding, which links the C++ SDK object library (`aplay_cpp_sdk`) and loads `APlaySdk` at runtime. The Android build uses `APLAY_BUILD_ANDROID=ON`; `sdk/src/main/cpp/osal/CMakeLists.txt` then enables Android OSAL codec/render modules and the JNI binding.

```sh
./scripts/android_build.sh
```

Debug AAR output:

```text
sdk/build/outputs/aar/APlaySdk-debug.aar
```

## Harmony Build

`app/harmony` is the DevEco Studio import entry. It builds the `APlayReceiver` HAP target and depends on the local `APlaySdk` HAR target rooted at `sdk`. The ETS SDK native interface is routed through the `aplay_napi` NAPI binding, which links the C++ SDK object library (`aplay_cpp_sdk`) into `libaplay_napi.so`. The `libaplay_napi.so` import is declared through the native module type package at `sdk/src/main/ets/libaplay_napi`, which is referenced by `sdk/oh-package.json5`. Harmony packaging should expose only `libaplay_napi.so`; the Harmony build uses `APLAY_BUILD_HARMONY=ON` and `OHOS_STL=c++_static`.

Build requires Harmony toolchain. Two options:

**Option 1 — DevEco Studio SDK** (Windows/macOS only): Use the SDK and bin bundled with DevEco Studio IDE.

**Option 2 — Command Line Tools** (macOS/Linux/Windows): Download [Command Line Tools for HMOS](https://developer.huawei.com/consumer/cn/download/command-line-tools-for-hmos) and extract.

By default the build script uses `${HOME}/tools/command-line-tools` as the Command Line Tools root. Override with the `HARMONY_COMMAND_LINE_TOOLS` environment variable if your installation differs.

```sh
./scripts/harmony_build.sh
```

The script sets up `HARMONY_SDK_HOME`, `PATH` (including `hvigorw`, `ohpm`, and `node`), and `NODE_HOME` automatically. It then runs `assembleHar` to output the SDK HAR, followed by `assembleHap` to output the application HAP. `app/harmony` uses `appTasks` to coordinate the application project and its modules; `assembleHar` belongs to `sdk` which uses `harTasks`. Signing is not configured yet; Hvigor preserves the unsigned HAP output.

## Submodules

Third-party C/C++ dependencies are kept under `sdk/src/main/cpp/third-party` so they stay inside the shared native SDK boundary.
`SpdlogHelper` also depends on `spdlog` as a nested submodule, so always update submodules recursively.

```sh
git submodule sync --recursive
git submodule update --init --recursive
```
