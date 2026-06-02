# APlay Software Design

## 设计原则

- 协议解析与业务状态分离。
- C++ 对象拥有明确生命周期，禁止裸全局状态承载会话逻辑。
- 加密和 RTP 处理可被 harness 离线调用。
- Linux/Android/Harmony 的 UI 与生命周期差异存在于 `app/linux`、`app/android` 和后续 `app/harmony`。
- SDK facade 位于 `sdk`：C++ SDK 源码在 `sdk/src/main/cpp`，Linux 输出 `APlaySdk` 共享库，Android/Harmony 仅链接 `aplay_cpp_sdk` 对象库；JNI binding 在 `sdk/src/main/cpp/osal/android/jni` 并为 Android 打包输出 `libAPlaySdk.so`，NAPI binding 在 `sdk/src/main/cpp/osal/harmony/napi`，Harmony NAPI 类型声明包在 `sdk/src/main/ets/libaplay_napi`，Java SDK 在 `sdk/src/main/java` 并输出 `APlaySdk` AAR，ETS SDK facade 在 `sdk/src/main/ets/com/kgbook/aplay`，Harmony HAR module 配置位于 `sdk` 根目录并输出 `APlaySdk` HAR。
- `utils` 当前承载 STL-based 的跨平台辅助实现，例如 socket/thread/poll。
- `osal` 当前只落地 codec/render 平台模块和平台 native binding 子模块选择。`sdk/src/main/cpp/osal/CMakeLists.txt` 负责按 `APLAY_BUILD_LINUX`、`APLAY_BUILD_ANDROID`、`APLAY_BUILD_HARMONY` 选择当前平台子模块。
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
- 通过 `:APlaySdk` Java SDK module 创建或控制 native runtime。
- 将 Android UI/Service 事件转换为服务启动、停止、重置动作。

边界：

- 不直接实现 MediaCodec、AudioTrack、Surface 渲染细节。
- 不直接解析 RTSP/RTP。
- 不直接加载 native library，不持有 C/C++ 代码。
- 通过 SDK 和 OSAL 使用 Android codec/render 能力。

### CppSdk

职责：

- 提供 `sdk/src/main/cpp` 下的 C++ SDK API。
- 输出 `aplay_cpp_sdk` 对象库，被所有平台复用。
- Linux 额外输出 `APlaySdk` 共享库。
- 承载 protocol、streaming、crypto、osal 等共享 native 子模块。
- 顶层 CMake 只声明平台开关并在 C++ SDK 对象目标创建后进入 `osal`；平台 binding 子模块由 OSAL 平台目录添加。
- Android/Harmony 仅链接 `aplay_cpp_sdk` 对象库，不依赖 Linux `APlaySdk` 共享库。

边界：

- 不承载 Android 或 Harmony UI 生命周期。
- 不直接暴露 Java/ETS API。

### JniBinding

职责：

- 提供 `sdk/src/main/cpp/osal/android/jni` 下的 Java SDK native 接口。
- 输出 Android 打包用的 `libAPlaySdk.so`。
- 通过 Android 平台构建开关 `APLAY_BUILD_ANDROID` 条件编译。
- 只做 Java/JNI 与 C++ SDK 的参数和返回值桥接。

边界：

- 不承载 Android app 生命周期。
- 不实现协议、流媒体、渲染业务。

### NapiBinding

职责：

- 提供 `sdk/src/main/cpp/osal/harmony/napi` 下的 ETS SDK native 接口。
- 输出 `aplay_napi` `.so`。
- 通过 Harmony 平台构建开关 `APLAY_BUILD_HARMONY` 条件编译。
- 只做 ETS/NAPI 与 C++ SDK 的参数和返回值桥接。
- ArkTS 可见 API 必须在 `sdk/src/main/ets/libaplay_napi/index.d.ts` 中声明，并由同目录 `oh-package.json5` 以 `libaplay_napi.so` 包名暴露。

边界：

- 由 Harmony HAR native 构建入口启用。
- 不承载 Harmony app 生命周期。
- 不实现协议、流媒体、渲染业务。
- 不把 native module `.d.ts` 声明放在 ETS facade 源目录或 C++/OSAL 目录。

### JavaSdk

职责：

- 提供 `sdk/src/main/java` 下的 Android Java SDK API。
- 包装 JNI binding library 加载和平台 SDK facade。
- 对外输出 Android AAR。

边界：

- 不承载 Android Activity/Service/Foreground Service 业务生命周期。
- 不把 app UI 状态塞进 JNI 或 OSAL。

### EtsSdk

职责：

- 提供 `sdk/src/main/ets/com/kgbook/aplay` 下的 Harmony ETS SDK API。
- 包装 NAPI binding library。
- 对外输出本地 Harmony HAR module。
- 复用 `sdk/src/main/cpp` 的 C++ SDK。
- 通过 `sdk/oh-package.json5` 依赖 `sdk/src/main/ets/libaplay_napi` 中的 `libaplay_napi.so` 类型声明包。

边界：

- 不承载 Harmony 大屏 app 生命周期。
- 不把 Harmony UI 状态塞进 OSAL。

### HarmonyHarRuntime

职责：

- 预留 Harmony HAP/HAR API 和生命周期入口。
- TODO：面向 HarmonyOS TV、机顶盒等大屏终端适配。
- 通过 ETS SDK HAR 和 NAPI binding 复用 `sdk/src/main/cpp` 下 C++ SDK。

边界：

- 当前不实现 Harmony 业务逻辑。
- 不把 Harmony 生命周期逻辑放入 OSAL。

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
- 通过 Java SDK 调用 native 能力。

`sdk` 负责：

- C++ SDK API，Linux 输出 `libAPlaySdk.so`，Android/Harmony 输出 `aplay_cpp_sdk` 对象库。
- JNI binding `.so` 输出（Android 打包为 `libAPlaySdk.so`）。
- NAPI binding `.so` 输出（`aplay_napi`）。
- Harmony NAPI native module 类型声明包（`sdk/src/main/ets/libaplay_napi`）。
- Java SDK API 和 Android AAR 输出。
- ETS SDK API 和 Harmony HAR 输出。
- JNI/NAPI native 桥接。

`app/harmony` 负责：

- Harmony app/HAP 消费入口。
- TODO：HarmonyOS 大屏终端生命周期和 API 绑定。

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

Linux OSAL 当前先保留 codec/render 层级，后续再实现 GStreamer。Android OSAL 当前保留 codec/render 与 JNI 层级，后续接入 MediaCodec/AudioTrack/Surface。通用 socket/thread/poll 逻辑由 `utils` 承担，不进入 OSAL。

## BLE TODO

BLE beacon 不进入首版实现。设计保留 `DiscoveryPublisher` 接口：

- `MdnsDiscoveryPublisher` 首版实现。
- `BleDiscoveryPublisher` 后续 TODO。

接口预留但 CMake 默认不构建 BLE 相关目标。
