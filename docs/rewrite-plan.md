# APlay Rewrite Plan

## Phase 0: 文档与工程基线

目标：

- 生成架构和详细设计文档。
- 建立 PlantUML 图源和渲染产物。
- 建立平台化构建入口、`aplay` 最小运行入口和 harness 目标。
- 建立 `sdk` module，作为 C++ `.so`、Java AAR、ETS HAR 和共享 C/C++ 代码归档位置。
- 建立 CTest 门禁。

验收：

- `cmake -S app/linux -B /tmp/aplay-build -G Ninja`
- `cmake --build /tmp/aplay-build`
- `ctest --test-dir /tmp/aplay-build --output-on-failure`
- `aplay --help`
- `aplay --smoke-run 100`

## Phase 1: App 平台层与 OSAL 边界

目标：

- 完成 `app/linux` 的 CLI、Config、Logger、进程生命周期。
- 建立 `sdk/src/main/cpp` C++ SDK 接口和 `.so` 输出。
- 建立 `sdk/src/main/cpp/osal/android/jni` Java SDK native binding 和条件编译。
- 建立 `sdk/src/main/cpp/osal/harmony/napi` ETS SDK native binding 占位和条件编译。
- 建立 `sdk/src/main/java` Java SDK 接口、JNI 入口和 Android AAR 构建。
- 建立 `sdk/src/main/ets` ETS SDK 接口和 Harmony HAR module 配置。
- 建立 `app/android` 的 Activity/Service/Foreground Service 设计与可编译入口，并通过 `:aplay-sdk` 使用 SDK。
- 建立 `app/harmony` DevEco Studio/HAP 入口；HarmonyOS 电视等大屏业务支持作为 TODO。
- 完成 Linux OSAL codec/render 层级，后续再接入 GStreamer。
- Android OSAL 提供可编译 codec/render 与 JNI 层级，后续再接入 MediaCodec/AudioTrack/Surface。
- `utils` 承担 STL-based 的跨平台辅助实现，例如 socket/thread/poll。
- 建立 service lifecycle：start、stop、reset、signal cleanup。

验收：

- Linux 服务可启动监听本地端口。
- smoke harness 能检查端口、发送最小请求、干净退出。
- C++ SDK `.so` 可由 CMake 构建。
- JNI binding `.so` 可由 Android AAR 构建。
- NAPI binding 保留 C++ 条件编译入口，由 Harmony HAR native 构建开启。
- Java SDK AAR 和 Android app 目标至少通过 Gradle/NDK 编译。
- ETS SDK/Harmony HAR 可作为本地 module 被 `app/harmony` 导入，不实现大屏业务。

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
- HarmonyOS TV/large-screen runtime 和平台 OSAL 适配 TODO。
- BLE beacon service discovery TODO。

验收：

- 每项功能独立 feature flag、独立 harness、独立 golden。

## 风险

- AirPlay legacy 协议依赖非公开行为，必须以 UxPlay 和抓包为行为基线。
- FairPlay/Pairing 迁移风险高，必须先固定 vectors。
- GStreamer 生命周期容易与协议 reset 耦合，必须通过 OSAL render/codec 抽象隔离。
- 真机验收不可完全自动化，因此离线 pcap/fixtures 是硬性门禁。
