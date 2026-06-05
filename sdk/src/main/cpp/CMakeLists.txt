cmake_minimum_required(VERSION 3.20)

project(APlayNative LANGUAGES CXX)
get_filename_component(TOP_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)
set(CMAKE_HELPER_MODULE_DIR "${TOP_DIR}/cmake/CMakeHelper/cmake")
list(FIND CMAKE_MODULE_PATH "${CMAKE_HELPER_MODULE_DIR}" CMAKE_HELPER_MODULE_DIR_INDEX)
if(CMAKE_HELPER_MODULE_DIR_INDEX EQUAL -1)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_HELPER_MODULE_DIR}")
endif()
include(CMakeHelper)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(APLAY_BUILD_HARNESS "Build APlay harness validation tools" OFF)
option(APLAY_BUILD_LINUX "Build Linux platform native modules" OFF)
option(APLAY_BUILD_ANDROID "Build Android platform native modules and JNI binding" OFF)
option(APLAY_BUILD_HARMONY "Build Harmony platform native modules and NAPI binding" OFF)

set(APLAY_SDK_CPP_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")

include(${ALL_SUBDIR_CMAKELISTS})

include(${CLEAR_VARS})
set(LOCAL_MODULE cpp_sdk)
set(LOCAL_EXPORT_C_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/core/runtime/include)
set(LOCAL_INTERFACE_LIBRARIES
        osal  core  flow crypto
        protocol streaming)
include(${BUILD_INTERFACE_LIBRARY})