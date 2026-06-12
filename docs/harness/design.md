# APlay Harness Design

## 目标

harness 是 APlay 的实现、验收、回归和完善入口。当前验证已经实现的 mDNS responder，并把真实网络抓包纳入 Linux 自动化验收。
harness 脚本由 agent/CI 调用并位于 `scripts/harness`；mDNS announce/replay harness 入口和内部验证代码位于 `sdk/src/main/cpp/protocol/mdns`，`example` 目录仅保留面向人工体验的示例。

## 职责边界

- `scripts/harness/`
  只放 agent/CI 使用的验证编排脚本：构建命令、目标选择、fixture/resource 路径、执行顺序和 pass/fail 判断。harness 不作为人手动体验功能的入口。
- `scripts/app/`
  放 app 构建脚本，例如 Linux、Android 和 Harmony 构建入口，和 harness 验证脚本分离。
- `sdk/src/main/cpp/protocol/mdns/harness/`
  放 mDNS harness 可执行入口。
- `sdk/src/main/cpp/protocol/mdns/src/`
  放 mDNS 模块内部 harness 验证实现，并通过 `APLAY_MDNS_HARNESS` 编译宏条件编译。
- `example/`
  放人可以直接运行的示例源码和可执行目标，用于手动确认已经实现的能力。example 只展示当前真实可用的功能；验证专用工具不放在 example 中，未实现的协议、加密、RTP、BLE 等能力不放占位示例。
- `resources/`
  放跨验证入口共享的外部输入，例如真实抓包。当前 Linux harness 默认把实时 mDNS 抓包写入 `resources/pcap/mdns_announce.pcapng`，再交给 replay 分析。

## 工程结构

- `scripts/harness/verify_mdns.sh`
  agent/CI 调用入口；构建 Linux SDK、APlayReceiver 和 mDNS harness 二进制，启动 live mDNS 抓包，运行 announce 广播，再用 replay 自动分析抓包。
- `resources/pcap/mdns_verify_progress.md`
  agent/CI 进度快照；每个阶段记录 `TODO`、`DOING` 或 `DONE`、progress、任务背景、目标、当前进度、验证和修改。
- `resources/pcap/mdns_verify_result.txt`
  agent/CI pass/fail 结果；失败时保留当前阶段为 `DOING`，用于继续修正直到验收通过。

## 验收类型

### Syntax

- CMake configure。
- C++ compile。
- harness 脚本可重复执行。

### Runtime

- `aplay_cpp_sdk` 作为 SDK facade 复用 core 导出的公共 runtime，runtime 实现位于 `sdk/src/main/cpp/core/runtime`。
- `aplay_sdk` 可构建并输出 `libaplay_sdk.so`。
- `:aplay-sdk` 可构建 Java SDK 并输出 `aplay-sdk` Android AAR。
- `sdk` 提供 Harmony HAR module 配置，`sdk/src/main/ets/com/kgbook/aplay` 提供 ETS SDK facade。
- `app/android` host stub 可通过 Java SDK 构建并输出 `APlayReceiver` APK。
- 后续扩展为端口监听、请求响应、退出清理。

### mDNS

- mDNS PTR/SRV/TXT/A/AAAA announcement，其中 IPv4 packet 只携带 host A，IPv6 packet 只携带 host AAAA。
- responder 启动时同时尝试 IPv4 `224.0.0.251:5353` 与 IPv6 `ff02::fb:5353`，至少一个 listener 成功即可运行；具备双栈条件的 harness capture 应同时可观测 IPv4/IPv6。
- mDNS goodbye 记录。
- AirPlay/RAOP PTR query response。
- `resources/pcap/mdns_announce.pcapng` live capture 中包含当前 announce receiver 的 AirPlay/RAOP DNS-SD 名称、按 family 发布的 host A/AAAA 记录以及 IPv4/IPv6 mDNS multicast。
- pcap 原始文件不直接作为唯一 golden；live capture 用于验证真实 UDP 5353 multicast 可观测，replay 仍负责协议语义判定。

## Harness Task Discipline

- 开始实现前先写验收规则和 harness 工程记录，避免边写边扩大范围。
- 每个阶段都必须有任务背景、目标、当前进度、验证和修改记录。
- 遵循 Karpathy 四项原则：
  Think Before Coding 先明确假设和验收；
  Simplicity First 只做当前验收需要的最小实现；
  Surgical Changes 只改 harness 与对应模块内部验证代码；
  Goal-Driven Execution 失败后继续修正直到对应验证通过。
- 需要访问模块内部细节的 harness 实现必须放回 owning module 内部，并通过编译宏条件编译；harness 入口只保留窄调用，不直接包含 responder、packet、service 等内部实现头。

## 当前 Phase 0 行为

当前 harness 已实现：

- `sdk` C++ SDK facade 与 `libaplay_sdk.so` 可编译入口。
- `sdk` Java SDK `aplay-sdk` AAR 可编译入口。
- `app/android` 可编译入口通过 `:aplay-sdk` 使用 SDK。
- `app/harmony` 可导入 DevEco Studio，并通过本地 ETS SDK HAR 使用 SDK facade。
- mDNS harness announce 工具输出到 `build/linux/scripts/harness/mdns`，默认持续广播 IPv4/IPv6 AirPlay/RAOP announcement，可通过 `--once` 做离线 packet 自检。
- Linux harness 默认把实时抓包保存到 `resources/pcap/mdns_announce.pcapng`。
- mDNS replay 可分析 live capture，并按 receiver name/device id/host name 校验 AirPlay/RAOP DNS-SD 名称和按 family 发布的 host A/AAAA 记录。
- mDNS replay 的 packet/responder 自检实现位于 `sdk/src/main/cpp/protocol/mdns/src`，仅在 `APLAY_MDNS_HARNESS` 编译宏开启时参与构建。

非 mDNS 的协议、加密、RTP 与运行时验证还没有实现，不在 `example` 中保留占位示例。

## BLE TODO

BLE beacon 的抓包和广播验证不进入首版 harness。后续添加：

- BLE advertisement fixture。
- beacon payload encoder/decoder vectors。
- 平台能力探测。
