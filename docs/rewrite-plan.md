# APlay Rewrite Plan

## Phase 0: 文档与工程基线

目标：

- 生成架构和详细设计文档。
- 建立 PlantUML 图源和渲染产物。
- 建立 CMake 工程、`aplay` 最小运行入口和 harness 目标。
- 建立 CTest 门禁。

验收：

- `cmake -S . -B /tmp/aplay-build -G Ninja`
- `cmake --build /tmp/aplay-build`
- `ctest --test-dir /tmp/aplay-build --output-on-failure`
- `aplay --help`
- `aplay --smoke-run 100`

## Phase 1: App 平台层与 OSAL 边界

目标：

- 完成 `app/linux` 的 CLI、Config、Logger、进程生命周期。
- 建立 `app/android` 的 Activity/Service/Foreground Service 设计与可编译入口。
- 完成 Linux OSAL：socket、thread、timer、clock、file、network interface、GStreamer render/codec 适配接口。
- Android OSAL 提供可编译 stub：socket/thread/timer/file/network interface、MediaCodec/AudioTrack/Surface render/codec 接口占位。
- 建立 service lifecycle：start、stop、reset、signal cleanup。

验收：

- Linux 服务可启动监听本地端口。
- smoke harness 能检查端口、发送最小请求、干净退出。
- Android app/OSAL 目标至少通过 host stub 编译；正式 Android 构建后续接 Gradle/NDK。

## Phase 2: RTSP/HTTP 协议核心

目标：

- 实现 request/response 模型。
- 实现 RTSP OPTIONS、GET /info、POST /pair-*、POST /fp-setup、SETUP、RECORD、FLUSH、TEARDOWN 骨架。
- 实现 HTTP /server-info、/reverse、/playback-info 骨架。
- 实现连接类型分类和 session 状态机。

验收：

- fixture replay 覆盖正常、未知请求、非法 CSeq、过长 header。
- golden summary 与 UxPlay 抓包语义一致。

## Phase 3: Crypto 与 Pairing

目标：

- 迁移 AES、Ed25519/X25519、SRP、FairPlay、digest auth。
- 所有 crypto 能力脱离网络单测。
- 生成固定 vectors 和 UxPlay 对照 fixtures。

验收：

- crypto vectors 全部通过。
- pairing/fp-setup fixture 输出与 golden 一致。

## Phase 4: RTP、Mirror 与 Timing

目标：

- 实现 RAOP audio RTP。
- 实现 mirror video stream。
- 实现 jitter buffer、flush、gap detection、resend request。
- 实现 NTP/timing 同步。

验收：

- pcap/fixtures 离线回放输出 audio/video frame summary。
- gap、reorder、flush、reset 场景通过。
- mock renderer 收到稳定事件序列。

## Phase 5: Linux 真实渲染与端到端运行

目标：

- 通过 Linux OSAL 接入 GStreamer audio/video render/codec。
- 实现 OSAL render/codec lifecycle 与协议 reset 解耦。
- 支持真实 iPhone/iPad/macOS 客户端可选集成验收。

验收：

- Linux 可编译、可运行。
- 真机连接 smoke 通过。
- 抓包与日志可关联到 session id、stream id、timestamp。

## Phase 6: 后续功能

目标：

- HLS 完整支持。
- mp4 mux/recording。
- DBus screensaver inhibition，归属 `app/linux` 桌面集成策略。
- Android 真实 `app/android`、Android OSAL 和 MediaCodec/AudioTrack/Surface render/codec。
- BLE beacon service discovery TODO。

验收：

- 每项功能独立 feature flag、独立 harness、独立 golden。

## 风险

- AirPlay legacy 协议依赖非公开行为，必须以 UxPlay 和抓包为行为基线。
- FairPlay/Pairing 迁移风险高，必须先固定 vectors。
- GStreamer 生命周期容易与协议 reset 耦合，必须通过 OSAL render/codec 抽象隔离。
- 真机验收不可完全自动化，因此离线 pcap/fixtures 是硬性门禁。
