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

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(APLAY_SDK_CPP_DIR "${TOP_DIR}/sdk/src/main/cpp")

if(NOT TARGET aplay_cpp_sdk)
    add_subdirectory("${APLAY_SDK_CPP_DIR}" "${CMAKE_BINARY_DIR}/sdk-cpp")
endif()

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_linux_runtime_objects)
set(LOCAL_SRC_FILES
    runtime.cpp)
set(LOCAL_EXPORT_C_INCLUDES
    "${APLAY_SDK_CPP_DIR}/include")
set(LOCAL_INTERFACE_LIBRARIES
    aplay_native)
include(${BUILD_OBJECT_LIBRARY})

include(${CLEAR_VARS})
set(LOCAL_MODULE aplay_linux_runtime)
set(LOCAL_OBJECT_LIBRARIES
    aplay_linux_runtime_objects)
set(LOCAL_INTERFACE_LIBRARIES
    aplay_native)
include(${BUILD_INTERFACE_LIBRARY})

include(${CLEAR_VARS})
set(LOCAL_MODULE APlayReceiver)
set(LOCAL_SRC_FILES
    main.cpp)
set(LOCAL_INTERFACE_LIBRARIES
    aplay_linux_runtime)
set(LOCAL_RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR})
include(${BUILD_EXECUTABLE})

if(APLAY_BUILD_EXAMPLES)
    enable_testing()
    add_subdirectory("${TOP_DIR}/example" "${CMAKE_BINARY_DIR}/example")
endif()
