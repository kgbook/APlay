cmake_minimum_required(VERSION 3.20)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(APlayNative LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

option(APLAY_BUILD_HARNESS "Build APlay validation harness tools" OFF)
option(APLAY_BUILD_LINUX_APP "Build Linux app entrypoint" OFF)
option(APLAY_BUILD_ANDROID_NATIVE "Build Android native library entrypoint" OFF)
option(APLAY_BUILD_JNI_BINDING "Build Java SDK JNI binding library" OFF)
option(APLAY_BUILD_NAPI_BINDING "Build ETS SDK NAPI binding library" OFF)

if(APLAY_BUILD_ANDROID_NATIVE)
    set(APLAY_BUILD_JNI_BINDING ON CACHE BOOL "" FORCE)
endif()

set(APLAY_NATIVE_MODULES
    core
    utils
    flow
    crypto
    protocol
    streaming
    osal)

foreach(APLAY_NATIVE_MODULE IN LISTS APLAY_NATIVE_MODULES)
    add_subdirectory(${APLAY_NATIVE_MODULE})
endforeach()

add_library(aplay_cpp_sdk SHARED
    cpp_sdk.cpp)

target_include_directories(aplay_cpp_sdk
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(aplay_cpp_sdk PRIVATE cxx_std_17)

target_link_libraries(aplay_cpp_sdk
    PRIVATE
        aplay_core)

set_target_properties(aplay_cpp_sdk PROPERTIES
    OUTPUT_NAME aplay_cpp_sdk)

if(APLAY_BUILD_JNI_BINDING)
    add_subdirectory(jni)
endif()

if(APLAY_BUILD_NAPI_BINDING)
    add_subdirectory(napi)
endif()
