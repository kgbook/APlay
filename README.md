# APlay

APlay is a staged rewrite target for a UxPlay-compatible AirPlay receiver.

Current status: phase 0 engineering baseline.

- Linux is built from the repository root CMake project and enters through `app/linux`.
- Android is a separate Gradle Java project rooted at `app/android`.
- The shared C/C++ CMake project is imported by Android as a native dependency.
- OSAL is only the platform abstraction layer for codec, render.
- Documents the target architecture and rewrite plan under `docs/`.
- BLE service discovery is intentionally deferred as a TODO.

## Linux Build

The root `CMakeLists.txt` is the Linux entrypoint. It configures shared modules and builds `app/linux`.

```sh
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

```sh
./build/aplay --help
./build/aplay --smoke-run 100
```

## Android Build

`app/android` is the Gradle entrypoint for the Android Java app. Native code follows the conventional Android path `app/android/src/main/cpp`; that CMake entry imports the repository root CMake project as a native dependency with Linux app and harness disabled.

```sh
cd app/android
./gradlew assembleDebug
```

The Android Gradle build uses:

- Gradle entrypoint: `app/android`
- Gradle wrapper: `app/android/gradlew` with Gradle 8.9
- Native CMake entrypoint: `app/android/src/main/cpp/CMakeLists.txt`
- Imported native root: repository root `CMakeLists.txt`

The Android native build passes these CMake options to the imported root:

```text
-DAPLAY_BUILD_LINUX_APP=OFF
-DAPLAY_BUILD_HARNESS=OFF
-DAPLAY_BUILD_ANDROID_NATIVE=ON
```

## Submodules

```sh
git submodule update --init --recursive
```
