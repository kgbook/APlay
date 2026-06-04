# APlay

APlay 是跨平台 AirPlay 接收端，实现 Linux、Android 与 HarmonyOS 间的跨生态投屏，未来规划扩展支持 RTOS 等更多平台。

当前状态：Phase 0 工程基线。

> [!NOTE]
> 本文档为中文版。英文版请参阅 [README.md](README.md)。

- `app/linux` 是 Linux 构建入口。
- `app/android` 是 Android Gradle 入口，通过 `:APlaySdk` module 使用 Java SDK，并输出 `APlayReceiver` APK。
- `app/harmony` 是 HarmonyOS 入口，通过本地 ETS SDK HAR 使用共享能力。
- 工程自有 native C/C++ 保持 C++11 兼容，面向嵌入式工具链；共享 native 代码使用 POSIX API 和 C++ STL。
- `harness` 放 agent/CI 调用的验证脚本；`example` 产出给人手动验证的可执行示例程序。
- BLE 服务发现首版暂不实现，作为 TODO 保留。

## Linux 构建

`app/linux/CMakeLists.txt` 是 Linux 构建入口，通过 `APLAY_BUILD_LINUX=ON` 从 `sdk/src/main/cpp` 导入共享 native 模块并构建 `APlayReceiver` 可执行文件和 `APlaySdk` shared library。

```sh
./scripts/linux_build.sh
```

agent/CI 验证入口：

```sh
./harness/verify_linux.sh
```

构建后可手动运行 mDNS 示例：

```sh
build/linux/example/aplay_example_mdns_announce APlayExample
```

加 `--serve` 可尝试启动真实 UDP 5353 multicast 广播。默认模式只做离线包生成和解析，避免开发机系统 mDNS 服务已占用 UDP 5353 时示例无法运行。mDNS 公共 API 从 `sdk/src/main/cpp/protocol/mdns/include/mdns.hpp` 导出，实现文件位于 `sdk/src/main/cpp/protocol/mdns/src`。

## Android 构建

`app/android/build.gradle.kts` 是 Android app 入口。Android Gradle root 命名为 `APlayReceiver`，并依赖 `:APlaySdk` Java SDK module。Java SDK 位于 `sdk/src/main/java`，native 接口通过 `sdk/src/main/cpp/osal/android/jni` 的 JNI binding 连接 C++ SDK 对象库（`aplay_cpp_sdk`），运行时加载 `APlaySdk`。Android 构建时通过 `APLAY_BUILD_ANDROID=ON` 进入 `sdk/src/main/cpp/osal/CMakeLists.txt`，再启用 Android OSAL codec/render 模块并编译 JNI binding。

```sh
./scripts/android_build.sh
```

debug AAR 输出位置：

```text
sdk/build/outputs/aar/APlaySdk-debug.aar
```

## Harmony 构建

`app/harmony` 是 DevEco Studio 导入入口，构建 `APlayReceiver` HAP target，并依赖以 `sdk` 为根的本地 `APlaySdk` HAR target。ETS SDK native 接口通过 `aplay_napi` NAPI binding 连接 C++ SDK 对象库（`aplay_cpp_sdk`），并合入 `libaplay_napi.so`。`libaplay_napi.so` import 通过 `sdk/src/main/ets/libaplay_napi` 下的 native module 声明包提供类型，并由 `sdk/oh-package.json5` 引用。Harmony 包内应只暴露 `libaplay_napi.so`；Harmony 构建使用 `APLAY_BUILD_HARMONY=ON` 和 `OHOS_STL=c++_static`。

构建需要配置 Harmony 工具链，有两种方式：

**方式一 — DevEco Studio 配套 SDK**（仅 Windows/macOS）：使用 DevEco Studio IDE 自带的 SDK 和 bin 目录。

**方式二 — Command Line Tools**（macOS/Linux/Windows 均有）：下载 [Command Line Tools for HMOS](https://developer.huawei.com/consumer/cn/download/command-line-tools-for-hmos) 并解压。

macOS 下如果已安装 DevEco Studio，构建脚本会自动使用 `/Applications/DevEco-Studio.app/Contents` 或 `/Applications/DevEco Studio.app/Contents`；否则回退到 `${HOME}/tools/command-line-tools` 作为 Command Line Tools 根目录。如需指定其他路径，可通过 `HARMONY_COMMAND_LINE_TOOLS` 环境变量覆盖。路径包含空格时需要加引号或使用反斜杠转义，例如 `HARMONY_COMMAND_LINE_TOOLS="/Applications/DevEco Studio.app/Contents"` 或 `HARMONY_COMMAND_LINE_TOOLS=/Applications/DevEco\ Studio.app/Contents`。

```sh
./scripts/harmony_build.sh
```

脚本会自动配置 `HARMONY_SDK_HOME`、`PATH`（包含 `hvigorw`、`ohpm` 和 `node`）以及 `NODE_HOME`，然后依次执行 `assembleHar` 输出 SDK HAR，再执行 `assembleHap` 输出应用 HAP。`app/harmony` 使用 `appTasks` 协调应用工程及其模块；`assembleHar` 属于使用 `harTasks` 的 `sdk` 模块。当前暂不配置签名，Hvigor 会保留 unsigned HAP 输出。

## 子模块

第三方 C/C++ 依赖统一放在 `sdk/src/main/cpp/third-party`，保持在共享 native SDK 边界内。
`SpdlogHelper` 还依赖嵌套子模块 `spdlog`，因此初始化和更新子模块时必须递归执行。

```sh
git submodule sync --recursive
git submodule update --init --recursive
```
