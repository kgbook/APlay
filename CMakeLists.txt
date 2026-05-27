cmake_minimum_required(VERSION 3.20)

project(APlay
    VERSION 0.1.0
    DESCRIPTION "C++ AirPlay receiver rewrite and validation harness"
    LANGUAGES CXX)

option(APLAY_BUILD_HARNESS "Build APlay validation harness tools" ON)
option(APLAY_BUILD_LINUX_APP "Build Linux app entrypoint" ON)
option(APLAY_BUILD_ANDROID_NATIVE "Build Android native library entrypoint" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(utils)
add_subdirectory(flow)
add_subdirectory(crypto)
add_subdirectory(protocol)
add_subdirectory(streaming)
add_subdirectory(osal)

if(APLAY_BUILD_LINUX_APP)
    add_subdirectory(app/linux)
endif()

if(APLAY_BUILD_ANDROID_NATIVE AND NOT APLAY_ANDROID_NATIVE_ENTRY_INCLUDED)
    add_subdirectory(app/android/src/main/cpp)
endif()

if(APLAY_BUILD_HARNESS)
    enable_testing()
    add_subdirectory(harness)
endif()
