add_library(aplay_napi SHARED
    napi_entry.cpp)

target_compile_features(aplay_napi PRIVATE cxx_std_17)
target_link_libraries(aplay_napi  PRIVATE aplay_cpp_sdk)
set_target_properties(aplay_napi PROPERTIES
    OUTPUT_NAME aplay_napi)
