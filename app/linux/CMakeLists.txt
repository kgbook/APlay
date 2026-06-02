cmake_minimum_required(VERSION 3.20)

project(APlayLinux
    VERSION 0.1.0
    DESCRIPTION "APlay Linux executable application"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(APLAY_SDK_CPP_DIR "${CMAKE_CURRENT_LIST_DIR}/../../sdk/src/main/cpp")

if(NOT TARGET aplay_cpp_sdk)
    set(APLAY_BUILD_HARNESS OFF CACHE BOOL "" FORCE)
    set(APLAY_BUILD_LINUX_APP OFF CACHE BOOL "" FORCE)
    set(APLAY_BUILD_JNI_BINDING OFF CACHE BOOL "" FORCE)
    set(APLAY_BUILD_NAPI_BINDING OFF CACHE BOOL "" FORCE)
    add_subdirectory("${APLAY_SDK_CPP_DIR}" "${CMAKE_BINARY_DIR}/sdk-cpp")
endif()

add_library(aplay_linux_app
    runtime.cpp)

target_include_directories(aplay_linux_app
    PUBLIC
        "${APLAY_SDK_CPP_DIR}/include")

target_compile_features(aplay_linux_app PUBLIC cxx_std_17)

target_link_libraries(aplay_linux_app
    PUBLIC
        aplay_core)

add_executable(aplay
    main.cpp)

target_link_libraries(aplay PRIVATE aplay_linux_app)

set_target_properties(aplay PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
