cmake_minimum_required(VERSION 3.20)

project(APlayReceiver
    VERSION 0.1.0
    DESCRIPTION "APlay Linux executable application"
    LANGUAGES CXX)

get_filename_component(TOP_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(CMAKE_HELPER_MODULE_DIR "${TOP_DIR}/cmake/CMakeHelper/cmake")
list(FIND CMAKE_MODULE_PATH "${CMAKE_HELPER_MODULE_DIR}" CMAKE_HELPER_MODULE_DIR_INDEX)
if(CMAKE_HELPER_MODULE_DIR_INDEX EQUAL -1)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_HELPER_MODULE_DIR}")
endif()
include(CMakeHelper)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/sdk-cpp)

option(APLAY_BUILD_HARNESS "Build APlay harness validation tools" ON)
set(APLAY_BUILD_LINUX ON CACHE BOOL "Build Linux platform native modules")

add_subdirectory("${TOP_DIR}/sdk/src/main/cpp" ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

include(${CLEAR_VARS})
set(LOCAL_MODULE APlayReceiver)
set(LOCAL_SRC_FILES main.cpp runtime.cpp)
set(LOCAL_INTERFACE_LIBRARIES cpp_sdk SpdlogHelper)
set(LOCAL_C_INCLUDES include)
set(LOCAL_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
include(${BUILD_EXECUTABLE})

if(APLAY_BUILD_HARNESS)
    enable_testing()
    set(LOCAL_PATH "${TOP_DIR}/harness")
    include(${ALL_CMAKELISTS_UNDER})
    unset(LOCAL_PATH)
endif()
