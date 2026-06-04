# APlay Harness Design

## 目标

harness 是 APlay 的实现、验收、回归和完善入口。当前只验证已经实现的 mDNS responder，同时保留真机抓包集成入口。
harness 由 agent/CI 调用；`example` 目录只保留已经实现的 mDNS 手动验证二进制，两者职责分开。

## 职责边界

- `harness/`
  只放 agent/CI 使用的验证编排：构建命令、目标选择、fixture/resource 路径、执行顺序和 pass/fail 判断。harness 可以调用 `example` 产出的二进制，但不放示例实现源码，也不作为人手动体验功能的入口。
- `example/`
  放人可以直接运行的示例源码和可执行目标，用于手动确认已经实现的能力。example 只展示当前真实可用的功能；未实现的协议、加密、RTP、BLE 等能力不放占位示例。
- `resources/`
  放跨验证入口共享的外部输入，例如真实抓包。当前 mDNS replay 使用 `resources/pcap/Airplay_Discovery_and_Connection.pcapng`，避免把抓包 fixture 混进人手动运行的 example 目录。

## 工程结构

- `harness/verify_linux.sh`
  agent/CI 调用入口；构建 Linux SDK、APlayReceiver 和 mDNS example 二进制，并执行离线验证。
- `example/mdns_replay.cpp`
  离线验证 mDNS responder 生成的 PTR/SRV/TXT/A/goodbye 记录。
- `example/mdns_announce.cpp`
  默认离线生成并解析 mDNS announcement；加 `--serve` 后启动短时 POSIX mDNS responder，供人手动浏览 `_airplay._tcp.local` / `_raop._tcp.local`。

## 验收类型

### Syntax

- CMake configure。
- C++ compile。
- harness 脚本可重复执行。

### Runtime

- `APlayReceiver` 不要求传入 options，应可直接启动。
- `aplay_cpp_sdk` 作为对象库承载共享 SDK 源码，公共 runtime 实现位于 `sdk/src/main/cpp/core/runtime`。
- `aplay_sdk` 可构建并输出 `libAPlaySdk.so`。
- `aplay_jni` 可作为 Java SDK native binding 构建，并为 Android 打包输出 `libAPlaySdk.so`。
- `aplay_napi` 保留 ETS SDK native binding 条件编译入口，并由 Harmony HAR native 构建启用。
- `sdk/src/main/ets/libaplay_napi` 提供 `libaplay_napi.so` 的 Harmony native module 类型声明包。
- `:APlaySdk` 可构建 Java SDK 并输出 `APlaySdk` Android AAR。
- `sdk` 提供 Harmony HAR module 配置，`sdk/src/main/ets/com/kgbook/aplay` 提供 ETS SDK facade。
- `app/android` host stub 可通过 Java SDK 构建并输出 `APlayReceiver` APK。
- 后续扩展为端口监听、请求响应、退出清理。

### mDNS

- mDNS PTR/SRV/TXT/A announcement。
- mDNS goodbye 记录。
- AirPlay/RAOP PTR query response。
- pcap 原始文件不直接作为唯一 golden；真机抓包作为增强验收，不替代离线门禁。

## 当前 Phase 0 行为

当前 harness 已实现：

- `sdk` C++ SDK 对象库与 `libAPlaySdk.so` 可编译入口。
- `sdk` Java SDK `APlaySdk` AAR 可编译入口。
- `sdk` ETS SDK/Harmony HAR module 入口及 NAPI native module 类型声明包。
- `app/android` 可编译入口通过 `:APlaySdk` 使用 SDK。
- `app/harmony` 可导入 DevEco Studio，并通过本地 ETS SDK HAR 使用 SDK facade。
- mDNS replay fixture summary。
- mDNS announce example 可编译为手动验证二进制。

非 mDNS 的协议、加密、RTP 与运行时验证还没有实现，不在 `example` 中保留占位示例。

## BLE TODO

BLE beacon 的抓包和广播验证不进入首版 harness。后续添加：

- BLE advertisement fixture。
- beacon payload encoder/decoder vectors。
- 平台能力探测。
