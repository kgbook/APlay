# APlay

APlay 是面向 UxPlay 兼容 AirPlay 接收端的分期重写工程。

当前状态：Phase 0 工程基线。

> [!NOTE]
> 本文档为中文版。英文版请参阅 [README.md](README.md)。

- Linux 由 `app/linux/CMakeLists.txt` 作为入口构建。
- `sdk` 是共享 SDK module。
- `sdk/src/main/cpp` 提供 C++ SDK 对象源码，并输出共享的 `aplay_sdk` `.so`。
- `sdk/src/main/cpp/utils` 提供跨平台工具辅助实现，例如 socket/thread/poll。
- `sdk/src/main/cpp/third-party` 存放 SDK 使用的第三方 C/C++ 依赖子模块。
- `sdk/src/main/cpp/osal/android/jni` 提供 Java SDK native binding，对外输出 `libaplay_jni.so`。
- `sdk/src/main/cpp/osal/harmony/napi` 提供 ETS SDK native binding，未来对外输出 `libaplay_napi.so`。
- `sdk/src/main/java` 提供 Java SDK，对外输出 Android AAR。
- `sdk/src/main/ets` 提供 ETS SDK 门面，并作为本地 Harmony HAR module 输出。
- `app/android` 是 Android Gradle 入口，通过 `:aplay-sdk` 使用 Java SDK。
- `app/harmony` 是 HarmonyOS/DevEco Studio 入口，通过本地 ETS SDK HAR 使用共享能力。
- OSAL 作为 codec/render 接口的平台抽象层，并负责各平台 native binding 子模块。
- BLE 服务发现首版暂不实现，作为 TODO 保留。

## Linux 构建

`app/linux/CMakeLists.txt` 是 Linux 构建入口，通过 `APLAY_BUILD_LINUX=ON` 从 `sdk/src/main/cpp` 导入共享 native 模块并构建 `aplay` 可执行文件和 `aplay_sdk` shared library。

```sh
./scripts/linux_build.sh
```

```sh
./build/linux/aplay --help
./build/linux/aplay --smoke-run 100
```

## Android 构建

`app/android/build.gradle.kts` 是 Android app 入口，依赖 `:aplay-sdk` Java SDK。Java SDK 位于 `sdk/src/main/java`，native 接口通过 `sdk/src/main/cpp/osal/android/jni` 的 JNI binding 连接 C++ SDK 对象库（`aplay_cpp_sdk`），运行时加载 `aplay_jni`。Android 构建时通过 `APLAY_BUILD_ANDROID=ON` 进入 `sdk/src/main/cpp/osal/CMakeLists.txt`，再启用 Android OSAL codec/render 模块并编译 `aplay_jni`。

```sh
./scripts/android_build.sh
```

debug AAR 输出位置：

```text
sdk/build/outputs/aar/aplay-sdk-debug.aar
```

## Harmony 构建

`app/harmony` 是 DevEco Studio 导入入口，构建 `entry` HAP，并依赖 `sdk/src/main/ets` 下的本地 `aplay_sdk` HAR module。ETS SDK native 接口通过 `aplay_napi` NAPI binding 连接 C++ SDK 对象库（`aplay_cpp_sdk`）。Harmony 构建时通过 `APLAY_BUILD_HARMONY=ON` 进入 `sdk/src/main/cpp/osal/CMakeLists.txt`，再启用 Harmony OSAL codec/render 模块并编译 `aplay_napi`。

构建需要配置 Harmony 工具链，有两种方式：

**方式一 — DevEco Studio 配套 SDK**（仅 Windows/macOS）：使用 DevEco Studio IDE 自带的 SDK 和 bin 目录。

**方式二 — Command Line Tools**（macOS/Linux/Windows 均有）：下载 [Command Line Tools for HMOS](https://developer.huawei.com/consumer/cn/download/command-line-tools-for-hmos) 并解压。

任选其一，配置环境变量：

```sh
export DEVECO_SDK_HOME="/path/to/sdk"          # DevEco Studio: .../sdk   Command Line Tools: .../command-line-tools/sdk
export PATH="/path/to/bin:/path/to/hvigor/bin:/path/to/ohpm/bin:$PATH"
```

然后执行构建：

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
