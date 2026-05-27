# APlay Software Design

## 设计原则

- 协议解析与业务状态分离。
- C++ 对象拥有明确生命周期，禁止裸全局状态承载会话逻辑。
- 加密和 RTP 处理可被 harness 离线调用。
- Linux/Android 的 UI 与生命周期差异存在于 `app/linux` 和 `app/android`。
- `osal` 只抽象平台能力，包括 codec、render、socket、thread、timer、file、network interface。
- 每个阶段都必须可编译、可运行、可测试。

## 主要对象

### LinuxAppRuntime

职责：

- 解析 Linux CLI/config。
- 管理 Linux 进程生命周期、信号、退出清理、桌面集成策略。
- 创建 logger、OSAL、protocol server、stream manager。
- 将 CLI/配置转换为服务启动参数。

边界：

- 不直接实现 codec/render。
- 不直接解析 RTSP/RTP。
- 通过 OSAL 使用 Linux GStreamer render/codec 能力。

### AndroidAppRuntime

职责：

- 管理 Android Activity/Service/Foreground Service 生命周期。
- 处理权限、通知、网络状态、投屏状态展示和用户控制。
- 创建 Android OSAL、protocol server、stream manager。
- 将 Android UI/Service 事件转换为服务启动、停止、重置动作。

边界：

- 不直接实现 MediaCodec、AudioTrack、Surface 渲染细节。
- 不直接解析 RTSP/RTP。
- 通过 OSAL 使用 Android codec/render 能力。

### ProtocolServer

职责：

- TCP accept 和连接管理。
- HTTP/RTSP parser 封装。
- 连接类型识别：RAOP、AirPlay、reverse HTTP、HLS。
- 将请求转交给 `SessionController`。

关键规则：

- Header 数量、字段长度和值长度必须有限制。
- CSeq 只接受非负整数并原样规范化返回。
- 未识别请求返回明确错误或记录 unhandled，不允许静默改变会话状态。

### SessionController

职责：

- 管理单个 client session。
- 实现 pairing、FairPlay、SETUP、RECORD、FLUSH、TEARDOWN 状态机。
- 创建或销毁 RTP、mirror、timing、HLS 子会话。

状态：

- `Idle`
- `Pairing`
- `Verified`
- `Configured`
- `Streaming`
- `Flushing`
- `TearingDown`
- `Closed`
- `Error`

### CryptoProvider

职责：

- AES CTR/CBC/GCM。
- Ed25519/X25519。
- SRP。
- FairPlay setup/handshake/decrypt。
- digest auth。

接口必须支持固定测试向量，不依赖网络和 renderer。

### StreamingManager

职责：

- 音频 RTP packet parse、decrypt、jitter buffer、resend request。
- 镜像视频 packet parse、decrypt、H264/H265 NAL 输出。
- NTP/timing 同步。
- flush/reset 事件传播。

输出统一为 typed frame：

- `AudioFrame`
- `VideoFrame`
- `MetadataFrame`
- `CoverArtFrame`
- `PlaybackEvent`

### OsalCodecRender

职责：

- 提供 codec/render 能力抽象。
- Linux 实现使用 GStreamer。
- Android 实现使用 MediaCodec、AudioTrack、Surface。
- harness 实现使用 mock render/codec，记录事件并生成 golden summary。

OSAL render/codec 不参与协议决策，只接受 start/stop/pause/resume/flush/frame 事件。

## 错误处理

- 参数错误：返回 usage error。
- 协议错误：生成可解释响应并记录 request summary。
- 加密错误：session 进入 `Error`，关闭相关通道。
- RTP 缺包：记录 gap，触发 resend 或在 harness summary 中暴露。
- Renderer 错误：向 Runtime 上报，Runtime 决定 reset 或退出。

## App 与 OSAL 平台边界

`app/linux` 负责：

- CLI。
- rc/config 文件。
- signal handler。
- Linux 桌面集成开关，例如 DBus screensaver inhibition。
- Linux 服务进程生命周期。

`app/android` 负责：

- Activity/Service/Foreground Service。
- 通知栏和权限流程。
- Android 网络状态监听。
- Android 媒体会话和 UI 状态展示。

`osal` 负责：

- `Socket`
- `TcpListener`
- `UdpSocket`
- `Thread`
- `Timer`
- `Clock`
- `FileSystem`
- `NetworkInterfaceProvider`
- `CodecFactory`
- `RenderSinkFactory`

Linux OSAL 首先实现真实 socket/thread/timer/GStreamer。Android OSAL 首版只要求接口稳定和 stub 可编译，后续接入 MediaCodec/AudioTrack/Surface。

## BLE TODO

BLE beacon 不进入首版实现。设计保留 `DiscoveryPublisher` 接口：

- `MdnsDiscoveryPublisher` 首版实现。
- `BleDiscoveryPublisher` 后续 TODO。

接口预留但 CMake 默认不构建 BLE 相关目标。
