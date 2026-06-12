# APlay Architecture

## 背景

APlay 目标是以现代 C++ 分期实现 AirPlay 接收能力。APlay 首版以 Linux 可运行为硬门槛，同时保留共享 SDK、Android app、Harmony 大屏终端入口和 OSAL 适配边界。

## Legacy Baseline

历史基线实现包含以下能力，需要在 APlay 中按模块重新实现：

- `uxplay.cpp`：CLI、全局配置、生命周期、回调桥接、GStreamer/DBus 集成。
- `lib/raop.c`：连接分类、RTSP/HTTP/HLS 分发、RAOP 会话上下文。
- `lib/raop_handlers.h`：RTSP 处理器，包括 pairing、fp-setup、SETUP、RECORD、FLUSH、TEARDOWN。
- `lib/http_handlers.h`：AirPlay HTTP/HLS 处理器，包括 reverse HTTP、playback-info、HLS playlist。
- `lib/raop_rtp.c`：音频 RTP、重传、flush、metadata、coverart。
- `lib/raop_rtp_mirror.c`：镜像视频 TCP/RTP、解密、NAL 输出。
- `lib/raop_ntp.c`：NTP/timing 同步。
- `lib/dnssd.c`、`lib/mdnsd.c`：内置 mDNS/DNS-SD。
- `lib/crypto.c`、`lib/pairing.c`、`lib/srp.c`、`lib/fairplay_playfair.c`：加密、配对和认证。
- `renderers/*.c`：GStreamer 音频、视频、mp4 mux。

主要问题是全局状态多、协议分发与业务状态耦合、渲染生命周期与协议回调强绑定、测试入口缺失。

## APlay 目标分层

APlay 采用边界清晰的分层架构：

- `app/linux`：Linux 平台 UI/业务逻辑，包括 CLI、配置文件、服务启动/停止、信号处理、桌面集成策略。
- `sdk`：共享 SDK module。`sdk/src/main/cpp` 提供 C++ SDK，Linux 输出 `libaplay-sdk.so`；`sdk/src/main/cpp/osal/android/jni` 提供 Java SDK native binding 并为 Android 打包输出 `libaplay-sdk.so`；`sdk/src/main/cpp/osal/harmony/napi` 提供 ETS SDK native binding，`sdk/src/main/java` 提供 Java SDK 并对外输出 `aplay-sdk` Android AAR，`sdk` 根目录提供本地 `aplay-sdk` Harmony HAR module 配置，`sdk/src/main/ets/com/kgbook/aplay` 提供 ETS SDK 门面，`sdk/src/main/ets/libaplay_napi` 提供 Harmony native module 类型声明包。
- `app/android`：Android 平台 UI/业务逻辑，包括前台服务、Activity/Service 生命周期、权限申请、网络/投屏状态展示、Android 媒体会话集成；Gradle root 命名为 `APlayReceiver`，输出 `APlayReceiver` APK。它通过 `:aplay-sdk` 使用 Java SDK，不直接持有 C/C++ 代码。
- `app/harmony`：HarmonyOS/DevEco Studio 导入入口，构建 `APlayReceiver` HAP target，依赖 `sdk` 本地 `aplay-sdk` ETS SDK HAR target；电视、机顶盒等大屏业务适配仍为 TODO。
- `protocol`：RTSP、HTTP、reverse HTTP、mDNS/DNS-SD、请求/响应模型和 httpd 等协议编解码基础能力。
- `streaming/connection`：连接首包识别、HTTP/HLS/RTSP 分类和分发，不承载具体音频或视频流处理。
- `streaming/airplay`：AirPlay 视频镜像数据流处理，包括 mirror TCP/RTP、解密后的视频 NAL 输出和后续视频渲染衔接。
- `streaming/raop`：RAOP 音频投屏数据流处理，包括音频 RTP、控制/重传、metadata、cover art 和后续音频渲染衔接。
- `streaming`：聚合 connection、airplay、raop、HLS、jitter buffer、NTP 时间同步等流媒体子模块。
- `crypto`：AES、FairPlay、Ed25519/X25519、SRP、digest auth、hash/base64。
- `core`：通用可复用 C/C++ 实现，承载 STL-based 的跨平台辅助能力，例如 socket/thread/poll，并在 `core/pattern` 承载可复用设计模式模板；不再单独设置 `utils` 模块。
- `osal`：平台能力抽象层，当前负责 codec/render 平台模块和平台 native binding 子模块选择，不承载 UI 或业务流程。

关键边界：

- `app/*` 可以依赖 SDK 或 runtime facade，负责把平台 UI/生命周期事件转换成服务控制动作。
- C++ SDK 位于 `sdk/src/main/cpp`，Linux app、Android Java SDK AAR 和 Harmony ETS SDK HAR 都从这里复用 native 能力。
- JNI 与 NAPI 是独立 C++ binding 子模块，通过 CMake option 条件编译，不把 Java/ETS 绑定逻辑混入核心 C++ SDK。
- Harmony NAPI `.so` 的 ArkTS 可见接口由 `sdk/src/main/ets/<library>` 下的 native module `.d.ts` 包声明，并通过 HAR module 的 `oh-package.json5` 引用。
- `protocol`、`streaming`、`crypto` 不依赖 `app/*`，保证协议和流媒体核心跨平台。
- `osal` 不主动调用业务逻辑，只提供平台能力；当前落地范围是 codec/render 和平台 native binding，Linux 的 GStreamer render/codec 与 Android 的 MediaCodec/AudioTrack/Surface 属于后续 OSAL 实现方向。

## 核心数据流

1. Client 通过 mDNS/DNS-SD 发现 APlay。
2. Client 建立 RTSP/HTTP 连接。
3. `protocol/http` 或 `protocol/rtsp` 完成请求编解码，`streaming/connection` 将连接分类为 HTTP、HLS 或 RTSP 并分发。
4. Pairing/FairPlay 生成会话密钥。
5. SETUP 建立音频 RTP、镜像视频或 HLS 通道。
6. `streaming/raop` 处理音频流，`streaming/airplay` 处理视频镜像流，其他连接控制面由 `streaming/connection` 分发到对应后续模块。
7. `streaming` 通过 `osal` 的 render/codec 抽象解码并渲染显示媒体帧。

## 线程模型

首版线程模型保持简单：

- Main thread：CLI、配置、生命周期、信号处理。
- Protocol IO thread：TCP accept、request parse、response write。
- RTP audio thread：UDP audio/control 收包、重传、jitter buffer。
- Mirror video thread：TCP mirror stream 收包、解密、NAL 输出。
- Timing thread：NTP/timing 同步。
- Render/codec thread：由 OSAL 平台实现管理，Linux 上交给 GStreamer main loop，Android 上交给 MediaCodec/AudioTrack/Surface 所在线程。

所有跨线程事件统一通过 typed callback 或 queue 传递，禁止模块直接访问对方内部状态。

## 首版非目标

- BLE service discovery：TODO。
- 完整 HLS：后续阶段。
- mp4 录制：后续阶段。
- Android 完整 UI：后续阶段，首版保留 Java SDK AAR 与 `app/android` 可编译入口。
- Harmony 大屏业务：TODO；首版保留可导入 DevEco Studio 的 `app/harmony` HAP、ETS SDK HAR facade、NAPI native binding 构建入口和对应 native module 类型声明。
