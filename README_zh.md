# APlay

APlay 是跨平台 AirPlay 接收端，实现 Linux、Android 与 HarmonyOS 间的跨生态投屏，未来规划扩展支持 RTOS 等更多平台。

> [!NOTE]
> 本文档为中文版。英文版请参阅 [README.md](README.md)。

## Linux 构建

`app/linux/CMakeLists.txt` 是 Linux 构建入口，通过 `APLAY_BUILD_LINUX=ON` 从 `sdk/src/main/cpp` 导入共享 native 模块并构建 `APlayReceiver` 可执行文件和 `aplay-sdk` shared library。
运行时，`APlayReceiver` 会使用系统分配的接收端服务端口，并通过 mDNS 广播该 AirPlay/RAOP 服务端口，用于首批控制端点。

```sh
./scripts/linux_build.sh
```

## Android 构建

`app/android/build.gradle.kts` 是 Android app 入口。Android Gradle root 命名为 `APlayReceiver`，并依赖 `:aplay-sdk` Java SDK module。Java SDK 位于 `sdk/src/main/java`，native 接口通过 `sdk/src/main/cpp/osal/android/jni` 的 JNI binding 连接 C++ SDK facade（`aplay_cpp_sdk`），运行时加载 `aplay-sdk`。Android 构建时通过 `APLAY_BUILD_ANDROID=ON` 进入。

```sh
./scripts/android_build.sh
```

debug AAR 输出位置：

```text
sdk/build/outputs/aar/aplay-sdk-debug.aar
```

## Harmony 构建

`app/harmony` 是 DevEco Studio 导入入口，构建 `APlayReceiver` HAP target，并依赖以 `sdk` 为根的本地 `aplay-sdk` HAR target。Harmony 构建使用 `APLAY_BUILD_HARMONY=ON`。

构建需要配置 Harmony 工具链，有两种方式：

**方式一 — DevEco Studio 配套 SDK**（仅 Windows/macOS）：使用 DevEco Studio IDE 自带的 SDK 和 bin 目录。

**方式二 — Command Line Tools**（macOS/Linux/Windows 均有）：下载 [Command Line Tools for HMOS](https://developer.huawei.com/consumer/cn/download/command-line-tools-for-hmos) 并解压。

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

## TODO

- BLE 服务发现。
