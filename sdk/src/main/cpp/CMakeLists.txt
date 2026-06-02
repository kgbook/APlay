cmake_minimum_required(VERSION 3.20)

project(APlayNative LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(APLAY_BUILD_HARNESS "Build APlay validation harness tools" OFF)
option(APLAY_BUILD_LINUX "Build Linux platform native modules" OFF)
option(APLAY_BUILD_ANDROID "Build Android platform native modules and JNI binding" OFF)
option(APLAY_BUILD_HARMONY "Build Harmony platform native modules and NAPI binding" OFF)

set(APLAY_NATIVE_MODULES
    core
    utils
    flow
    crypto
    protocol
    streaming)

foreach(APLAY_NATIVE_MODULE IN LISTS APLAY_NATIVE_MODULES)
    add_subdirectory(${APLAY_NATIVE_MODULE})
endforeach()

add_library(aplay_cpp_sdk OBJECT
    cpp_sdk.cpp)

target_include_directories(aplay_cpp_sdk
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(aplay_cpp_sdk PUBLIC cxx_std_17)

target_link_libraries(aplay_cpp_sdk
    PUBLIC
        aplay_core)

set_target_properties(aplay_cpp_sdk PROPERTIES
    POSITION_INDEPENDENT_CODE ON)

if(APLAY_BUILD_LINUX)
    add_library(aplay_sdk SHARED)

    target_link_libraries(aplay_sdk
        PRIVATE
            aplay_cpp_sdk)

    set_target_properties(aplay_sdk PROPERTIES
        OUTPUT_NAME APlaySdk)
endif()

add_subdirectory(osal)
