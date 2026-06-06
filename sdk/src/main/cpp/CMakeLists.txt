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

option(APLAY_BUILD_LINUX "Build Linux platform native modules" OFF)
option(APLAY_BUILD_ANDROID "Build Android platform native modules and JNI binding" OFF)
option(APLAY_BUILD_HARMONY "Build Harmony platform native modules and NAPI binding" OFF)

include(${ALL_SUBDIR_CMAKELISTS})

include(${CLEAR_VARS})
set(LOCAL_MODULE cpp_sdk)
set(LOCAL_INTERFACE_LIBRARIES core crypto flow osal
                protocol streaming third-party)
include(${BUILD_INTERFACE_LIBRARY})