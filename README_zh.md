# APlay

APlay 是面向 UxPlay 兼容 AirPlay 接收端的分期重写工程。

当前状态：Phase 0 工程基线。

> [!NOTE]
> 本文档为中文版。英文版请参阅 [README.md](README.md)。

- Linux 由 `app/linux/CMakeLists.txt` 作为入口构建。
- `sdk` 是共享 SDK module。
- `sdk/src/main/cpp` 提供 C++ SDK，对外输出 `.so`。
- `sdk/src/main/cpp/third-party` 存放 SDK 使用的第三方 C/C++ 依赖子模块。
- `sdk/src/main/cpp/jni` 提供 Java SDK native binding，对外输出 `libaplay_jni.so`。
- `sdk/src/main/cpp/napi` 提供 ETS SDK native binding，未来对外输出 `libaplay_napi.so`。
- `sdk/src/main/java` 提供 Java SDK，对外输出 Android AAR。
- `sdk/src/main/ets` 提供 ETS SDK 门面，并作为本地 Harmony HAR module 输出。
- `app/android` 是 Android Gradle 入口，通过 `:aplay-sdk` 使用 Java SDK。
- `app/harmony` 是 HarmonyOS/DevEco Studio 入口，通过本地 ETS SDK HAR 使用共享能力。
- OSAL 作为 codec、render 接口的平台抽象层。
- BLE 服务发现首版暂不实现，作为 TODO 保留。

## Linux 构建

`app/linux/CMakeLists.txt` 是 Linux 构建入口，从 `sdk/src/main/cpp` 导入共享 native 模块并构建 `aplay` 可执行文件。

```sh
./scripts/linux_build.sh
```

```sh
./build/linux/aplay --help
./build/linux/aplay --smoke-run 100
```

## C++ SDK

C++ SDK 位于 `sdk/src/main/cpp`，构建目标是 `aplay_cpp_sdk` shared library。

```sh
cmake -S sdk/src/main/cpp -B build/sdk-cpp -G Ninja
cmake --build build/sdk-cpp --target aplay_cpp_sdk
```

JNI 和 NAPI 是独立的 C++ binding 子模块：

```text
sdk/src/main/cpp/jni   -> aplay_jni
sdk/src/main/cpp/napi  -> aplay_napi
```

通过 `APLAY_BUILD_JNI_BINDING` 和 `APLAY_BUILD_NAPI_BINDING` 条件编译。

## Java SDK AAR

Java SDK 由 `sdk` module 构建，并作为 Android AAR 对外提供。面向 app 的 Java API 位于 `sdk/src/main/java`，native 接口调用 `sdk/src/main/cpp/jni`。

```sh
./app/android/gradlew -p app/android :aplay-sdk:assembleDebug
```

debug AAR 输出位置：

```text
sdk/build/outputs/aar/aplay-sdk-debug.aar
```

## Android App

`app/android/build.gradle.kts` 是 Android app 入口，依赖 `:aplay-sdk`，并通过 Java SDK 接口调用 native 能力，不直接加载 native library。

```sh
./scripts/android_build.sh
```

## Harmony App 与 ETS SDK HAR

`app/harmony` 是 DevEco Studio 导入入口，构建 `entry` HAP，并依赖 `sdk/src/main/ets` 下的本地 `aplay_sdk` HAR module。ETS SDK native 接口通过 `sdk/src/main/cpp/napi` 的 NAPI binding 连接到 `aplay_cpp_sdk`。

执行脚本前需要确保 Harmony 工具链命令已在 `PATH` 中，例如 `ohpm` 和 `hvigorw`/`hvigor`。`DEVECO_SDK_HOME` 必须指向 DevEco SDK 根目录。

```sh
./scripts/harmony_build.sh
```

## 子模块

第三方 C/C++ 依赖统一放在 `sdk/src/main/cpp/third-party`，保持在共享 native SDK 边界内。
`SpdlogHelper` 还依赖嵌套子模块 `spdlog`，因此初始化和更新子模块时必须递归执行。

```sh
git submodule sync --recursive
git submodule update --init --recursive
```
