# APlay Architecture

## 背景

APlay 目标是以现代 C++ 分期重写 UxPlay 的 AirPlay 接收能力。UxPlay 当前可在本机完整编译，基线依赖包括 GStreamer、OpenSSL、libplist、DBus、X11、pthread、llhttp 和 playfair。APlay 首版以 Linux 可运行为硬门槛，同时保留共享 SDK、Android app、Harmony 大屏终端入口和 OSAL 适配边界。

BLE 服务发现不进入首版实现，记录为 TODO。

## UxPlay 现状

UxPlay 约 2 万行 C/C++：

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
- `sdk`：共享 SDK module。`sdk/src/main/cpp` 提供 C++ SDK 并对外输出 `.so`，`sdk/src/main/cpp/jni` 提供 Java SDK native binding，`sdk/src/main/cpp/napi` 提供 ETS SDK native binding，`sdk/src/main/java` 提供 Java SDK 并对外输出 Android AAR，`sdk/src/main/ets` 提供 ETS SDK 门面并输出本地 Harmony HAR module。
- `app/android`：Android 平台 UI/业务逻辑，包括前台服务、Activity/Service 生命周期、权限申请、网络/投屏状态展示、Android 媒体会话集成。它通过 `:aplay-sdk` 使用 Java SDK，不直接持有 C/C++ 代码。
- `app/harmony`：HarmonyOS/DevEco Studio 导入入口，构建 entry HAP，依赖 `sdk/src/main/ets` 本地 ETS SDK HAR；电视、机顶盒等大屏业务适配仍为 TODO。
- `protocol`：RTSP、HTTP、reverse HTTP、mDNS/DNS-SD、请求/响应模型和协议状态机。
- `streaming`：RAOP 音频 RTP、AirPlay mirror video、HLS、jitter buffer、NTP 时间同步。
- `crypto`：AES、FairPlay、Ed25519/X25519、SRP、digest auth、hash/base64。
- `osal`：平台能力抽象层，只负责 socket、thread、timer、file、network interface、codec、render 等能力接口和平台实现，不承载 UI 或业务流程。
- `harness`：离线回放、golden 对比、smoke run、真机增强验收。

关键边界：

- `app/*` 可以依赖 SDK 或 runtime facade，负责把平台 UI/生命周期事件转换成服务控制动作。
- C++ SDK 位于 `sdk/src/main/cpp`，Linux app、Android Java SDK AAR 和 Harmony ETS SDK HAR 都从这里复用 native 能力。
- JNI 与 NAPI 是独立 C++ binding 子模块，通过 CMake option 条件编译，不把 Java/ETS 绑定逻辑混入核心 C++ SDK。
- `protocol`、`streaming`、`crypto` 不依赖 `app/*`，保证协议核心可被 harness 离线驱动。
- `osal` 不主动调用业务逻辑，只提供平台能力；Linux 的 GStreamer render/codec 和 Android 的 MediaCodec/AudioTrack/Surface 都属于 OSAL 实现。
- `harness` 使用 mock OSAL 或 mock render/codec 能力验收协议和数据流，不依赖真实 UI。

## 核心数据流

1. Client 通过 mDNS/DNS-SD 发现 APlay。
2. Client 建立 RTSP/HTTP 连接。
3. `protocol` 将连接分类为 RAOP、AirPlay、reverse HTTP 或 HLS。
4. Pairing/FairPlay 生成会话密钥。
5. SETUP 建立音频 RTP、镜像视频或 HLS 通道。
6. `streaming` 解密、排序、同步并输出媒体帧。
7. `streaming` 通过 `osal` 的 render/codec 抽象输出媒体帧。
8. `harness` 可在无真机环境中回放相同步骤并比对结果。

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
- DBus screensaver inhibition：后续阶段。
- Android 完整 UI：后续阶段，首版保留 Java SDK AAR 与 `app/android` 可编译入口。
- Harmony 大屏业务：TODO；首版保留可导入 DevEco Studio 的 `app/harmony` HAP、ETS SDK HAR facade 和 NAPI native binding 构建入口。
