# APlay Harness Design

## 目标

harness 是 APlay 的实现、验收、回归和完善入口。它必须让协议、加密、RTP 和运行时在没有真实 Apple 设备的情况下被验证，同时保留真机抓包集成入口。

## 工程结构

- `harness/smoke_main.cpp`
  验证 `app/linux` runtime lifecycle；后续增加 `app/android` host stub lifecycle 验收。
- `harness/protocol_replay.cpp`
  回放 RTSP/HTTP fixture，输出 JSON summary。
- `harness/rtp_replay.cpp`
  回放 RTP CSV/pcap 派生 fixture，检查 seq、timestamp、gap。
- `harness/crypto_vectors.cpp`
  Phase 0 为占位 vector，后续替换为 AES/FairPlay/SRP/Ed25519 vectors。
- `harness/fixtures/`
  存放脱敏请求、RTP 摘要、golden 输入。

## 验收类型

### Syntax

- CMake configure。
- C++ compile。
- CTest discovery。

### Runtime

- `aplay --help`。
- `aplay --smoke-run`。
- `app/android` host stub 可构建。
- 后续扩展为端口监听、请求响应、退出清理。

### Protocol Logic

- RTSP request line。
- headers。
- CSeq。
- plist body。
- state transition。
- response status/header/body。

### Crypto

- AES known answer tests。
- FairPlay setup/handshake fixtures。
- pairing session fixtures。
- digest auth fixtures。

### RTP and Timing

- audio RTP seq/timestamp。
- mirror frame boundary。
- jitter buffer reorder。
- gap detection。
- resend request。
- NTP clock mapping。

### Packet Capture

- pcap 原始文件不直接作为唯一 golden。
- 通过解析脚本生成稳定 JSON summary。
- summary 与 golden 比对。
- 真机抓包作为增强验收，不替代离线门禁。

## 当前 Phase 0 行为

当前 harness 已实现：

- `app/linux` runtime smoke。
- `app/android` 可编译入口。
- RTSP OPTIONS fixture summary。
- RTP CSV gap summary。
- crypto placeholder vector。

这些不是最终协议实现，只是固定工程入口和 CTest 门禁。后续阶段必须在不改变入口语义的前提下替换内部逻辑。

## BLE TODO

BLE beacon 的抓包和广播验证不进入首版 harness。后续添加：

- BLE advertisement fixture。
- beacon payload encoder/decoder vectors。
- 平台能力探测。
