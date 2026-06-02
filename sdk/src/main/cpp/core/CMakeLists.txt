add_library(aplay_core STATIC
    runtime.cpp)

target_include_directories(aplay_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_compile_features(aplay_core PUBLIC cxx_std_17)

set_target_properties(aplay_core PROPERTIES
    POSITION_INDEPENDENT_CODE ON)
