add_library(aplay_linux_app
    runtime.cpp)

target_include_directories(aplay_linux_app
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

target_compile_features(aplay_linux_app PUBLIC cxx_std_17)

add_library(aplay_core ALIAS aplay_linux_app)

add_executable(aplay
    main.cpp)

target_link_libraries(aplay PRIVATE aplay_linux_app)

set_target_properties(aplay PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
