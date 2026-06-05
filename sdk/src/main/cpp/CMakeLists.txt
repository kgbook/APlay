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

option(APLAY_BUILD_EXAMPLES "Compatibility alias for APlay harness validation tools" OFF)
option(APLAY_BUILD_HARNESS "Build APlay harness validation tools" OFF)
option(APLAY_BUILD_LINUX "Build Linux platform native modules" OFF)
option(APLAY_BUILD_ANDROID "Build Android platform native modules and JNI binding" OFF)
option(APLAY_BUILD_HARMONY "Build Harmony platform native modules and NAPI binding" OFF)

if(APLAY_BUILD_HARNESS)
    set(APLAY_BUILD_EXAMPLES ON)
endif()

set(APLAY_NATIVE_MODULES
    core
    flow
    crypto
    protocol
    streaming)

foreach(APLAY_NATIVE_MODULE IN LISTS APLAY_NATIVE_MODULES)
    add_subdirectory(${APLAY_NATIVE_MODULE})
endforeach()

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_native)
set(LOCAL_INTERFACE_LIBRARIES
    aplay_core
    aplay_flow
    aplay_crypto
    aplay_protocol
    aplay_streaming)
include(${BUILD_INTERFACE_LIBRARY})

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_cpp_sdk)
set(LOCAL_EXPORT_C_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/core/runtime/include)
set(LOCAL_INTERFACE_LIBRARIES
    aplay_native
    aplay_core_runtime)
include(${BUILD_INTERFACE_LIBRARY})

add_subdirectory(osal)

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_sdk_static)
set(LOCAL_INTERFACE_LIBRARIES
    aplay_cpp_sdk
    osal)
set(LOCAL_MODULE_FILENAME
    APlaySdk)
include(${BUILD_STATIC_LIBRARY})

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_sdk_prebuilt)
set(LOCAL_PREBUILT_MODULE_TYPE
    STATIC)
set(LOCAL_PREBUILT_LOCATION
    $<TARGET_FILE:aplay_sdk_static>)
set(LOCAL_EXPORT_C_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/core/runtime/include)
set(LOCAL_INTERFACE_LIBRARIES
    osal)
include(${BUILD_PREBUILT})

if(APLAY_BUILD_LINUX)
    include(${CLEAR_VARS})
    set(LOCAL_MODULE aplay_sdk)
    set(LOCAL_INTERFACE_LIBRARIES
        aplay_cpp_sdk
        osal)
    set(LOCAL_MODULE_FILENAME
        APlaySdk)
    include(${BUILD_SHARED_LIBRARY})
endif()
