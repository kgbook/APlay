# APlay Harness Design

## 目标

harness 是 APlay 的实现、验收、回归和完善入口。当前验证已经实现的 mDNS responder，并把真实网络抓包纳入 Linux 自动化验收。
harness 由 agent/CI 调用；mDNS announce/replay 验证工具位于 `harness/mdns`，`example` 目录仅保留面向人工体验的示例。

## 职责边界

- `harness/`
  只放 agent/CI 使用的验证编排：构建命令、目标选择、fixture/resource 路径、执行顺序和 pass/fail 判断。harness 不作为人手动体验功能的入口。
- `harness/mdns/`
  放 mDNS validation utilities，包括 announce 广播工具、replay 分析工具和对应 CMake 目标。
- `example/`
  放人可以直接运行的示例源码和可执行目标，用于手动确认已经实现的能力。example 只展示当前真实可用的功能；验证专用工具不放在 example 中，未实现的协议、加密、RTP、BLE 等能力不放占位示例。
- `resources/`
  放跨验证入口共享的外部输入，例如真实抓包。当前 Linux harness 默认把实时 mDNS 抓包写入 `resources/pcap/mdns_announce.pcapng`，再交给 replay 分析。

## 工程结构

- `harness/verify_mdns.sh`
  agent/CI 调用入口；构建 Linux SDK、APlayReceiver 和 mDNS harness 二进制，启动 live mDNS 抓包，运行 announce 广播，再用 replay 自动分析抓包。
- `harness/mdns/mdns_replay.cpp`
  离线验证 mDNS responder 生成的 PTR/SRV/TXT/A/AAAA/goodbye 记录，并扫描 pcap 中的 IPv4/IPv6 mDNS multicast 与 DNS 编码服务名，确认抓到预期 receiver 的 AirPlay/RAOP announcement。
- `harness/mdns/mdns_announce.cpp`
  默认启动 POSIX mDNS responder 并持续发送 announcement 广播包，直到收到 `SIGINT` 或 `SIGTERM`；加 `--once` 后只离线生成并解析 packet。

## 验收类型

### Syntax

- CMake configure。
- C++ compile。
- harness 脚本可重复执行。

### Runtime

- `APlayReceiver` 不要求传入 options，应可直接启动。
- `aplay_cpp_sdk` 作为 SDK facade 复用 core 导出的公共 runtime，runtime 实现位于 `sdk/src/main/cpp/core/runtime`。
- `aplay_sdk` 可构建并输出 `libAPlaySdk.so`。
- `aplay_jni` 可作为 Java SDK native binding 构建，并为 Android 打包输出 `libAPlaySdk.so`。
- `aplay_napi` 保留 ETS SDK native binding 条件编译入口，并由 Harmony HAR native 构建启用。
- `sdk/src/main/ets/libaplay_napi` 提供 `libaplay_napi.so` 的 Harmony native module 类型声明包。
- `:APlaySdk` 可构建 Java SDK 并输出 `APlaySdk` Android AAR。
- `sdk` 提供 Harmony HAR module 配置，`sdk/src/main/ets/com/kgbook/aplay` 提供 ETS SDK facade。
- `app/android` host stub 可通过 Java SDK 构建并输出 `APlayReceiver` APK。
- 后续扩展为端口监听、请求响应、退出清理。

### mDNS

- mDNS PTR/SRV/TXT/A/AAAA announcement。
- responder 启动时同时尝试 IPv4 `224.0.0.251:5353` 与 IPv6 `ff02::fb:5353`，至少一个 listener 成功即可运行；具备双栈条件的 harness capture 应同时可观测 IPv4/IPv6。
- mDNS goodbye 记录。
- AirPlay/RAOP PTR query response。
- `resources/pcap/mdns_announce.pcapng` live capture 中包含当前 announce receiver 的 AirPlay/RAOP DNS-SD 名称、host A/AAAA 记录以及 IPv4/IPv6 mDNS multicast。
- pcap 原始文件不直接作为唯一 golden；live capture 用于验证真实 UDP 5353 multicast 可观测，replay 仍负责协议语义判定。

## 当前 Phase 0 行为

当前 harness 已实现：

- `sdk` C++ SDK facade 与 `libAPlaySdk.so` 可编译入口。
- `sdk` Java SDK `APlaySdk` AAR 可编译入口。
- `sdk` ETS SDK/Harmony HAR module 入口及 NAPI native module 类型声明包。
- `app/android` 可编译入口通过 `:APlaySdk` 使用 SDK。
- `app/harmony` 可导入 DevEco Studio，并通过本地 ETS SDK HAR 使用 SDK facade。
- mDNS harness announce 工具默认持续广播 IPv4/IPv6 AirPlay/RAOP announcement，可通过 `--once` 做离线 packet 自检。
- Linux harness 默认把实时抓包保存到 `resources/pcap/mdns_announce.pcapng`。
- mDNS replay 可分析 live capture，并按 receiver name/device id/host name 校验 AirPlay/RAOP DNS-SD 名称和 host A/AAAA 记录。

非 mDNS 的协议、加密、RTP 与运行时验证还没有实现，不在 `example` 中保留占位示例。

## BLE TODO

BLE beacon 的抓包和广播验证不进入首版 harness。后续添加：

- BLE advertisement fixture。
- beacon payload encoder/decoder vectors。
- 平台能力探测。
