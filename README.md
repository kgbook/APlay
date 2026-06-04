# APlay

APlay is a cross-platform AirPlay receiver that enables seamless media casting across Linux, Android, and HarmonyOS, with future plans to expand support to additional platforms such as RTOS.

> [!NOTE]
> This document is the English version. For 中文版, see [README_zh.md](README_zh.md).

## Linux Build

`app/linux/CMakeLists.txt` is the Linux entrypoint. It imports shared native modules from `sdk/src/main/cpp` with `APLAY_BUILD_LINUX=ON` and builds the `APlayReceiver` executable and `APlaySdk` shared library.

```sh
./scripts/linux_build.sh
```

Agent validation:

```sh
./harness/verify_mdns.sh
```

This captures live UDP 5353 mDNS packets to `resources/pcap/mdns_announce.pcapng` by default, then analyzes the capture with `aplay_harness_mdns_replay`. Use `APLAY_CAPTURE_IFACE` to choose the capture interface and `APLAY_PCAP_CAPTURE` to choose the output file.

mDNS harness announcer after build:

```sh
build/linux/harness/mdns/aplay_harness_mdns_announce APlayHarness
```

By default it starts the UDP 5353 multicast responder and keeps sending AirPlay/RAOP announcement packets until `SIGINT` or `SIGTERM`. Add `--once` to only build and parse announcement packets offline.

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

## TODO

- BLE service discovery.
