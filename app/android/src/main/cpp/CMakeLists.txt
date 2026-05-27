cmake_minimum_required(VERSION 3.20)

project(APlayAndroidNative LANGUAGES CXX)

get_filename_component(APLAY_REPO_ROOT
    "${CMAKE_CURRENT_LIST_DIR}/../../../../.."
    ABSOLUTE)

if(NOT CMAKE_SOURCE_DIR STREQUAL APLAY_REPO_ROOT)
    set(APLAY_ANDROID_NATIVE_ENTRY_INCLUDED ON CACHE BOOL "" FORCE)
    set(APLAY_BUILD_LINUX_APP OFF CACHE BOOL "" FORCE)
    set(APLAY_BUILD_HARNESS OFF CACHE BOOL "" FORCE)
    set(APLAY_BUILD_ANDROID_NATIVE ON CACHE BOOL "" FORCE)
    add_subdirectory(${APLAY_REPO_ROOT} ${CMAKE_BINARY_DIR}/aplay_native_root)
endif()

add_library(aplay_android_native SHARED
    native_entry.cpp)

target_include_directories(aplay_android_native
    PRIVATE
        ${APLAY_REPO_ROOT}/include)

target_compile_features(aplay_android_native PRIVATE cxx_std_17)
