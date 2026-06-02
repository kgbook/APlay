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
    add_subdirectory("${APLAY_SDK_CPP_DIR}" "${CMAKE_BINARY_DIR}/sdk-cpp")
endif()

add_library(aplay_linux_runtime
    runtime.cpp)

target_include_directories(aplay_linux_runtime
    PUBLIC
        "${APLAY_SDK_CPP_DIR}/include")

target_compile_features(aplay_linux_runtime PUBLIC cxx_std_17)

target_link_libraries(aplay_linux_runtime
    PUBLIC
        aplay_core)

add_executable(aplay
    main.cpp)

target_link_libraries(aplay PRIVATE aplay_linux_runtime)

set_target_properties(aplay PROPERTIES
    OUTPUT_NAME APlayReceiver
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
