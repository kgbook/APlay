# APlay

APlay 是面向 UxPlay 兼容 AirPlay 接收端的分期重写工程。

当前状态：Phase 0 工程基线。

- Linux 由仓库根目录 `CMakeLists.txt` 作为入口构建，并进入 `app/linux`。
- Android 是独立 Gradle Java 工程，入口位于 `app/android`。
- 共享 C/C++ CMake 工程作为 Android native 依赖导入。
- OSAL 只作为codec、render等平台能力抽象层。
- 架构、详细设计、重写计划和 harness 设计位于 `docs/`。
- BLE 服务发现首版暂不实现，作为 TODO 保留。

## Linux 构建

根目录 `CMakeLists.txt` 是 Linux 构建入口，负责配置公共模块并构建 `app/linux`。

```sh
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

```sh
./build/aplay --help
./build/aplay --smoke-run 100
```

## Android 构建

`app/android` 是 Android Java Gradle 工程入口。Native 代码遵循 Android 传统目录 `app/android/src/main/cpp`；该目录下的 CMake 入口会导入仓库根目录 CMake 工程作为 native 依赖，并关闭 Linux app 与 harness。

```sh
cd app/android
./gradlew assembleDebug
```

Android 构建入口：

- Gradle 入口：`app/android`
- Gradle Wrapper：`app/android/gradlew`，使用 Gradle 8.9
- Native CMake 入口：`app/android/src/main/cpp/CMakeLists.txt`
- 导入的 native 根工程：仓库根目录 `CMakeLists.txt`

Android native 构建传入导入根工程的 CMake 参数：

```text
-DAPLAY_BUILD_LINUX_APP=OFF
-DAPLAY_BUILD_HARNESS=OFF
-DAPLAY_BUILD_ANDROID_NATIVE=ON
```

## 子模块

```sh
git submodule update --init --recursive
```
